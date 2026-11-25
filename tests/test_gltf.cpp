// glTF loading without OpenGL: hierarchy, meshes, materials, animation, skinning, morph
// targets and errors. The test files are written here (JSON with base64-embedded buffers).

#include "test_main.h"

#include <ivf/gltf_model.h>
#include <ivf/mesh.h>
#include <ivf/model_loader.h>
#include <ivf/pbr_material.h>
#include <ivf/vertices.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace ivf;

namespace {

std::string base64(const std::vector<std::uint8_t> &data)
{
    static const char *table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    std::size_t i = 0;
    for (; i + 2 < data.size(); i += 3)
    {
        const std::uint32_t v = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out += table[(v >> 18) & 63];
        out += table[(v >> 12) & 63];
        out += table[(v >> 6) & 63];
        out += table[v & 63];
    }
    if (i + 1 == data.size())
    {
        const std::uint32_t v = data[i] << 16;
        out += table[(v >> 18) & 63];
        out += table[(v >> 12) & 63];
        out += "==";
    }
    else if (i + 2 == data.size())
    {
        const std::uint32_t v = (data[i] << 16) | (data[i + 1] << 8);
        out += table[(v >> 18) & 63];
        out += table[(v >> 12) & 63];
        out += table[(v >> 6) & 63];
        out += '=';
    }
    return out;
}

/// Collects buffer data and writes accessors/bufferViews for a small glTF file.
class GltfWriter {
public:
    /// Float accessor; type "SCALAR", "VEC3", "VEC4", "MAT4", ...
    int floats(const std::vector<float> &values, const std::string &type, bool minMax = false)
    {
        const std::size_t offset = append(values.data(), values.size() * sizeof(float));
        const int components = type == "SCALAR" ? 1 : type == "VEC2" ? 2 : type == "VEC3" ? 3 : type == "VEC4" ? 4 : 16;
        std::ostringstream a;
        a << R"({"bufferView":)" << view(offset, values.size() * sizeof(float)) << R"(,"componentType":5126,"count":)"
          << values.size() / static_cast<std::size_t>(components) << R"(,"type":")" << type << '"';
        if (minMax)
        {
            std::vector<float> lo(static_cast<std::size_t>(components), 1e30f), hi(lo.size(), -1e30f);
            for (std::size_t i = 0; i < values.size(); ++i)
            {
                lo[i % lo.size()] = std::min(lo[i % lo.size()], values[i]);
                hi[i % hi.size()] = std::max(hi[i % hi.size()], values[i]);
            }
            a << R"(,"min":[)" << join(lo) << R"(],"max":[)" << join(hi) << ']';
        }
        a << '}';
        m_accessors.push_back(a.str());
        return static_cast<int>(m_accessors.size()) - 1;
    }

    int ushorts(const std::vector<std::uint16_t> &values, const std::string &type)
    {
        const std::size_t offset = append(values.data(), values.size() * sizeof(std::uint16_t));
        const int components = type == "SCALAR" ? 1 : 4;
        std::ostringstream a;
        a << R"({"bufferView":)" << view(offset, values.size() * 2) << R"(,"componentType":5123,"count":)"
          << values.size() / static_cast<std::size_t>(components) << R"(,"type":")" << type << R"("})";
        m_accessors.push_back(a.str());
        return static_cast<int>(m_accessors.size()) - 1;
    }

    /// Write the file with the given top-level members (meshes, nodes, ...)
    std::string write(const std::string &name, const std::string &members)
    {
        std::ostringstream json;
        json << R"({"asset":{"version":"2.0"},)" << members << R"(,"accessors":[)";
        for (std::size_t i = 0; i < m_accessors.size(); ++i)
            json << (i ? "," : "") << m_accessors[i];
        json << R"(],"bufferViews":[)";
        for (std::size_t i = 0; i < m_views.size(); ++i)
            json << (i ? "," : "") << m_views[i];
        json << R"(],"buffers":[{"byteLength":)" << m_data.size()
             << R"(,"uri":"data:application/octet-stream;base64,)" << base64(m_data) << R"("}]})";
        const auto path = (std::filesystem::temp_directory_path() / name).string();
        std::ofstream(path) << json.str();
        return path;
    }

private:
    std::vector<std::uint8_t> m_data;
    std::vector<std::string> m_accessors;
    std::vector<std::string> m_views;

    std::size_t append(const void *data, std::size_t size)
    {
        while (m_data.size() % 4 != 0)
            m_data.push_back(0);
        const std::size_t offset = m_data.size();
        const auto *bytes = static_cast<const std::uint8_t *>(data);
        m_data.insert(m_data.end(), bytes, bytes + size);
        return offset;
    }

    int view(std::size_t offset, std::size_t size)
    {
        std::ostringstream v;
        v << R"({"buffer":0,"byteOffset":)" << offset << R"(,"byteLength":)" << size << '}';
        m_views.push_back(v.str());
        return static_cast<int>(m_views.size()) - 1;
    }

    static std::string join(const std::vector<float> &v)
    {
        std::ostringstream s;
        for (std::size_t i = 0; i < v.size(); ++i)
            s << (i ? "," : "") << v[i];
        return s.str();
    }
};

glm::vec3 vertex(const MeshNodePtr &node, int mesh, int index)
{
    return node->meshes()[static_cast<std::size_t>(mesh)]->vertices()->vertex(static_cast<GLuint>(index));
}

bool near(const glm::vec3 &a, const glm::vec3 &b, float eps = 1e-4f)
{
    return glm::length(a - b) <= eps;
}

} // namespace

IVF_TEST(hierarchy_meshes_and_materials)
{
    GltfWriter w;
    const int pos = w.floats({0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0}, "VEC3", true);
    const int idx = w.ushorts({0, 1, 2, 2, 1, 3}, "SCALAR");
    const std::string path = w.write("ivf2_test_hierarchy.gltf", R"(
        "scene":0,"scenes":[{"nodes":[0]}],
        "nodes":[{"name":"parent","translation":[1,0,0],"children":[1]},
                 {"name":"child","rotation":[0,0.7071068,0,0.7071068],"scale":[2,2,2],"mesh":0}],
        "meshes":[{"name":"quad","primitives":[{"attributes":{"POSITION":)" +
                                                           std::to_string(pos) + R"(},"indices":)" +
                                                           std::to_string(idx) + R"(,"material":0}]}],
        "materials":[{"name":"red","pbrMetallicRoughness":{"baseColorFactor":[1,0,0,1],"metallicFactor":0.2,
                      "roughnessFactor":0.7},"emissiveFactor":[0,0.5,0],"alphaMode":"MASK","alphaCutoff":0.3}])");

    auto result = GltfModel::load(path);
    IVF_CHECK(result.has_value());
    if (!result)
        return;
    auto model = *result;
    IVF_CHECK(model->nodeCount() == 2);
    IVF_CHECK(model->findNodeIndex("child") == 1);
    IVF_CHECK(model->findNode("missing") == nullptr);

    auto parent = model->findNode("parent");
    auto child = model->findNode("child");
    IVF_CHECK(near(parent->pos(), {1, 0, 0}));
    IVF_CHECK(near(child->scale(), {2, 2, 2}));
    IVF_CHECK(std::abs(glm::dot(child->rotation(), glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0)))) > 0.9999f);

    // The child's world transform: T(1,0,0) * Ry(90) * S(2)
    glm::mat4 world(1.0f);
    parent->applyLocalTransform(world);
    child->applyLocalTransform(world);
    const glm::vec3 p = glm::vec3(world * glm::vec4(1, 0, 0, 1));
    IVF_CHECK(near(p, {1, 0, -2}));

    auto meshes = model->meshNodes();
    IVF_CHECK(meshes.size() == 1);
    IVF_CHECK(meshes[0]->meshes().size() == 1);
    IVF_CHECK(near(vertex(meshes[0], 0, 3), {1, 1, 0}));

    auto material = std::dynamic_pointer_cast<PBRMaterial>(meshes[0]->meshes()[0]->material());
    IVF_CHECK(material != nullptr);
    if (material)
    {
        IVF_CHECK(near(glm::vec3(material->albedo()), {1, 0, 0}));
        IVF_CHECK_NEAR(material->metallic(), 0.2, 1e-6);
        IVF_CHECK_NEAR(material->roughness(), 0.7, 1e-6);
        IVF_CHECK(near(material->emissive(), {0, 0.5f, 0}));
        IVF_CHECK(material->alphaMode() == PBRMaterial::AlphaMode::Mask);
        IVF_CHECK_NEAR(material->alphaCutoff(), 0.3, 1e-6);
    }

    // ModelLoader hands glTF files to GltfModel
    auto viaLoader = ModelLoader::load(path);
    IVF_CHECK(viaLoader.has_value() && std::dynamic_pointer_cast<GltfModel>(*viaLoader) != nullptr);
    std::remove(path.c_str());
}

IVF_TEST(node_animation_linear_step_and_looping)
{
    GltfWriter w;
    const int times = w.floats({0, 1, 2}, "SCALAR", true);
    const int moves = w.floats({0, 0, 0, 2, 0, 0, 2, 4, 0}, "VEC3");
    const int angles = w.floats({0, 0, 0, 1, 0, 0.7071068f, 0, 0.7071068f, 0, 1, 0, 0}, "VEC4");
    const std::string path = w.write("ivf2_test_anim.gltf", R"(
        "scenes":[{"nodes":[0,1]}],
        "nodes":[{"name":"mover"},{"name":"turner"}],
        "animations":[{"name":"go","channels":[{"sampler":0,"target":{"node":0,"path":"translation"}},
                                               {"sampler":1,"target":{"node":1,"path":"rotation"}}],
                       "samplers":[{"input":)" + std::to_string(times) + R"(,"output":)" + std::to_string(moves) + R"(},
                                   {"input":)" + std::to_string(times) + R"(,"output":)" + std::to_string(angles) + R"(,
                                    "interpolation":"LINEAR"}]},
                      {"name":"steps","channels":[{"sampler":0,"target":{"node":0,"path":"translation"}}],
                       "samplers":[{"input":)" + std::to_string(times) + R"(,"output":)" + std::to_string(moves) + R"(,
                                    "interpolation":"STEP"}]}])");

    auto result = GltfModel::load(path);
    IVF_CHECK(result.has_value());
    if (!result)
        return;
    auto model = *result;
    IVF_CHECK(model->animationCount() == 2);
    IVF_CHECK(model->animationName(1) == "steps");
    IVF_CHECK(model->findAnimation("go") == 0);
    IVF_CHECK_NEAR(model->animationDuration(0), 2.0, 1e-6);

    model->setAnimation(0);
    model->setTime(0.5f);
    IVF_CHECK(near(model->node(0)->pos(), {1, 0, 0}));
    model->setTime(1.5f);
    IVF_CHECK(near(model->node(0)->pos(), {2, 2, 0}));
    // Rotation halfway to 90 degrees about Y: 45 degrees (spherical interpolation)
    model->setTime(0.5f);
    const glm::quat r = model->node(1)->rotation();
    IVF_CHECK(std::abs(glm::dot(r, glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0)))) > 0.9999f);

    // Looping wraps, holding keeps the last pose
    model->setTime(2.5f);
    IVF_CHECK(near(model->node(0)->pos(), {1, 0, 0}));
    model->setLooping(false);
    model->setTime(2.5f);
    IVF_CHECK(near(model->node(0)->pos(), {2, 4, 0}));

    model->setLooping(true);
    model->setAnimation(1);
    model->setTime(0.9f);
    IVF_CHECK(near(model->node(0)->pos(), {0, 0, 0}));
    model->setTime(1.1f);
    IVF_CHECK(near(model->node(0)->pos(), {2, 0, 0}));

    // No animation: rest pose
    model->setAnimation(-1);
    IVF_CHECK(near(model->node(0)->pos(), {0, 0, 0}));
    std::remove(path.c_str());
}

IVF_TEST(skinning_follows_the_joints)
{
    // A strip of two vertices per height, bound to two joints along Y
    GltfWriter w;
    const int pos = w.floats({0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 2, 0, 1, 2, 0}, "VEC3", true);
    const int idx = w.ushorts({0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5}, "SCALAR");
    const int joints = w.ushorts({0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}, "VEC4");
    const int weights = w.floats({1, 0, 0, 0, 1, 0, 0, 0, 0.5f, 0.5f, 0, 0, 0.5f, 0.5f, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
                                 "VEC4");
    // Inverse bind matrices: joint 0 at the origin, joint 1 at y = 1
    const int ibm = w.floats({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
                              1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1}, "MAT4");
    const std::string path = w.write("ivf2_test_skin.gltf", R"(
        "scenes":[{"nodes":[0,1]}],
        "nodes":[{"name":"skinned","mesh":0,"skin":0},
                 {"name":"root","children":[2]},
                 {"name":"tip","translation":[0,1,0]}],
        "skins":[{"joints":[1,2],"inverseBindMatrices":)" + std::to_string(ibm) + R"(}],
        "meshes":[{"primitives":[{"attributes":{"POSITION":)" + std::to_string(pos) + R"(,"JOINTS_0":)" +
                                     std::to_string(joints) + R"(,"WEIGHTS_0":)" + std::to_string(weights) +
                                     R"(},"indices":)" + std::to_string(idx) + R"(}]}])");

    auto result = GltfModel::load(path);
    IVF_CHECK(result.has_value());
    if (!result)
        return;
    auto model = *result;
    IVF_CHECK(model->hasDeformation());
    auto mesh = model->meshNodes().at(0);

    // Bind pose: vertices unchanged
    IVF_CHECK(near(vertex(mesh, 0, 5), {1, 2, 0}));

    // Turn the tip joint 90 degrees about Z: the top vertices swing around (0, 1, 0)
    auto tip = model->findNode("tip");
    tip->setRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 0, 1)));
    model->update();
    IVF_CHECK(near(vertex(mesh, 0, 4), {-1, 1, 0}));
    IVF_CHECK(near(vertex(mesh, 0, 5), {-1, 2, 0}));
    // Vertices weighted half/half end between the two poses
    IVF_CHECK(near(vertex(mesh, 0, 2), {0, 1, 0}));
    IVF_CHECK(near(vertex(mesh, 0, 3), {0.5f, 1.5f, 0}));
    // Joint 0 alone: unchanged
    IVF_CHECK(near(vertex(mesh, 0, 1), {1, 0, 0}));
    std::remove(path.c_str());
}

IVF_TEST(morph_targets_blend_with_weights)
{
    GltfWriter w;
    const int pos = w.floats({0, 0, 0, 1, 0, 0, 0, 1, 0}, "VEC3", true);
    const int target = w.floats({0, 0, 0, 0, 0, 0, 0, 2, 0}, "VEC3", true);
    const int times = w.floats({0, 1}, "SCALAR", true);
    const int values = w.floats({0, 1}, "SCALAR");
    const std::string path = w.write("ivf2_test_morph.gltf", R"(
        "scenes":[{"nodes":[0]}],
        "nodes":[{"name":"blob","mesh":0}],
        "meshes":[{"weights":[0.25],"primitives":[{"attributes":{"POSITION":)" + std::to_string(pos) +
                                     R"(},"targets":[{"POSITION":)" + std::to_string(target) + R"(}]}]}],
        "animations":[{"channels":[{"sampler":0,"target":{"node":0,"path":"weights"}}],
                       "samplers":[{"input":)" + std::to_string(times) + R"(,"output":)" + std::to_string(values) + R"(}]}])");

    auto result = GltfModel::load(path);
    IVF_CHECK(result.has_value());
    if (!result)
        return;
    auto model = *result;
    auto mesh = model->meshNodes().at(0);

    // Default weight from the mesh: a quarter of the target
    IVF_CHECK(near(vertex(mesh, 0, 2), {0, 1.5f, 0}));
    IVF_CHECK(model->morphWeights(0).size() == 1);

    model->setMorphWeights(0, {1.0f});
    model->update();
    IVF_CHECK(near(vertex(mesh, 0, 2), {0, 3, 0}));

    // Animated weight
    model->setAnimation(0);
    model->setLooping(false);
    model->setTime(0.5f);
    IVF_CHECK(near(vertex(mesh, 0, 2), {0, 2, 0}));
    std::remove(path.c_str());
}

IVF_TEST(errors)
{
    auto missing = GltfModel::load("does_not_exist.glb");
    IVF_CHECK(!missing && missing.error().code == ErrorCode::FileNotFound);

    const auto bad = (std::filesystem::temp_directory_path() / "ivf2_test_bad.gltf").string();
    std::ofstream(bad) << "{ this is not json";
    auto invalid = GltfModel::load(bad);
    IVF_CHECK(!invalid && invalid.error().code == ErrorCode::UnsupportedFormat);

    const auto draco = (std::filesystem::temp_directory_path() / "ivf2_test_draco.gltf").string();
    std::ofstream(draco) << R"({"asset":{"version":"2.0"},"extensionsUsed":["KHR_draco_mesh_compression"],
                              "extensionsRequired":["KHR_draco_mesh_compression"]})";
    auto unsupported = GltfModel::load(draco);
    IVF_CHECK(!unsupported && unsupported.error().code == ErrorCode::NotSupported);

    std::remove(bad.c_str());
    std::remove(draco.c_str());
}

IVF_TEST_MAIN()
