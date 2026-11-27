#include <ivfvis/volume.h>

#include <ivf/draw_context.h>
#include <ivf/gl_debug.h>
#include <ivf/light_manager.h>
#include <ivf/logger.h>
#include <ivf/program.h>
#include <ivf/render_context.h>
#include <ivf/render_queue.h>
#include <ivf/selection_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>
#include <ivf/uniform_blocks.h>
#include <ivf/utils.h>

#include <glad/glad.h>
#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>
#include <array>
#include <cmath>

using namespace ivfvis;

// ---------------------------------------------------------------------------------------
// Transfer function
// ---------------------------------------------------------------------------------------

float ivfvis::opacityAt(const std::vector<glm::vec2> &points, float t)
{
    if (points.empty())
        return t;
    if (t <= points.front().x)
        return std::clamp(points.front().y, 0.0f, 1.0f);
    for (std::size_t i = 1; i < points.size(); ++i)
        if (t <= points[i].x)
        {
            const glm::vec2 a = points[i - 1], b = points[i];
            const float f = b.x > a.x ? (t - a.x) / (b.x - a.x) : 1.0f;
            return std::clamp(a.y + f * (b.y - a.y), 0.0f, 1.0f);
        }
    return std::clamp(points.back().y, 0.0f, 1.0f);
}

std::vector<glm::vec4> ivfvis::transferTable(const Colormap &colormap, const std::vector<glm::vec2> &opacity, int n)
{
    std::vector<glm::vec4> table = colormap.table(n);
    for (std::size_t i = 0; i < table.size(); ++i)
    {
        const float t = n > 1 ? static_cast<float>(i) / static_cast<float>(n - 1) : 0.5f;
        table[i].a = opacityAt(opacity, t);
    }
    return table;
}

// ---------------------------------------------------------------------------------------
// Shaders
// ---------------------------------------------------------------------------------------

namespace {

const char *const ProgramName = "ivfvis_volume";
constexpr int TransferSize = 256;
constexpr int VolumeUnit = 0, TransferUnit = 1, DepthUnit = 2;

const std::string volume_vert = std::string(R"(#version 330 core
layout (location = 0) in vec3 aPos;
)") + IVF_CAMERA_BLOCK_GLSL + R"(
uniform mat4 model;
out vec3 vLocal;
void main()
{
    vLocal = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const std::string volume_frag = R"(#version 330 core
in vec3 vLocal;
out vec4 fragColor;

uniform sampler3D volume;
uniform sampler2D transfer;
uniform sampler2D sceneDepth;
uniform bool useSceneDepth;
uniform vec4 viewport;
uniform mat4 invViewProjection;
uniform mat4 invModel;
uniform vec3 eye;              // camera in the node's coordinates
uniform vec3 boxMin;
uniform vec3 boxMax;
uniform vec3 texScale;
uniform vec3 texOffset;
uniform vec3 texelSize;
uniform vec2 range;
uniform bool logScale;
uniform float extinction;
uniform float stepSize;
uniform int maxSteps;
uniform bool shading;

// Random jitter of the ray starts per pixel, against banding (noise is less visible than rings)
float jitter(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float position(float v)
{
    if (logScale)
        return v <= 0.0 ? 0.0 : clamp((log(v) - log(range.x)) / (log(range.y) - log(range.x)), 0.0, 1.0);
    if (range.y <= range.x)
        return 0.5;
    return clamp((v - range.x) / (range.y - range.x), 0.0, 1.0);
}

void main()
{
    vec3 dir = normalize(vLocal - eye);

    // Where the ray is inside the box
    vec3 invDir = 1.0 / dir;
    vec3 ta = (boxMin - eye) * invDir;
    vec3 tb = (boxMax - eye) * invDir;
    vec3 tmin = min(ta, tb);
    vec3 tmax = max(ta, tb);
    float tNear = max(max(max(tmin.x, tmin.y), tmin.z), 0.0);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);

    // Stop at opaque geometry
    if (useSceneDepth)
    {
        vec2 uv = (gl_FragCoord.xy - viewport.xy) / viewport.zw;
        float d = texture(sceneDepth, uv).r;
        if (d < 1.0)
        {
            vec4 world = invViewProjection * vec4(uv * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
            vec3 local = (invModel * vec4(world.xyz / world.w, 1.0)).xyz;
            tFar = min(tFar, dot(local - eye, dir));
        }
    }
    if (tFar <= tNear)
        discard;

    vec3 color = vec3(0.0);
    float alpha = 0.0;
    float t = tNear + jitter(gl_FragCoord.xy) * stepSize;
    for (int i = 0; i < maxSteps && t < tFar; ++i, t += stepSize)
    {
        vec3 tc = (eye + dir * t) * texScale + texOffset;
        float v = texture(volume, tc).r;
        vec4 c = texture(transfer, vec2((position(v) * 255.0 + 0.5) / 256.0, 0.5));
        if (c.a <= 0.0)
            continue;
        float a = 1.0 - exp(-c.a * extinction * stepSize);
        vec3 rgb = c.rgb;
        if (shading)
        {
            vec3 g = vec3(texture(volume, tc + vec3(texelSize.x, 0.0, 0.0)).r - texture(volume, tc - vec3(texelSize.x, 0.0, 0.0)).r,
                          texture(volume, tc + vec3(0.0, texelSize.y, 0.0)).r - texture(volume, tc - vec3(0.0, texelSize.y, 0.0)).r,
                          texture(volume, tc + vec3(0.0, 0.0, texelSize.z)).r - texture(volume, tc - vec3(0.0, 0.0, texelSize.z)).r);
            if (length(g) > 1e-6)
                rgb *= 0.3 + 0.7 * abs(dot(normalize(g), dir));
        }
        color += (1.0 - alpha) * a * rgb;
        alpha += (1.0 - alpha) * a;
        if (alpha > 0.99)
            break;
    }
    if (alpha < 0.002)
        discard;
    fragColor = vec4(color / alpha, alpha);
}
)";

} // namespace

struct Volume::Gpu {
    ivf::RenderContext *context{nullptr};
    std::weak_ptr<const void> lifetime;
    GLuint vao{0}, vbo{0}, ebo{0};
    glm::vec3 boxMin{0.0f}, boxMax{0.0f};
    GLuint transfer{0};
    const Colormap *transferColormap{nullptr};
    std::uint64_t transferVersion{0}, transferOpacity{0};
    // Copy of the scene's depth
    GLuint depthFbo{0}, depthTexture{0};
    int depthWidth{0}, depthHeight{0};
    GLenum depthFormat{0};
    bool depthFailed{false};
};

Volume::Volume(UniformGridPtr grid, std::string scalars, ColormapPtr colormap)
    : m_grid(std::move(grid)), m_scalars(std::move(scalars)), m_colormap(std::move(colormap)),
      m_texture(ivf::Texture3D::create())
{
    setName("Volume");
    setCastShadows(false);
    setRenderLayer(ivf::RenderLayer::Transparent);
    update();
}

Volume::~Volume()
{
    for (auto &gpu : m_gpu)
        release(*gpu);
}

std::shared_ptr<Volume> Volume::create(UniformGridPtr grid, std::string scalars, ColormapPtr colormap)
{
    return std::make_shared<Volume>(std::move(grid), std::move(scalars), std::move(colormap));
}

void Volume::setGrid(UniformGridPtr grid)
{
    m_grid = std::move(grid);
    update();
}

UniformGridPtr Volume::grid() const
{
    return m_grid;
}

void Volume::setScalars(const std::string &scalars)
{
    m_scalars = scalars;
    update();
}

const std::string &Volume::scalars() const
{
    return m_scalars;
}

void Volume::setColormap(ColormapPtr colormap)
{
    m_colormap = std::move(colormap);
}

ColormapPtr Volume::colormap() const
{
    return m_colormap;
}

void Volume::setOpacity(std::vector<glm::vec2> points)
{
    std::sort(points.begin(), points.end(), [](const glm::vec2 &a, const glm::vec2 &b) { return a.x < b.x; });
    m_opacity = std::move(points);
    ++m_opacityVersion;
}

const std::vector<glm::vec2> &Volume::opacity() const
{
    return m_opacity;
}

void Volume::setDensity(float density)
{
    m_density = std::max(density, 0.0f);
}

float Volume::density() const
{
    return m_density;
}

void Volume::setSamples(int samples)
{
    m_samples = std::clamp(samples, 8, 4096);
}

int Volume::samples() const
{
    return m_samples;
}

void Volume::setShading(bool shading)
{
    m_shading = shading;
}

bool Volume::shading() const
{
    return m_shading;
}

void Volume::update()
{
    if (!m_grid || m_scalars.empty())
    {
        setLocalBoundingBox(ivf::BoundingBox());
        return;
    }
    m_texture->setData(m_grid->dimensions(), m_grid->scalars(m_scalars));
    setLocalBoundingBox(m_grid->bounds());
}

ivf::Texture3DPtr Volume::texture() const
{
    return m_texture;
}

Volume::Gpu &Volume::gpu(ivf::RenderContext &context)
{
    std::erase_if(m_gpu, [](const std::unique_ptr<Gpu> &gpu) { return gpu->lifetime.expired(); });
    for (auto &gpu : m_gpu)
        if (gpu->context == &context)
            return *gpu;
    auto gpu = std::make_unique<Gpu>();
    gpu->context = &context;
    gpu->lifetime = context.lifetime();
    m_gpu.push_back(std::move(gpu));
    return *m_gpu.back();
}

void Volume::release(Gpu &gpu)
{
    if (gpu.lifetime.expired() || gpu.context == nullptr)
        return;
    const GLuint vao = gpu.vao, vbo = gpu.vbo, ebo = gpu.ebo, transfer = gpu.transfer, fbo = gpu.depthFbo,
                 depth = gpu.depthTexture;
    auto destroy = [=]() {
        if (vao != 0)
            glDeleteVertexArrays(1, &vao);
        for (GLuint buffer : {vbo, ebo})
            if (buffer != 0)
                glDeleteBuffers(1, &buffer);
        for (GLuint texture : {transfer, depth})
            if (texture != 0)
                glDeleteTextures(1, &texture);
        if (fbo != 0)
            glDeleteFramebuffers(1, &fbo);
    };
    if (gpu.context->isCurrent())
        destroy();
    else
        gpu.context->deleteLater(destroy);
}

bool Volume::copySceneDepth(Gpu &gpu, int width, int height)
{
    if (gpu.depthFailed)
        return false;

    GLint drawFbo = 0, readFbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(drawFbo));

    // The copy must have the same depth format as the framebuffer being drawn
    const GLenum depthAttachment = drawFbo == 0 ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
    const GLenum stencilAttachment = drawFbo == 0 ? GL_STENCIL : GL_STENCIL_ATTACHMENT;
    GLint type = GL_NONE, depthBits = 0, stencilType = GL_NONE, stencilBits = 0, component = 0;
    glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, depthAttachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &type);
    if (type != GL_NONE)
    {
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, depthAttachment,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits);
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, depthAttachment,
                                              GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &component);
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, stencilAttachment,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &stencilType);
        if (stencilType != GL_NONE)
            glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, stencilAttachment,
                                                  GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencilBits);
    }
    if (type == GL_NONE || depthBits == 0)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFbo));
        return false; // no depth buffer: nothing to stop at
    }

    const bool stencil = stencilBits > 0;
    const bool isFloat = component == GL_FLOAT;
    GLenum internal, format, dataType;
    if (isFloat)
    {
        internal = stencil ? GL_DEPTH32F_STENCIL8 : GL_DEPTH_COMPONENT32F;
        dataType = stencil ? GL_FLOAT_32_UNSIGNED_INT_24_8_REV : GL_FLOAT;
    }
    else if (depthBits <= 16 && !stencil)
    {
        internal = GL_DEPTH_COMPONENT16;
        dataType = GL_UNSIGNED_SHORT;
    }
    else
    {
        internal = stencil ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT24;
        dataType = stencil ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_INT;
    }
    format = stencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT;

    if (gpu.depthFbo == 0)
        glGenFramebuffers(1, &gpu.depthFbo);
    if (gpu.depthTexture == 0 || gpu.depthWidth != width || gpu.depthHeight != height || gpu.depthFormat != internal)
    {
        if (gpu.depthTexture == 0)
            glGenTextures(1, &gpu.depthTexture);
        glActiveTexture(GL_TEXTURE0 + DepthUnit);
        glBindTexture(GL_TEXTURE_2D, gpu.depthTexture);
        ivf::glLabel(GL_TEXTURE, gpu.depthTexture, "Volume scene depth");
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internal), width, height, 0, format, dataType, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);

        // Our framebuffer for both reading and drawing while it is set up (no colour buffers)
        glBindFramebuffer(GL_FRAMEBUFFER, gpu.depthFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, gpu.depthTexture, 0);
        const GLenum none = GL_NONE;
        glDrawBuffers(1, &none); // glDrawBuffer() is not in OpenGL ES
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
        gpu.depthWidth = width;
        gpu.depthHeight = height;
        gpu.depthFormat = internal;
    }

    ivf::clearError();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gpu.depthFbo);
    GLint viewport[4] = {0, 0, width, height};
    glGetIntegerv(GL_VIEWPORT, viewport);
    glBlitFramebuffer(viewport[0], viewport[1], viewport[0] + width, viewport[1] + height, 0, 0, width, height,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    const bool ok = glGetError() == GL_NO_ERROR;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFbo));
    if (!ok)
    {
        gpu.depthFailed = true;
        ivf::logWarning("Could not copy the scene's depth: volumes are drawn over opaque objects", "Volume");
    }
    return ok;
}

void Volume::doDrawPass(const ivf::DrawContext &context)
{
    // Only the camera's image (the copy of its depth belongs to it)
    if (context.pass != ivf::RenderPass::Main || !m_colormap || !m_grid || m_scalars.empty())
        return;
    const ivf::BoundingBox box = m_grid->bounds();
    const glm::vec3 size = box.size();
    if (!box.isValid() || glm::length(size) <= 0.0f)
        return;

    auto *shaders = context.context.shaders();
    auto program = shaders->program(ProgramName);
    if (!program)
    {
        auto previous = shaders->currentProgram();
        auto result = shaders->compileProgramFromStrings(volume_vert, volume_frag, ProgramName, false);
        if (previous)
            shaders->setCurrentProgram(previous);
        if (!result)
            return; // logged
        program = *result;
    }

    Gpu &g = gpu(context.context);

    // The box (in the grid's coordinates), faces wound counter-clockwise seen from outside
    if (g.vao == 0 || g.boxMin != box.min() || g.boxMax != box.max())
    {
        const glm::vec3 lo = box.min(), hi = box.max();
        std::array<glm::vec3, 8> corners;
        for (int c = 0; c < 8; ++c)
            corners[static_cast<std::size_t>(c)] = glm::vec3(c & 1 ? hi.x : lo.x, c & 2 ? hi.y : lo.y, c & 4 ? hi.z : lo.z);
        const int faces[6][4] = {{0, 2, 6, 4}, {1, 3, 7, 5}, {0, 1, 5, 4}, {2, 3, 7, 6}, {0, 1, 3, 2}, {4, 5, 7, 6}};
        const glm::vec3 centre = 0.5f * (lo + hi);
        std::vector<GLuint> indices;
        for (const auto &f : faces)
        {
            const glm::vec3 a = corners[static_cast<std::size_t>(f[0])], b = corners[static_cast<std::size_t>(f[1])],
                            c = corners[static_cast<std::size_t>(f[2])];
            const bool outward = glm::dot(glm::cross(b - a, c - a), a - centre) > 0.0f;
            const GLuint q[4] = {static_cast<GLuint>(f[0]), static_cast<GLuint>(f[1]), static_cast<GLuint>(f[2]),
                                 static_cast<GLuint>(f[3])};
            if (outward)
                indices.insert(indices.end(), {q[0], q[1], q[2], q[0], q[2], q[3]});
            else
                indices.insert(indices.end(), {q[0], q[2], q[1], q[0], q[3], q[2]});
        }
        if (g.vao == 0)
        {
            glGenVertexArrays(1, &g.vao);
            glGenBuffers(1, &g.vbo);
            glGenBuffers(1, &g.ebo);
        }
        glBindVertexArray(g.vao);
        ivf::glLabel(GL_VERTEX_ARRAY, g.vao, "Volume box");
        glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)), indices.data(),
                     GL_STATIC_DRAW);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        g.boxMin = lo;
        g.boxMax = hi;
    }

    // Transfer function texture
    if (g.transfer == 0 || g.transferColormap != m_colormap.get() || g.transferVersion != m_colormap->version() ||
        g.transferOpacity != m_opacityVersion)
    {
        const auto table = transferTable(*m_colormap, m_opacity, TransferSize);
        std::vector<std::uint8_t> bytes;
        for (const glm::vec4 &c : table)
            for (int k = 0; k < 4; ++k)
                bytes.push_back(static_cast<std::uint8_t>(std::clamp(c[k], 0.0f, 1.0f) * 255.0f + 0.5f));
        glActiveTexture(GL_TEXTURE0 + TransferUnit);
        if (g.transfer == 0)
        {
            glGenTextures(1, &g.transfer);
            glBindTexture(GL_TEXTURE_2D, g.transfer);
            ivf::glLabel(GL_TEXTURE, g.transfer, "Volume transfer function");
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glBindTexture(GL_TEXTURE_2D, g.transfer);
        GLint alignmentWas = 4;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignmentWas);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TransferSize, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes.data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignmentWas);
        const GLint filter = m_colormap->steps() > 0 ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        g.transferColormap = m_colormap.get();
        g.transferVersion = m_colormap->version();
        g.transferOpacity = m_opacityVersion;
    }

    GLint viewport[4] = {0, 0, 1, 1};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const bool sceneDepth = copySceneDepth(g, viewport[2], viewport[3]);

    // Uniforms
    const glm::mat4 &model = context.item.world;
    const glm::mat4 invModel = glm::inverse(model);
    const glm::mat4 invViewProjection = glm::inverse(context.projection * context.view);
    const glm::vec3 eyeWorld = glm::vec3(glm::inverse(context.view)[3]);
    const glm::vec3 eye = glm::vec3(invModel * glm::vec4(eyeWorld, 1.0f));
    const glm::ivec3 dims = m_grid->dimensions();
    glm::vec3 texScale(0.0f), texOffset(0.5f), texel(0.0f);
    for (int a = 0; a < 3; ++a)
    {
        const float n = static_cast<float>(dims[a]);
        texel[a] = 1.0f / n;
        if (dims[a] > 1 && size[a] > 0.0f)
        {
            texScale[a] = (n - 1.0f) / (n * size[a]);
            texOffset[a] = 0.5f / n - box.min()[a] * texScale[a];
        }
    }
    const float diagonal = glm::length(size);
    const float step = diagonal / static_cast<float>(m_samples);
    const ValueRange range = m_colormap->range();
    const bool log = m_colormap->scale() == ColormapScale::Log && range.min > 0.0f && range.max > range.min;

    auto previous = shaders->currentProgram();
    shaders->setCurrentProgram(program);
    program->uniformMatrix4("model", model);
    program->uniformMatrix4("invModel", invModel);
    program->uniformMatrix4("invViewProjection", invViewProjection);
    program->uniformVec3("eye", eye);
    program->uniformVec3("boxMin", box.min());
    program->uniformVec3("boxMax", box.max());
    program->uniformVec3("texScale", texScale);
    program->uniformVec3("texOffset", texOffset);
    program->uniformVec3("texelSize", texel);
    glUniform2f(program->uniformLoc("range"), range.min, range.max);
    program->uniformBool("logScale", log);
    program->uniformFloat("extinction", m_density * 12.0f / diagonal);
    program->uniformFloat("stepSize", step);
    program->uniformInt("maxSteps", 2 * m_samples);
    program->uniformBool("shading", m_shading);
    program->uniformBool("useSceneDepth", sceneDepth);
    program->uniformVec4f("viewport", static_cast<float>(viewport[0]), static_cast<float>(viewport[1]),
                          static_cast<float>(viewport[2]), static_cast<float>(viewport[3]));
    program->uniformInt("volume", VolumeUnit);
    program->uniformInt("transfer", TransferUnit);
    program->uniformInt("sceneDepth", DepthUnit);

    m_texture->bind(context.context, VolumeUnit);
    glActiveTexture(GL_TEXTURE0 + TransferUnit);
    glBindTexture(GL_TEXTURE_2D, g.transfer);
    glActiveTexture(GL_TEXTURE0 + DepthUnit);
    glBindTexture(GL_TEXTURE_2D, sceneDepth ? g.depthTexture : 0);

    // Back faces of the box (also when the camera is inside), no depth test: rays stop at
    // the copied scene depth instead
    const GLboolean depthTestWas = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean cullWas = glIsEnabled(GL_CULL_FACE);
    GLint cullModeWas = GL_BACK, frontFaceWas = GL_CCW;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullModeWas);
    glGetIntegerv(GL_FRONT_FACE, &frontFaceWas);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glBindVertexArray(g.vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    context.context.countDrawCall();
    glBindVertexArray(0);

    glCullFace(static_cast<GLenum>(cullModeWas));
    glFrontFace(static_cast<GLenum>(frontFaceWas));
    if (!cullWas)
        glDisable(GL_CULL_FACE);
    if (depthTestWas)
        glEnable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0 + DepthUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + TransferUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    ivf::Texture3D::unbind(VolumeUnit);

    if (previous)
    {
        shaders->setCurrentProgram(previous);
        context.context.transforms()->refreshForProgram();
        context.context.lights()->refreshForProgram();
        context.context.selection()->refreshForProgram();
    }
}
