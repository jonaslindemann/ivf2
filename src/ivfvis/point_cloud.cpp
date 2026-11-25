#include <ivfvis/point_cloud.h>

#include <ivf/draw_context.h>
#include <ivf/gl_debug.h>
#include <ivf/light_manager.h>
#include <ivf/picking.h>
#include <ivf/program.h>
#include <ivf/ray.h>
#include <ivf/render_context.h>
#include <ivf/render_queue.h>
#include <ivf/selection_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>
#include <ivf/uniform_blocks.h>

#include <glad/glad.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

using namespace ivfvis;

namespace {

const char *const ProgramName = "ivfvis_points";
constexpr int ColormapEntries = 256;

const std::string point_vert = std::string(R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aValue;
layout (location = 2) in float aSize;
)") + IVF_CAMERA_BLOCK_GLSL + R"(
uniform mat4 model;
uniform float pointSize;
uniform bool worldSize;
uniform float viewportHeight;

out float vValue;

void main()
{
    vec4 eye = view * model * vec4(aPos, 1.0);
    gl_Position = projection * eye;
    float size = pointSize * aSize;
    if (worldSize)
    {
        // Diameter in world units: pixels per unit at this depth (orthographic: everywhere)
        float scale = length(vec3(model[0]));
        float depth = projection[3][3] > 0.5 ? 1.0 : max(-eye.z, 1e-4);
        size *= scale * projection[1][1] * viewportHeight * 0.5 / depth;
    }
    gl_PointSize = max(size, 1.0);
    vValue = aValue;
}
)";

const std::string point_frag = R"(#version 330 core
in float vValue;
out vec4 fragColor;

uniform int shape;            // 0 disc, 1 sphere, 2 square
uniform bool useColormap;
uniform vec4 solidColor;
uniform sampler2D colormap;
uniform float colormapSize;
uniform vec2 range;
uniform bool logScale;
uniform bool useBelow;
uniform vec4 belowColor;
uniform bool useAbove;
uniform vec4 aboveColor;
uniform vec4 nanColor;
uniform float opacity;
uniform bool selectionRendering;
uniform uint objectId;

vec4 mapValue(float v)
{
    if (isnan(v))
        return nanColor;
    float t;
    if (logScale)
        t = v <= 0.0 ? -1.0 : (log(v) - log(range.x)) / (log(range.y) - log(range.x));
    else if (range.y > range.x)
        t = (v - range.x) / (range.y - range.x);
    else
        t = v < range.x ? -1.0 : (v > range.y ? 2.0 : 0.5);
    if (t < 0.0 && useBelow)
        return belowColor;
    if (t > 1.0 && useAbove)
        return aboveColor;
    t = clamp(t, 0.0, 1.0);
    return texture(colormap, vec2((t * (colormapSize - 1.0) + 0.5) / colormapSize, 0.5));
}

void main()
{
    vec2 c = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(c, c);
    if (shape != 2 && r2 > 1.0)
        discard;

    if (selectionRendering)
    {
        uint mask = uint(255);
        fragColor = vec4(float(objectId & mask) / 255.0, float((objectId >> uint(8)) & mask) / 255.0,
                         float((objectId >> uint(16)) & mask) / 255.0, 1.0);
        return;
    }

    vec4 color = useColormap ? mapValue(vValue) : solidColor;
    if (shape == 1)
    {
        // Shade the disc like a sphere facing the viewer
        vec3 n = vec3(c.x, -c.y, sqrt(max(1.0 - r2, 0.0)));
        vec3 l = normalize(vec3(0.4, 0.6, 0.7));
        float diffuse = max(dot(n, l), 0.0);
        float specular = pow(max(dot(reflect(-l, n), vec3(0.0, 0.0, 1.0)), 0.0), 32.0);
        color.rgb = color.rgb * (0.3 + 0.7 * diffuse) + vec3(0.25 * specular);
    }
    fragColor = vec4(color.rgb, color.a * opacity);
}
)";

} // namespace

struct PointCloud::Gpu {
    ivf::RenderContext *context{nullptr};
    std::weak_ptr<const void> lifetime;
    GLuint vao{0};
    GLuint vbo{0};
    GLuint texture{0};
    GLsizei count{0};
    std::uint64_t dataVersion{0};
    const Colormap *colormap{nullptr};
    std::uint64_t colormapVersion{0};
};

PointCloud::PointCloud(PointSetPtr points) : m_points(std::move(points))
{
    setName("PointCloud");
    setCastShadows(false);
    update();
}

PointCloud::~PointCloud()
{
    for (auto &gpu : m_gpu)
        release(*gpu);
}

std::shared_ptr<PointCloud> PointCloud::create(PointSetPtr points)
{
    return std::make_shared<PointCloud>(std::move(points));
}

void PointCloud::setPoints(PointSetPtr points)
{
    m_points = std::move(points);
    update();
}

PointSetPtr PointCloud::points() const
{
    return m_points;
}

void PointCloud::update()
{
    ++m_dataVersion;
    rebuildSizes();

    // Bounds for culling, grown by the largest point radius when it is in world units
    ivf::BoundingBox box = m_points ? m_points->bounds() : ivf::BoundingBox();
    if (box.isValid() && m_sizeMode == PointSizeMode::World)
    {
        const glm::vec3 r(0.5f * m_pointSize * m_maxSizeFactor);
        box = ivf::BoundingBox(box.min() - r, box.max() + r);
    }
    setLocalBoundingBox(box);
}

void PointCloud::rebuildSizes()
{
    m_sizes.clear();
    m_maxSizeFactor = 1.0f;
    if (m_sizeBy.empty() || !m_points || !m_points->hasScalars(m_sizeBy))
        return;
    const auto values = m_points->scalars(m_sizeBy);
    const ValueRange range = m_points->range(m_sizeBy);
    m_sizes.resize(values.size(), m_maxScale);
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        const float v = values[i];
        const float t = range.length() > 0.0f && v == v ? (v - range.min) / range.length() : 1.0f;
        m_sizes[i] = m_minScale + std::clamp(t, 0.0f, 1.0f) * (m_maxScale - m_minScale);
    }
    m_maxSizeFactor = std::max(m_minScale, m_maxScale);
}

void PointCloud::setColorBy(const std::string &name, ColormapPtr colormap)
{
    m_colorBy = name;
    m_colormap = std::move(colormap);
    ++m_dataVersion;
}

void PointCloud::clearColorBy()
{
    m_colorBy.clear();
    ++m_dataVersion;
}

const std::string &PointCloud::colorBy() const
{
    return m_colorBy;
}

void PointCloud::setColormap(ColormapPtr colormap)
{
    m_colormap = std::move(colormap);
}

ColormapPtr PointCloud::colormap() const
{
    return m_colormap;
}

void PointCloud::setColor(const glm::vec3 &color)
{
    m_color = color;
}

glm::vec3 PointCloud::color() const
{
    return m_color;
}

void PointCloud::setSizeBy(const std::string &name, float minScale, float maxScale)
{
    m_sizeBy = name;
    m_minScale = std::max(minScale, 0.0f);
    m_maxScale = std::max(maxScale, 0.0f);
    update();
}

void PointCloud::clearSizeBy()
{
    m_sizeBy.clear();
    update();
}

const std::string &PointCloud::sizeBy() const
{
    return m_sizeBy;
}

void PointCloud::setPointSize(float size)
{
    m_pointSize = std::max(size, 0.0f);
    update(); // bounds
}

float PointCloud::pointSize() const
{
    return m_pointSize;
}

void PointCloud::setSizeMode(PointSizeMode mode)
{
    m_sizeMode = mode;
    update(); // bounds
}

PointSizeMode PointCloud::sizeMode() const
{
    return m_sizeMode;
}

void PointCloud::setShape(PointShape shape)
{
    m_shape = shape;
}

PointShape PointCloud::shape() const
{
    return m_shape;
}

void PointCloud::setOpacity(float opacity)
{
    m_opacity = std::clamp(opacity, 0.0f, 1.0f);
    setRenderLayer(m_opacity < 1.0f ? ivf::RenderLayer::Transparent : ivf::RenderLayer::Opaque);
}

float PointCloud::opacity() const
{
    return m_opacity;
}

std::size_t PointCloud::pointCount() const
{
    return m_points ? m_points->pointCount() : 0;
}

PointCloud::Gpu *PointCloud::gpu(ivf::RenderContext &context)
{
    std::erase_if(m_gpu, [](const std::unique_ptr<Gpu> &gpu) { return gpu->lifetime.expired(); });
    for (auto &gpu : m_gpu)
        if (gpu->context == &context)
            return gpu.get();
    auto gpu = std::make_unique<Gpu>();
    gpu->context = &context;
    gpu->lifetime = context.lifetime();
    m_gpu.push_back(std::move(gpu));
    return m_gpu.back().get();
}

void PointCloud::upload(Gpu &gpu)
{
    const std::size_t n = pointCount();
    const bool colored = !m_colorBy.empty() && m_points && m_points->hasScalars(m_colorBy);
    const std::span<const float> values = colored ? std::as_const(*m_points).scalars(m_colorBy) : std::span<const float>();

    // Interleaved: position, value, size factor
    std::vector<float> data;
    data.reserve(n * 5);
    for (std::size_t i = 0; i < n; ++i)
    {
        const glm::vec3 p = m_points->position(i);
        data.insert(data.end(), {p.x, p.y, p.z, colored ? values[i] : 0.0f, m_sizes.empty() ? 1.0f : m_sizes[i]});
    }

    if (gpu.vao == 0)
    {
        glGenVertexArrays(1, &gpu.vao);
        glGenBuffers(1, &gpu.vbo);
        glBindVertexArray(gpu.vao);
        glBindBuffer(GL_ARRAY_BUFFER, gpu.vbo);
        ivf::glLabel(GL_VERTEX_ARRAY, gpu.vao, "PointCloud");
        ivf::glLabel(GL_BUFFER, gpu.vbo, "PointCloud points");
        const GLsizei stride = 5 * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(4 * sizeof(float)));
        glBindVertexArray(0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, gpu.vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(float)), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    gpu.count = static_cast<GLsizei>(n);
    gpu.dataVersion = m_dataVersion;
}

void PointCloud::uploadColormap(Gpu &gpu)
{
    const auto bytes = m_colormap->rgba8(ColormapEntries);
    if (gpu.texture == 0)
    {
        glGenTextures(1, &gpu.texture);
        glBindTexture(GL_TEXTURE_2D, gpu.texture);
        ivf::glLabel(GL_TEXTURE, gpu.texture, "PointCloud colormap");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, gpu.texture);
    GLint alignmentWas = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignmentWas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ColormapEntries, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignmentWas);
    // Discrete steps stay crisp with nearest sampling
    const GLint filter = m_colormap->steps() > 0 ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glBindTexture(GL_TEXTURE_2D, 0);
    gpu.colormap = m_colormap.get();
    gpu.colormapVersion = m_colormap->version();
}

void PointCloud::release(Gpu &gpu)
{
    if (gpu.lifetime.expired() || gpu.context == nullptr)
        return; // the context took its objects with it
    const GLuint vao = gpu.vao, vbo = gpu.vbo, texture = gpu.texture;
    auto destroy = [vao, vbo, texture]() {
        if (vao != 0)
            glDeleteVertexArrays(1, &vao);
        if (vbo != 0)
            glDeleteBuffers(1, &vbo);
        if (texture != 0)
            glDeleteTextures(1, &texture);
    };
    if (gpu.context->isCurrent())
        destroy();
    else
        gpu.context->deleteLater(destroy);
}

void PointCloud::doDrawPass(const ivf::DrawContext &context)
{
    if (context.pass == ivf::RenderPass::Shadow || pointCount() == 0)
        return;

    auto *shaders = context.context.shaders();
    auto program = shaders->program(ProgramName);
    if (!program)
    {
        auto previous = shaders->currentProgram();
        auto result = shaders->compileProgramFromStrings(point_vert, point_frag, ProgramName, false);
        if (previous)
            shaders->setCurrentProgram(previous);
        if (!result)
            return; // logged
        program = *result;
    }

    Gpu &g = *gpu(context.context);
    if (g.dataVersion != m_dataVersion)
        upload(g);
    const bool useColormap = !m_colorBy.empty() && m_colormap && m_points->hasScalars(m_colorBy);
    if (useColormap && (g.colormap != m_colormap.get() || g.colormapVersion != m_colormap->version()))
        uploadColormap(g);

    GLint viewport[4] = {0, 0, 1, 1};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const float viewportHeight = static_cast<float>(viewport[3]);
    if (context.pass == ivf::RenderPass::Main)
    {
        m_lastProjectionScale = context.projection[1][1] * viewportHeight * 0.5f;
        m_lastOrthographic = context.projection[3][3] > 0.5f;
    }

    auto previous = shaders->currentProgram();
    shaders->setCurrentProgram(program);

    program->uniformMatrix4("model", context.item.world);
    program->uniformFloat("pointSize", m_pointSize);
    program->uniformBool("worldSize", m_sizeMode == PointSizeMode::World);
    program->uniformFloat("viewportHeight", viewportHeight);
    program->uniformInt("shape", static_cast<int>(m_shape));
    program->uniformBool("useColormap", useColormap);
    program->uniformVec4f("solidColor", m_color.r, m_color.g, m_color.b, 1.0f);
    program->uniformFloat("opacity", m_opacity);
    program->uniformBool("selectionRendering", context.pass == ivf::RenderPass::Selection);
    program->uniformUInt("objectId", static_cast<int>(objectId()));
    if (useColormap)
    {
        const ValueRange range = m_colormap->range();
        const bool log = m_colormap->scale() == ColormapScale::Log && range.min > 0.0f && range.max > range.min;
        program->uniformInt("colormap", 0);
        program->uniformFloat("colormapSize", static_cast<float>(ColormapEntries));
        glUniform2f(program->uniformLoc("range"), range.min, range.max);
        program->uniformBool("logScale", log);
        const auto below = m_colormap->belowRangeColor();
        const auto above = m_colormap->aboveRangeColor();
        const glm::vec4 b = below.value_or(glm::vec4(0.0f)), a = above.value_or(glm::vec4(0.0f));
        const glm::vec4 nan = m_colormap->nanColor();
        program->uniformBool("useBelow", below.has_value());
        program->uniformVec4f("belowColor", b.r, b.g, b.b, b.a);
        program->uniformBool("useAbove", above.has_value());
        program->uniformVec4f("aboveColor", a.r, a.g, a.b, a.a);
        program->uniformVec4f("nanColor", nan.r, nan.g, nan.b, nan.a);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g.texture);
    }

    const GLboolean pointSizeWas = glIsEnabled(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(g.vao);
    glDrawArrays(GL_POINTS, 0, g.count);
    context.context.countDrawCall();
    glBindVertexArray(0);
    if (!pointSizeWas)
        glDisable(GL_PROGRAM_POINT_SIZE);
    if (useColormap)
        glBindTexture(GL_TEXTURE_2D, 0);

    if (previous)
    {
        shaders->setCurrentProgram(previous);
        context.context.transforms()->refreshForProgram();
        context.context.lights()->refreshForProgram();
        context.context.selection()->refreshForProgram();
    }
}

std::optional<bool> PointCloud::intersectRay(const ivf::Ray &ray, const glm::mat4 &world, float maxDistance,
                                             ivf::RayHit &hit) const
{
    const std::size_t n = pointCount();
    if (n == 0)
        return false;

    // Radius of point i at ray distance t, in world units
    const float worldScale = glm::length(glm::vec3(world[0]));
    const bool pixels = m_sizeMode == PointSizeMode::Pixels;
    if (pixels && m_lastProjectionScale <= 0.0f)
        return false; // never drawn: the size on screen is unknown
    auto radius = [&](std::size_t i, float t) {
        const float factor = m_sizes.empty() ? 1.0f : m_sizes[i];
        if (!pixels)
            return 0.5f * m_pointSize * factor * worldScale;
        const float depth = m_lastOrthographic ? 1.0f : std::max(t, 1e-4f);
        return 0.5f * std::max(m_pointSize * factor, 1.0f) * depth / m_lastProjectionScale;
    };

    float bestT = maxDistance;
    std::size_t best = n;
    glm::vec3 bestCenter(0.0f);
    for (std::size_t i = 0; i < n; ++i)
    {
        const glm::vec3 c = glm::vec3(world * glm::vec4(m_points->position(i), 1.0f));
        const glm::vec3 oc = c - ray.origin;
        const float along = glm::dot(oc, ray.direction);
        if (along <= 0.0f)
            continue;
        const float d2 = glm::dot(oc, oc) - along * along;
        const float r = radius(i, along);
        if (d2 > r * r)
            continue;
        const float t = std::max(along - std::sqrt(std::max(r * r - d2, 0.0f)), 0.0f);
        if (t < bestT)
        {
            bestT = t;
            best = i;
            bestCenter = c;
        }
    }
    if (best == n)
        return false;

    hit.mesh = nullptr;
    hit.distance = bestT;
    hit.position = ray.at(bestT);
    const glm::vec3 out = hit.position - bestCenter;
    hit.normal = glm::length(out) > 1e-6f ? glm::normalize(out) : -ray.direction;
    hit.triangle = -1;
    hit.point = static_cast<int>(best);
    hit.barycentric = glm::vec2(0.0f);
    hit.exact = true;
    return true;
}
