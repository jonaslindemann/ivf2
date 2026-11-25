#include <ivf/gltf_model.h>

#include <ivf/logger.h>
#include <ivf/mesh.h>
#include <ivf/normals.h>
#include <ivf/pbr_material.h>
#include <ivf/texture.h>
#include <ivf/vertices.h>

// cgltf is a private dependency: only this file includes it.
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>

using namespace ivf;

namespace {

/// A glTF primitive drawn by one Mesh, with what is needed to deform it on the CPU.
struct Primitive {
    MeshPtr mesh;
    std::vector<glm::vec3> positions; ///< Rest positions.
    std::vector<glm::vec3> normals;   ///< Rest normals.
    std::vector<glm::uvec4> joints;   ///< Skin joints (indices into the skin's joint list).
    std::vector<glm::vec4> weights;   ///< Skin weights.
    std::vector<std::vector<glm::vec3>> targetPositions; ///< Morph target position deltas.
    std::vector<std::vector<glm::vec3>> targetNormals;   ///< Morph target normal deltas (may be empty).
};

struct NodeRecord {
    CompositeNodePtr node;
    int parent{-1};
    std::string name;
    // Rest pose and current pose
    glm::vec3 restTranslation{0.0f};
    glm::quat restRotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 restScale{1.0f};
    glm::vec3 translation{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};
    // Mesh
    MeshNodePtr meshNode;
    std::vector<Primitive> primitives;
    int skin{-1};
    std::vector<float> restWeights;
    std::vector<float> weights;
};

struct Skin {
    std::vector<int> joints;
    std::vector<glm::mat4> inverseBindMatrices;
};

enum class Path { Translation, Rotation, Scale, Weights };

struct Sampler {
    std::vector<float> times;
    std::vector<float> values; ///< Flattened output (with in/out tangents for cubic splines).
    int components{0};         ///< Floats per value.
    cgltf_interpolation_type interpolation{cgltf_interpolation_type_linear};
};

struct Channel {
    int node{-1};
    Path path{Path::Translation};
    int sampler{-1};
};

struct Animation {
    std::string name;
    float duration{0.0f};
    std::vector<Sampler> samplers;
    std::vector<Channel> channels;
};

glm::vec3 readVec3(const cgltf_accessor *accessor, cgltf_size i)
{
    glm::vec3 v(0.0f);
    cgltf_accessor_read_float(accessor, i, glm::value_ptr(v), 3);
    return v;
}

} // namespace

struct ivf::GltfModel::Impl {
    std::vector<NodeRecord> nodes;
    std::vector<Skin> skins;
    std::vector<Animation> animations;
    std::vector<PBRMaterialPtr> materials;
    std::string path;
    int animation{-1};
    bool looping{true};
    float time{0.0f};

    glm::mat4 localMatrix(const NodeRecord &n) const
    {
        return glm::translate(glm::mat4(1.0f), n.translation) * glm::mat4_cast(n.rotation) *
               glm::scale(glm::mat4(1.0f), n.scale);
    }

    /// Matrices of all nodes relative to the model (parents come before children in nodes)
    std::vector<glm::mat4> worldMatrices() const
    {
        std::vector<glm::mat4> world(nodes.size(), glm::mat4(1.0f));
        std::vector<char> done(nodes.size(), 0);
        auto compute = [&](auto &&self, int i) -> const glm::mat4 & {
            auto index = static_cast<std::size_t>(i);
            if (!done[index])
            {
                const glm::mat4 local = localMatrix(nodes[index]);
                world[index] = nodes[index].parent >= 0 ? self(self, nodes[index].parent) * local : local;
                done[index] = 1;
            }
            return world[index];
        };
        for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
            compute(compute, i);
        return world;
    }

    void applyTransforms()
    {
        for (auto &n : nodes)
        {
            n.node->setPos(n.translation);
            n.node->setRotation(n.rotation);
            n.node->setScale(n.scale);
        }
    }

    void resetPose()
    {
        for (auto &n : nodes)
        {
            n.translation = n.restTranslation;
            n.rotation = n.restRotation;
            n.scale = n.restScale;
            n.weights = n.restWeights;
        }
    }

    static float sample(const Sampler &s, float t, float *out)
    {
        const auto keys = s.times.size();
        const int c = s.components;
        const bool cubic = s.interpolation == cgltf_interpolation_type_cubic_spline;
        // Value k (the middle of the in-tangent, value, out-tangent triplet for cubic splines)
        auto value = [&](std::size_t k, int part) {
            return &s.values[(cubic ? (k * 3 + static_cast<std::size_t>(part)) : k) * static_cast<std::size_t>(c)];
        };
        if (keys == 0)
            return 0.0f;
        if (keys == 1 || t <= s.times.front())
        {
            std::copy_n(value(0, 1), c, out);
            return 0.0f;
        }
        if (t >= s.times.back())
        {
            std::copy_n(value(keys - 1, 1), c, out);
            return 0.0f;
        }
        const auto next = static_cast<std::size_t>(std::upper_bound(s.times.begin(), s.times.end(), t) - s.times.begin());
        const std::size_t k = next - 1;
        const float t0 = s.times[k];
        const float dt = s.times[next] - t0;
        const float u = dt > 0.0f ? (t - t0) / dt : 0.0f;

        if (s.interpolation == cgltf_interpolation_type_step)
            std::copy_n(value(k, 1), c, out);
        else if (cubic)
        {
            const float u2 = u * u, u3 = u2 * u;
            const float h00 = 2 * u3 - 3 * u2 + 1, h10 = u3 - 2 * u2 + u, h01 = -2 * u3 + 3 * u2, h11 = u3 - u2;
            const float *p0 = value(k, 1), *m0 = value(k, 2), *p1 = value(next, 1), *m1 = value(next, 0);
            for (int i = 0; i < c; ++i)
                out[i] = h00 * p0[i] + h10 * dt * m0[i] + h01 * p1[i] + h11 * dt * m1[i];
        }
        else
        {
            const float *a = value(k, 1), *b = value(next, 1);
            for (int i = 0; i < c; ++i)
                out[i] = a[i] + (b[i] - a[i]) * u;
        }
        return u;
    }

    void animate(float t)
    {
        if (animation < 0 || animation >= static_cast<int>(animations.size()))
            return;
        const Animation &anim = animations[static_cast<std::size_t>(animation)];
        if (anim.duration > 0.0f)
            t = looping ? std::fmod(std::fmod(t, anim.duration) + anim.duration, anim.duration)
                        : std::clamp(t, 0.0f, anim.duration);

        std::vector<float> buffer;
        for (const auto &channel : anim.channels)
        {
            const Sampler &s = anim.samplers[static_cast<std::size_t>(channel.sampler)];
            NodeRecord &n = nodes[static_cast<std::size_t>(channel.node)];
            buffer.assign(static_cast<std::size_t>(std::max(s.components, 1)), 0.0f);

            if (channel.path == Path::Rotation && s.interpolation == cgltf_interpolation_type_linear &&
                s.times.size() > 1 && t > s.times.front() && t < s.times.back())
            {
                // Quaternions: spherical interpolation
                const auto next = static_cast<std::size_t>(
                    std::upper_bound(s.times.begin(), s.times.end(), t) - s.times.begin());
                const std::size_t k = next - 1;
                const float u = (t - s.times[k]) / std::max(s.times[next] - s.times[k], 1e-9f);
                const float *a = &s.values[k * 4], *b = &s.values[next * 4];
                n.rotation = glm::normalize(glm::slerp(glm::quat(a[3], a[0], a[1], a[2]),
                                                       glm::quat(b[3], b[0], b[1], b[2]), u));
                continue;
            }

            sample(s, t, buffer.data());
            switch (channel.path)
            {
            case Path::Translation:
                n.translation = glm::vec3(buffer[0], buffer[1], buffer[2]);
                break;
            case Path::Rotation:
                n.rotation = glm::normalize(glm::quat(buffer[3], buffer[0], buffer[1], buffer[2]));
                break;
            case Path::Scale:
                n.scale = glm::vec3(buffer[0], buffer[1], buffer[2]);
                break;
            case Path::Weights:
                n.weights.assign(buffer.begin(), buffer.begin() + std::min<std::ptrdiff_t>(
                                                                       static_cast<std::ptrdiff_t>(buffer.size()),
                                                                       static_cast<std::ptrdiff_t>(n.weights.size())));
                break;
            }
        }
    }

    void deform()
    {
        const bool anyDeformation = std::any_of(nodes.begin(), nodes.end(), [](const NodeRecord &n) {
            return n.skin >= 0 || std::any_of(n.primitives.begin(), n.primitives.end(),
                                              [](const Primitive &p) { return !p.targetPositions.empty(); });
        });
        if (!anyDeformation)
            return;

        const std::vector<glm::mat4> world = worldMatrices();
        std::vector<glm::mat4> jointMatrices;
        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
            NodeRecord &n = nodes[i];
            if (!n.meshNode)
                continue;
            const bool skinned = n.skin >= 0;
            if (skinned)
            {
                // Joint matrices relative to the mesh's node (the node's own transform is ignored)
                const Skin &skin = skins[static_cast<std::size_t>(n.skin)];
                const glm::mat4 inverseMesh = glm::inverse(world[i]);
                jointMatrices.resize(skin.joints.size());
                for (std::size_t j = 0; j < skin.joints.size(); ++j)
                    jointMatrices[j] = inverseMesh * world[static_cast<std::size_t>(skin.joints[j])] *
                                       skin.inverseBindMatrices[j];
            }

            bool changed = false;
            for (auto &p : n.primitives)
            {
                if (!skinned && p.targetPositions.empty())
                    continue;
                auto vertices = p.mesh->vertices();
                auto normals = p.mesh->normals();
                for (std::size_t v = 0; v < p.positions.size(); ++v)
                {
                    glm::vec3 pos = p.positions[v];
                    glm::vec3 nrm = p.normals.empty() ? glm::vec3(0.0f, 1.0f, 0.0f) : p.normals[v];
                    for (std::size_t t = 0; t < p.targetPositions.size() && t < n.weights.size(); ++t)
                    {
                        const float w = n.weights[t];
                        if (w == 0.0f)
                            continue;
                        pos += w * p.targetPositions[t][v];
                        if (!p.targetNormals[t].empty())
                            nrm += w * p.targetNormals[t][v];
                    }
                    if (skinned && v < p.joints.size())
                    {
                        const glm::uvec4 &jn = p.joints[v];
                        const glm::vec4 &wt = p.weights[v];
                        glm::mat4 m(0.0f);
                        for (int k = 0; k < 4; ++k)
                            if (wt[k] != 0.0f && jn[k] < jointMatrices.size())
                                m += wt[k] * jointMatrices[jn[k]];
                        pos = glm::vec3(m * glm::vec4(pos, 1.0f));
                        nrm = glm::mat3(m) * nrm;
                    }
                    vertices->setVertex(static_cast<GLuint>(v), pos);
                    if (normals)
                        normals->setNormal(static_cast<GLuint>(v), glm::length(nrm) > 0.0f ? glm::normalize(nrm) : nrm);
                }
                p.mesh->updateVertices();
                p.mesh->updateNormals();
                changed = true;
            }
            if (changed)
                n.meshNode->updateBoundingBox();
        }
    }
};

GltfModel::GltfModel() : m_impl(std::make_unique<Impl>())
{
    setName("glTF model");
}

GltfModel::~GltfModel() = default;

GltfModelPtr GltfModel::create()
{
    return std::make_shared<GltfModel>();
}

// ---- Loading ----------------------------------------------------------------

namespace ivf {

/// Builds a GltfModel from parsed cgltf data (friend of GltfModel).
struct GltfLoaderAccess {
    const cgltf_data *data{nullptr};
    std::filesystem::path directory;
    std::string fileName;
    std::map<const cgltf_texture *, TexturePtr> textures;
    std::map<const cgltf_material *, PBRMaterialPtr> materials;

    TexturePtr texture(const cgltf_texture_view &view)
    {
        const cgltf_texture *tex = view.texture;
        if (tex == nullptr || tex->image == nullptr)
            return nullptr;
        if (auto it = textures.find(tex); it != textures.end())
            return it->second;

        const cgltf_image *image = tex->image;
        const std::string name = image->name ? image->name : (image->uri ? image->uri : "embedded");
        auto texture = Texture::create();
        Result<void> result;
        if (image->buffer_view != nullptr)
        {
            const uint8_t *bytes = cgltf_buffer_view_data(image->buffer_view);
            result = texture->loadFromMemory(bytes, image->buffer_view->size, fileName + ": " + name);
        }
        else if (image->uri != nullptr && std::strncmp(image->uri, "data:", 5) == 0)
        {
            // Base64 data URI: "data:image/png;base64,..."
            result = makeError(ErrorCode::InvalidData, "Bad image data URI in " + fileName, "GltfModel");
            if (const char *comma = std::strchr(image->uri, ','))
            {
                const char *base64 = comma + 1;
                const std::size_t length = std::strlen(base64);
                std::size_t size = length / 4 * 3;
                if (length >= 1 && base64[length - 1] == '=')
                    --size;
                if (length >= 2 && base64[length - 2] == '=')
                    --size;
                cgltf_options options{};
                void *decoded = nullptr;
                if (cgltf_load_buffer_base64(&options, size, base64, &decoded) == cgltf_result_success)
                {
                    result = texture->loadFromMemory(static_cast<const unsigned char *>(decoded), size,
                                                     fileName + ": " + name);
                    std::free(decoded);
                }
            }
        }
        else if (image->uri != nullptr)
        {
            std::string uri = image->uri;
            uri.push_back('\0');
            cgltf_decode_uri(uri.data());
            result = texture->load((directory / std::string(uri.c_str())).string());
        }
        if (!result)
        {
            logWarning(logStr("Texture ", name, " of ", fileName, " not loaded"), "GltfModel");
            textures[tex] = nullptr;
            return nullptr;
        }

        if (const cgltf_sampler *s = tex->sampler)
        {
            if (s->wrap_s != 0)
                texture->setWrapS(static_cast<GLint>(s->wrap_s));
            if (s->wrap_t != 0)
                texture->setWrapT(static_cast<GLint>(s->wrap_t));
            if (s->min_filter != 0)
                texture->setMinFilter(static_cast<GLint>(s->min_filter));
            if (s->mag_filter != 0)
                texture->setMagFilter(static_cast<GLint>(s->mag_filter));
        }
        textures[tex] = texture;
        return texture;
    }

    PBRMaterialPtr material(const cgltf_material *m)
    {
        if (auto it = materials.find(m); it != materials.end())
            return it->second;

        auto material = PBRMaterial::create();
        material->setUseVertexColors(true); // primitives without COLOR_0 get white
        if (m == nullptr)
        {
            // glTF default material
            material->setMetallic(1.0f);
            material->setRoughness(1.0f);
            materials[m] = material;
            return material;
        }

        if (m->has_pbr_metallic_roughness)
        {
            const auto &pbr = m->pbr_metallic_roughness;
            material->setAlbedo(glm::make_vec4(pbr.base_color_factor));
            material->setMetallic(pbr.metallic_factor);
            material->setRoughness(pbr.roughness_factor);
            material->setAlbedoMap(texture(pbr.base_color_texture));
            material->setMetallicRoughnessMap(texture(pbr.metallic_roughness_texture));
        }
        else
        {
            material->setMetallic(0.0f);
            material->setRoughness(0.8f);
        }
        material->setNormalMap(texture(m->normal_texture));
        material->setAOMap(texture(m->occlusion_texture));
        glm::vec3 emissive = glm::make_vec3(m->emissive_factor);
        if (m->has_emissive_strength)
            emissive *= m->emissive_strength.emissive_strength;
        material->setEmissive(emissive);
        material->setEmissiveMap(texture(m->emissive_texture));
        switch (m->alpha_mode)
        {
        case cgltf_alpha_mode_mask:
            material->setAlphaMode(PBRMaterial::AlphaMode::Mask);
            material->setAlphaCutoff(m->alpha_cutoff);
            break;
        case cgltf_alpha_mode_blend:
            material->setAlphaMode(PBRMaterial::AlphaMode::Blend);
            break;
        default:
            material->setAlphaMode(PBRMaterial::AlphaMode::Opaque);
            break;
        }
        materials[m] = material;
        return material;
    }

    /// Triangle list of a primitive (indices into its vertices)
    static std::vector<GLuint> triangles(const cgltf_primitive &prim, cgltf_size vertexCount)
    {
        std::vector<GLuint> source;
        if (prim.indices != nullptr)
        {
            source.resize(prim.indices->count);
            for (cgltf_size i = 0; i < prim.indices->count; ++i)
                source[i] = static_cast<GLuint>(cgltf_accessor_read_index(prim.indices, i));
        }
        else
        {
            source.resize(vertexCount);
            for (cgltf_size i = 0; i < vertexCount; ++i)
                source[i] = static_cast<GLuint>(i);
        }

        std::vector<GLuint> out;
        switch (prim.type)
        {
        case cgltf_primitive_type_triangles:
            out.assign(source.begin(), source.begin() + static_cast<std::ptrdiff_t>(source.size() / 3 * 3));
            break;
        case cgltf_primitive_type_triangle_strip:
            for (std::size_t i = 2; i < source.size(); ++i)
            {
                if (i % 2 == 0)
                    out.insert(out.end(), {source[i - 2], source[i - 1], source[i]});
                else
                    out.insert(out.end(), {source[i - 1], source[i - 2], source[i]});
            }
            break;
        case cgltf_primitive_type_triangle_fan:
            for (std::size_t i = 2; i < source.size(); ++i)
                out.insert(out.end(), {source[0], source[i - 1], source[i]});
            break;
        default:
            break;
        }
        return out;
    }

    void buildMesh(NodeRecord &record, const cgltf_mesh &mesh, bool deformable)
    {
        auto meshNode = MeshNode::create();
        meshNode->setName(mesh.name ? mesh.name : (record.name + " mesh"));

        for (cgltf_size p = 0; p < mesh.primitives_count; ++p)
        {
            const cgltf_primitive &prim = mesh.primitives[p];
            const cgltf_accessor *position = nullptr, *normal = nullptr, *uv = nullptr, *color = nullptr,
                                 *joints = nullptr, *weights = nullptr;
            for (cgltf_size a = 0; a < prim.attributes_count; ++a)
            {
                const cgltf_attribute &attr = prim.attributes[a];
                switch (attr.type)
                {
                case cgltf_attribute_type_position:
                    position = attr.data;
                    break;
                case cgltf_attribute_type_normal:
                    normal = attr.data;
                    break;
                case cgltf_attribute_type_texcoord:
                    if (attr.index == 0)
                        uv = attr.data;
                    break;
                case cgltf_attribute_type_color:
                    if (attr.index == 0)
                        color = attr.data;
                    break;
                case cgltf_attribute_type_joints:
                    if (attr.index == 0)
                        joints = attr.data;
                    break;
                case cgltf_attribute_type_weights:
                    if (attr.index == 0)
                        weights = attr.data;
                    break;
                default:
                    break;
                }
            }
            if (position == nullptr)
                continue;
            if (prim.type != cgltf_primitive_type_triangles && prim.type != cgltf_primitive_type_triangle_strip &&
                prim.type != cgltf_primitive_type_triangle_fan)
            {
                logWarning(logStr("Skipping a primitive of ", fileName, ": only triangles are supported"),
                           "GltfModel");
                continue;
            }

            const cgltf_size count = position->count;
            const std::vector<GLuint> tris = triangles(prim, count);
            if (tris.empty())
                continue;

            Primitive out;
            out.positions.resize(count);
            for (cgltf_size v = 0; v < count; ++v)
                out.positions[v] = readVec3(position, v);
            out.normals.resize(count, glm::vec3(0.0f));
            if (normal != nullptr)
            {
                for (cgltf_size v = 0; v < count; ++v)
                    out.normals[v] = readVec3(normal, v);
            }
            else
            {
                // Smooth normals from the (counter-clockwise) triangles
                for (std::size_t t = 0; t + 2 < tris.size(); t += 3)
                {
                    const glm::vec3 &a = out.positions[tris[t]], &b = out.positions[tris[t + 1]],
                                    &c = out.positions[tris[t + 2]];
                    const glm::vec3 n = glm::cross(b - a, c - a);
                    out.normals[tris[t]] += n;
                    out.normals[tris[t + 1]] += n;
                    out.normals[tris[t + 2]] += n;
                }
                for (auto &n : out.normals)
                    n = glm::length(n) > 0.0f ? glm::normalize(n) : glm::vec3(0.0f, 1.0f, 0.0f);
            }

            if (joints != nullptr && weights != nullptr && record.skin >= 0)
            {
                out.joints.resize(count);
                out.weights.resize(count);
                for (cgltf_size v = 0; v < count; ++v)
                {
                    cgltf_uint j[4] = {0, 0, 0, 0};
                    cgltf_accessor_read_uint(joints, v, j, 4);
                    out.joints[v] = glm::uvec4(j[0], j[1], j[2], j[3]);
                    glm::vec4 w(0.0f);
                    cgltf_accessor_read_float(weights, v, glm::value_ptr(w), 4);
                    const float sum = w.x + w.y + w.z + w.w;
                    out.weights[v] = sum > 0.0f ? w / sum : glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
                }
            }
            for (cgltf_size t = 0; t < prim.targets_count; ++t)
            {
                std::vector<glm::vec3> dp(count, glm::vec3(0.0f)), dn;
                const cgltf_morph_target &target = prim.targets[t];
                for (cgltf_size a = 0; a < target.attributes_count; ++a)
                {
                    const cgltf_attribute &attr = target.attributes[a];
                    if (attr.type == cgltf_attribute_type_position)
                        for (cgltf_size v = 0; v < count && v < attr.data->count; ++v)
                            dp[v] = readVec3(attr.data, v);
                    else if (attr.type == cgltf_attribute_type_normal)
                    {
                        dn.assign(count, glm::vec3(0.0f));
                        for (cgltf_size v = 0; v < count && v < attr.data->count; ++v)
                            dn[v] = readVec3(attr.data, v);
                    }
                }
                out.targetPositions.push_back(std::move(dp));
                out.targetNormals.push_back(std::move(dn));
            }

            const bool deforms = deformable && (!out.joints.empty() || !out.targetPositions.empty());
            meshNode->newMesh(static_cast<int>(count), static_cast<int>(tris.size() / 3), GL_TRIANGLES,
                              deforms ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            auto m = meshNode->currentMesh();
            m->setGenerateNormals(false);
            m->begin(GL_TRIANGLES);
            for (cgltf_size v = 0; v < count; ++v)
            {
                m->vertex3f(out.positions[v]);
                m->normal3f(out.normals[v]);
                glm::vec2 st(0.0f);
                if (uv != nullptr)
                    cgltf_accessor_read_float(uv, v, glm::value_ptr(st), 2);
                m->tex2f(st.x, st.y);
                glm::vec4 rgba(1.0f);
                if (color != nullptr)
                    cgltf_accessor_read_float(color, v, glm::value_ptr(rgba), cgltf_num_components(color->type));
                m->color4f(rgba.r, rgba.g, rgba.b, rgba.a);
            }
            for (std::size_t t = 0; t + 2 < tris.size(); t += 3)
                m->index3i(tris[t], tris[t + 1], tris[t + 2]);
            m->end();
            m->setMaterial(material(prim.material));

            if (deforms)
            {
                out.mesh = m;
                record.primitives.push_back(std::move(out));
            }
        }

        if (mesh.weights_count > 0)
            record.restWeights.assign(mesh.weights, mesh.weights + mesh.weights_count);
        if (!record.primitives.empty() && !record.primitives.front().targetPositions.empty())
            record.restWeights.resize(record.primitives.front().targetPositions.size(), 0.0f);
        record.weights = record.restWeights;

        meshNode->updateBoundingBox();
        record.meshNode = meshNode;
        record.node->add(meshNode);
    }

    Result<GltfModelPtr> build()
    {
        auto model = GltfModel::create();
        model->setName(fileName);
        GltfModel::Impl &impl = *model->m_impl;

        // Nodes
        impl.nodes.resize(data->nodes_count);
        for (cgltf_size i = 0; i < data->nodes_count; ++i)
        {
            const cgltf_node &src = data->nodes[i];
            NodeRecord &n = impl.nodes[i];
            n.name = src.name ? src.name : ("node " + std::to_string(i));
            n.node = CompositeNode::create();
            n.node->setName(n.name);
            n.parent = src.parent ? static_cast<int>(cgltf_node_index(data, src.parent)) : -1;
            if (src.has_matrix)
            {
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(glm::make_mat4(src.matrix), n.restScale, n.restRotation, n.restTranslation, skew,
                               perspective);
            }
            else
            {
                if (src.has_translation)
                    n.restTranslation = glm::make_vec3(src.translation);
                if (src.has_rotation)
                    n.restRotation = glm::quat(src.rotation[3], src.rotation[0], src.rotation[1], src.rotation[2]);
                if (src.has_scale)
                    n.restScale = glm::make_vec3(src.scale);
            }
            n.skin = src.skin ? static_cast<int>(cgltf_skin_index(data, src.skin)) : -1;
        }

        // Skins
        impl.skins.resize(data->skins_count);
        for (cgltf_size s = 0; s < data->skins_count; ++s)
        {
            const cgltf_skin &src = data->skins[s];
            Skin &skin = impl.skins[s];
            for (cgltf_size j = 0; j < src.joints_count; ++j)
            {
                skin.joints.push_back(static_cast<int>(cgltf_node_index(data, src.joints[j])));
                glm::mat4 ibm(1.0f);
                if (src.inverse_bind_matrices != nullptr)
                    cgltf_accessor_read_float(src.inverse_bind_matrices, j, glm::value_ptr(ibm), 16);
                skin.inverseBindMatrices.push_back(ibm);
            }
        }

        // Which meshes and nodes are animated (their vertices change)
        for (cgltf_size i = 0; i < data->nodes_count; ++i)
        {
            const cgltf_node &src = data->nodes[i];
            if (src.mesh == nullptr)
                continue;
            NodeRecord &n = impl.nodes[i];
            buildMesh(n, *src.mesh, true);
            if (src.weights_count > 0)
            {
                n.restWeights.assign(src.weights, src.weights + src.weights_count);
                n.weights = n.restWeights;
            }
        }

        // Hierarchy: children under their parents, the scene's roots under the model
        for (cgltf_size i = 0; i < data->nodes_count; ++i)
        {
            NodeRecord &n = impl.nodes[i];
            if (n.parent >= 0)
                impl.nodes[static_cast<std::size_t>(n.parent)].node->add(n.node);
        }
        const cgltf_scene *scene = data->scene ? data->scene : (data->scenes_count > 0 ? &data->scenes[0] : nullptr);
        if (scene != nullptr)
        {
            for (cgltf_size r = 0; r < scene->nodes_count; ++r)
                model->add(impl.nodes[cgltf_node_index(data, scene->nodes[r])].node);
        }
        else
        {
            for (auto &n : impl.nodes)
                if (n.parent < 0)
                    model->add(n.node);
        }

        // Animations
        for (cgltf_size a = 0; a < data->animations_count; ++a)
        {
            const cgltf_animation &src = data->animations[a];
            Animation anim;
            anim.name = src.name ? src.name : ("animation " + std::to_string(a));
            for (cgltf_size s = 0; s < src.samplers_count; ++s)
            {
                const cgltf_animation_sampler &ss = src.samplers[s];
                Sampler sampler;
                sampler.interpolation = ss.interpolation;
                sampler.times.resize(ss.input->count);
                for (cgltf_size k = 0; k < ss.input->count; ++k)
                    cgltf_accessor_read_float(ss.input, k, &sampler.times[k], 1);
                if (!sampler.times.empty())
                    anim.duration = std::max(anim.duration, sampler.times.back());
                const cgltf_size components = cgltf_num_components(ss.output->type);
                sampler.values.resize(ss.output->count * components);
                for (cgltf_size k = 0; k < ss.output->count; ++k)
                    cgltf_accessor_read_float(ss.output, k, &sampler.values[k * components], components);
                sampler.components = static_cast<int>(components);
                anim.samplers.push_back(std::move(sampler));
            }
            for (cgltf_size c = 0; c < src.channels_count; ++c)
            {
                const cgltf_animation_channel &sc = src.channels[c];
                if (sc.target_node == nullptr || sc.sampler == nullptr)
                    continue;
                Channel channel;
                channel.node = static_cast<int>(cgltf_node_index(data, sc.target_node));
                channel.sampler = static_cast<int>(cgltf_animation_sampler_index(&src, sc.sampler));
                switch (sc.target_path)
                {
                case cgltf_animation_path_type_translation:
                    channel.path = Path::Translation;
                    break;
                case cgltf_animation_path_type_rotation:
                    channel.path = Path::Rotation;
                    break;
                case cgltf_animation_path_type_scale:
                    channel.path = Path::Scale;
                    break;
                case cgltf_animation_path_type_weights: {
                    channel.path = Path::Weights;
                    // Weights: one value per target, so the components are the target count
                    Sampler &s = anim.samplers[static_cast<std::size_t>(channel.sampler)];
                    const std::size_t targets = impl.nodes[static_cast<std::size_t>(channel.node)].weights.size();
                    const std::size_t keys = s.times.size() * (s.interpolation == cgltf_interpolation_type_cubic_spline ? 3 : 1);
                    if (targets == 0 || keys == 0 || s.values.size() != keys * targets)
                        continue;
                    s.components = static_cast<int>(targets);
                    break;
                }
                default:
                    continue;
                }
                anim.channels.push_back(channel);
            }
            impl.animations.push_back(std::move(anim));
        }

        for (const auto &[source, material] : materials)
            impl.materials.push_back(material);

        impl.resetPose();
        impl.applyTransforms();
        impl.deform();
        return model;
    }
};

} // namespace ivf

Result<GltfModelPtr> GltfModel::load(const std::string &path)
{
    if (!std::filesystem::exists(path))
        return makeError(ErrorCode::FileNotFound, "glTF file not found: " + path, "GltfModel");

    cgltf_options options{};
    cgltf_data *data = nullptr;
    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success)
        return makeError(ErrorCode::UnsupportedFormat, "Not a valid glTF 2.0 file: " + path, "GltfModel");

    struct DataGuard {
        cgltf_data *data;
        ~DataGuard()
        {
            cgltf_free(data);
        }
    } guard{data};

    for (cgltf_size i = 0; i < data->extensions_required_count; ++i)
    {
        const std::string extension = data->extensions_required[i];
        if (extension != "KHR_materials_emissive_strength" && extension != "KHR_lights_punctual")
            return makeError(ErrorCode::NotSupported,
                             "glTF extension " + extension + " is required by " + path + " but not supported",
                             "GltfModel");
    }

    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success)
        return makeError(ErrorCode::ReadFailed, "Could not read the buffers of " + path, "GltfModel");
    if (cgltf_validate(data) != cgltf_result_success)
        return makeError(ErrorCode::InvalidData, "Invalid glTF data in " + path, "GltfModel");

    GltfLoaderAccess loader;
    loader.data = data;
    loader.directory = std::filesystem::path(path).parent_path();
    loader.fileName = std::filesystem::path(path).filename().string();
    auto model = loader.build();
    if (model)
        (*model)->m_impl->path = path;
    if (model)
        logInfo(logStr("Loaded ", loader.fileName, ": ", data->nodes_count, " nodes, ", data->meshes_count, " meshes, ",
                       data->materials_count, " materials, ", data->animations_count, " animations"),
                "GltfModel");
    return model;
}

Result<void> GltfModel::reload(const std::string &path)
{
    const std::string file = path.empty() ? m_impl->path : path;
    auto fresh = load(file);
    if (!fresh)
        return std::unexpected(fresh.error());

    // Keep what the application chose
    const std::string animationName = this->animationName(m_impl->animation);
    const bool looping = m_impl->looping;
    const float time = m_impl->time;

    clear();
    for (auto &child : (*fresh)->nodes())
        add(child);
    (*fresh)->clear();
    std::swap(m_impl, (*fresh)->m_impl);

    m_impl->looping = looping;
    m_impl->time = time;
    setAnimation(animationName.empty() ? -1 : findAnimation(animationName));
    return {};
}

const std::string &GltfModel::path() const
{
    return m_impl->path;
}

// ---- Nodes and animation ----------------------------------------------------

int GltfModel::nodeCount() const
{
    return static_cast<int>(m_impl->nodes.size());
}

CompositeNodePtr GltfModel::node(int index) const
{
    if (index < 0 || index >= nodeCount())
        return nullptr;
    return m_impl->nodes[static_cast<std::size_t>(index)].node;
}

int GltfModel::findNodeIndex(std::string_view name) const
{
    for (std::size_t i = 0; i < m_impl->nodes.size(); ++i)
        if (m_impl->nodes[i].name == name)
            return static_cast<int>(i);
    return -1;
}

CompositeNodePtr GltfModel::findNode(std::string_view name) const
{
    return node(findNodeIndex(name));
}

std::vector<MeshNodePtr> GltfModel::meshNodes() const
{
    std::vector<MeshNodePtr> result;
    for (const auto &n : m_impl->nodes)
        if (n.meshNode)
            result.push_back(n.meshNode);
    return result;
}

std::vector<PBRMaterialPtr> GltfModel::materials() const
{
    return m_impl->materials;
}

int GltfModel::animationCount() const
{
    return static_cast<int>(m_impl->animations.size());
}

std::string GltfModel::animationName(int index) const
{
    if (index < 0 || index >= animationCount())
        return {};
    return m_impl->animations[static_cast<std::size_t>(index)].name;
}

float GltfModel::animationDuration(int index) const
{
    if (index < 0 || index >= animationCount())
        return 0.0f;
    return m_impl->animations[static_cast<std::size_t>(index)].duration;
}

int GltfModel::findAnimation(std::string_view name) const
{
    for (std::size_t i = 0; i < m_impl->animations.size(); ++i)
        if (m_impl->animations[i].name == name)
            return static_cast<int>(i);
    return -1;
}

void GltfModel::setAnimation(int index)
{
    m_impl->animation = (index >= 0 && index < animationCount()) ? index : -1;
    m_impl->resetPose();
    m_impl->animate(m_impl->time);
    m_impl->applyTransforms();
    m_impl->deform();
}

int GltfModel::animation() const
{
    return m_impl->animation;
}

void GltfModel::setLooping(bool flag)
{
    m_impl->looping = flag;
}

bool GltfModel::looping() const
{
    return m_impl->looping;
}

void GltfModel::setTime(float seconds)
{
    m_impl->time = seconds;
    m_impl->animate(seconds);
    m_impl->applyTransforms();
    m_impl->deform();
}

float GltfModel::time() const
{
    return m_impl->time;
}

void GltfModel::setMorphWeights(int index, const std::vector<float> &weights)
{
    if (index < 0 || index >= nodeCount())
        return;
    auto &n = m_impl->nodes[static_cast<std::size_t>(index)];
    for (std::size_t i = 0; i < n.weights.size() && i < weights.size(); ++i)
        n.weights[i] = weights[i];
}

std::vector<float> GltfModel::morphWeights(int index) const
{
    if (index < 0 || index >= nodeCount())
        return {};
    return m_impl->nodes[static_cast<std::size_t>(index)].weights;
}

void GltfModel::update()
{
    // Take over node transforms changed by hand (node(i)->setPos(), setRotation(), setScale())
    for (auto &n : m_impl->nodes)
    {
        n.translation = n.node->pos();
        n.rotation = n.node->rotation();
        n.scale = n.node->scale();
    }
    m_impl->deform();
}

bool GltfModel::hasDeformation() const
{
    return std::any_of(m_impl->nodes.begin(), m_impl->nodes.end(),
                       [](const NodeRecord &n) { return !n.primitives.empty(); });
}
