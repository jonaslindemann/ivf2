#include <ivf/environment_map.h>

#include <ivf/gl_debug.h>
#include <ivf/image_io.h>
#include <ivf/logger.h>
#include <ivf/program.h>
#include <ivf/render_context.h>
#include <ivf/shader_manager.h>

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <vector>

using namespace ivf;

namespace {

constexpr int kIrradianceSize = 32;
constexpr int kPrefilterSize = 128;
constexpr int kPrefilterLevels = 5; // roughness 0, 0.25, 0.5, 0.75, 1
constexpr int kBrdfLutSize = 256;

// Full-screen triangle; uv covers [0, 1] over the viewport
const std::string kVertexSource = R"(
#version 330 core
out vec2 uv;
void main()
{
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    uv = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

// Direction of a texel of cube map face `face` (OpenGL's cube map convention, so that
// texture(cube, direction) returns what was rendered for that direction)
const std::string kFaceDirection = R"(
uniform int face;
const float PI = 3.14159265359;

vec3 faceDirection(vec2 uv)
{
    vec2 st = uv * 2.0 - 1.0;
    vec3 d;
    if (face == 0)      d = vec3( 1.0, -st.y, -st.x);
    else if (face == 1) d = vec3(-1.0, -st.y,  st.x);
    else if (face == 2) d = vec3( st.x,  1.0,  st.y);
    else if (face == 3) d = vec3( st.x, -1.0, -st.y);
    else if (face == 4) d = vec3( st.x, -st.y,  1.0);
    else                d = vec3(-st.x, -st.y, -1.0);
    return normalize(d);
}
)";

const std::string kEquirectSource = R"(
#version 330 core
in vec2 uv;
out vec4 fragColor;
uniform sampler2D equirect;
)" + kFaceDirection + R"(
void main()
{
    vec3 d = faceDirection(uv);
    vec2 e = vec2(atan(d.z, d.x) / (2.0 * PI) + 0.5, acos(clamp(d.y, -1.0, 1.0)) / PI);
    fragColor = vec4(textureLod(equirect, e, 0.0).rgb, 1.0);
}
)";

// Cosine-weighted hemisphere integral of the environment around each normal
const std::string kIrradianceSource = R"(
#version 330 core
in vec2 uv;
out vec4 fragColor;
uniform samplerCube environment;
uniform float sourceLod;
uniform float sampleDelta;
)" + kFaceDirection + R"(
void main()
{
    vec3 N = faceDirection(uv);
    vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = cross(N, right);

    vec3 sum = vec3(0.0);
    float count = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 t = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 s = t.x * right + t.y * up + t.z * N;
            sum += textureLod(environment, s, sourceLod).rgb * cos(theta) * sin(theta);
            count += 1.0;
        }
    }
    fragColor = vec4(PI * sum / count, 1.0);
}
)";

const std::string kSamplingSource = R"(
float radicalInverse(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint n)
{
    return vec2(float(i) / float(n), radicalInverse(i));
}

// GGX-distributed half vector around N
vec3 importanceSampleGGX(vec2 xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}
)";

// Environment convolved with the GGX lobe of one roughness (split sum, first part).
// Samples come from blurrier source mips where they are sparse, which removes the noise
// of bright spots like the sun.
const std::string kPrefilterSource = R"(
#version 330 core
in vec2 uv;
out vec4 fragColor;
uniform samplerCube environment;
uniform float roughness;
uniform float sourceResolution;
uniform int sampleCount;
)" + kFaceDirection + kSamplingSource + R"(
float distributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

void main()
{
    vec3 N = faceDirection(uv);
    if (roughness <= 0.0)
    {
        fragColor = vec4(textureLod(environment, N, 0.0).rgb, 1.0);
        return;
    }

    vec3 V = N;
    vec3 sum = vec3(0.0);
    float weight = 0.0;
    float texelSolidAngle = 4.0 * PI / (6.0 * sourceResolution * sourceResolution);
    uint n = uint(sampleCount);
    for (uint i = 0u; i < n; ++i)
    {
        vec3 H = importanceSampleGGX(hammersley(i, n), N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = dot(N, L);
        if (NdotL > 0.0)
        {
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = distributionGGX(NdotH, roughness) * NdotH / (4.0 * HdotV) + 0.0001;
            float sampleSolidAngle = 1.0 / (float(sampleCount) * pdf + 0.0001);
            float lod = 0.5 * log2(sampleSolidAngle / texelSolidAngle);
            sum += textureLod(environment, L, max(lod, 0.0)).rgb * NdotL;
            weight += NdotL;
        }
    }
    fragColor = vec4(sum / max(weight, 0.0001), 1.0);
}
)";

// Scale and bias for F0 over N.V and roughness (split sum, second part)
const std::string kBrdfSource = R"(
#version 330 core
in vec2 uv;
out vec4 fragColor;
const float PI = 3.14159265359;
)" + kSamplingSource + R"(
float geometrySchlickGGX(float NdotV, float roughness)
{
    float k = (roughness * roughness) / 2.0; // IBL remapping
    return NdotV / (NdotV * (1.0 - k) + k);
}

void main()
{
    float NdotV = max(uv.x, 1e-3);
    float roughness = uv.y;
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    vec3 N = vec3(0.0, 0.0, 1.0);

    float A = 0.0;
    float B = 0.0;
    const uint n = 256u;
    for (uint i = 0u; i < n; ++i)
    {
        vec3 H = importanceSampleGGX(hammersley(i, n), N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        if (NdotL > 0.0)
        {
            float G = geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
            float Gvis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * Gvis;
            B += Fc * Gvis;
        }
    }
    fragColor = vec4(A / float(n), B / float(n), 0.0, 1.0);
}
)";

// Programs are compiled once per render context and kept by its shader manager
ProgramPtr iblProgram(const std::string &name, const std::string &fragmentSource)
{
    auto *shaders = ShaderManager::instance();
    if (auto program = shaders->program(name))
        return program;
    return shaders->compileProgramFromStrings(kVertexSource, fragmentSource, name, false).value_or(nullptr);
}

// Saves and restores the GL state the precomputation changes
class StateGuard {
public:
    StateGuard()
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_framebuffer);
        glGetIntegerv(GL_VIEWPORT, m_viewport);
        glGetIntegerv(GL_CURRENT_PROGRAM, &m_program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_vao);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &m_activeTexture);
        m_depthTest = glIsEnabled(GL_DEPTH_TEST);
        m_cullFace = glIsEnabled(GL_CULL_FACE);
        m_blend = glIsEnabled(GL_BLEND);
        m_scissor = glIsEnabled(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
    }

    ~StateGuard()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(m_framebuffer));
        glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
        glUseProgram(static_cast<GLuint>(m_program));
        glBindVertexArray(static_cast<GLuint>(m_vao));
        glActiveTexture(static_cast<GLenum>(m_activeTexture));
        restore(GL_DEPTH_TEST, m_depthTest);
        restore(GL_CULL_FACE, m_cullFace);
        restore(GL_BLEND, m_blend);
        restore(GL_SCISSOR_TEST, m_scissor);
        if (auto *context = RenderContext::current())
            context->resetProgramTracking(static_cast<GLuint>(m_program));
    }

private:
    GLint m_framebuffer{0}, m_viewport[4]{}, m_program{0}, m_vao{0}, m_activeTexture{GL_TEXTURE0};
    GLboolean m_depthTest{GL_FALSE}, m_cullFace{GL_FALSE}, m_blend{GL_FALSE}, m_scissor{GL_FALSE};

    static void restore(GLenum cap, GLboolean on)
    {
        if (on)
            glEnable(cap);
        else
            glDisable(cap);
    }
};

int nextPowerOfTwo(int value)
{
    int p = 1;
    while (p < value)
        p <<= 1;
    return p;
}

} // namespace

EnvironmentMap::EnvironmentMap() = default;

EnvironmentMap::~EnvironmentMap()
{
    if (m_brdfLut != 0)
        glDeleteTextures(1, &m_brdfLut);
}

std::shared_ptr<EnvironmentMap> EnvironmentMap::create()
{
    return std::make_shared<EnvironmentMap>();
}

Result<void> EnvironmentMap::load(const std::string &path)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
        return makeError(ErrorCode::FileNotFound, "Could not load environment '" + path + "': file not found",
                         "EnvironmentMap");
    if (std::filesystem::path(path).extension() == ".exr")
        return makeError(ErrorCode::UnsupportedFormat,
                         "Could not load environment '" + path + "': .exr is not supported, use .hdr",
                         "EnvironmentMap");

    // Top row first, whatever the global flip setting is
    const bool flip = imageFlipVerticallyOnLoad();
    setImageFlipVerticallyOnLoad(false);
    int width = 0, height = 0, channels = 0;
    float *data = loadImageF(path.c_str(), &width, &height, &channels, 3);
    setImageFlipVerticallyOnLoad(flip);
    if (data == nullptr)
        return makeError(ErrorCode::UnsupportedFormat,
                         "Could not load environment '" + path + "': " + imageFailureReason(), "EnvironmentMap");

    logInfo(logStr("Loaded environment ", path, " (", width, " x ", height, ")"), "EnvironmentMap");
    auto result = loadEquirectangular(data, width, height);
    freeImage(data);
    return result;
}

Result<void> EnvironmentMap::loadEquirectangular(const float *rgb, int width, int height)
{
    if (rgb == nullptr || width < 2 || height < 1)
        return makeError(ErrorCode::InvalidArgument, "Environment image is empty", "EnvironmentMap");

    GLuint equirect = 0;
    glGenTextures(1, &equirect);
    glBindTexture(GL_TEXTURE_2D, equirect);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, rgb);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // A cube face covers a quarter of the image's width
    const int size = std::clamp(nextPowerOfTwo(width / 4), 64, std::max(m_cubemapSize, 64));
    auto result = precompute(equirect, size);
    glDeleteTextures(1, &equirect);
    return result;
}

std::vector<float> EnvironmentMap::skyImage(const SkySettings &settings)
{
    const int width = std::max(settings.width, 8);
    const int height = width / 2;
    std::vector<float> image(static_cast<size_t>(width) * height * 3);

    const glm::vec3 sun = glm::normalize(settings.sunDirection);
    const float sunRadius = glm::radians(std::max(settings.sunRadiusDeg, 0.01f));
    const float pi = glm::pi<float>();

    for (int y = 0; y < height; ++y)
    {
        const float theta = (float(y) + 0.5f) / float(height) * pi; // 0 at the top
        for (int x = 0; x < width; ++x)
        {
            // Inverse of the lookup in the equirect shader: u = atan(z, x) / 2pi + 0.5
            const float phi = ((float(x) + 0.5f) / float(width) - 0.5f) * 2.0f * pi;
            const glm::vec3 d(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));

            glm::vec3 color;
            if (d.y >= 0.0f)
                color = glm::mix(settings.horizonColor, settings.zenithColor, std::pow(d.y, 0.5f));
            else
                color = glm::mix(settings.horizonColor * 0.6f, settings.groundColor,
                                 glm::smoothstep(0.0f, 0.08f, -d.y));

            const float angle = std::acos(std::clamp(glm::dot(d, sun), -1.0f, 1.0f));
            // Disc with a soft edge (no stair steps when it is reprojected onto the cube)
            const float disc = 1.0f - glm::smoothstep(sunRadius * 0.8f, sunRadius, angle);
            color += settings.sunColor * settings.sunIntensity * disc;
            color += settings.sunColor * settings.sunGlow * std::exp(-angle * 6.0f);

            float *p = &image[(static_cast<size_t>(y) * width + x) * 3];
            p[0] = color.r;
            p[1] = color.g;
            p[2] = color.b;
        }
    }
    return image;
}

Result<void> EnvironmentMap::loadSky(const SkySettings &settings)
{
    const auto image = skyImage(settings);
    const int width = std::max(settings.width, 8);
    return loadEquirectangular(image.data(), width, width / 2);
}

void EnvironmentMap::bind() const
{
    if (!m_valid)
        return;
    m_irradiance->bind(IrradianceMapUnit);
    m_prefiltered->bind(PrefilterMapUnit);
    glActiveTexture(GL_TEXTURE0 + BrdfLutUnit);
    glBindTexture(GL_TEXTURE_2D, m_brdfLut);
    glActiveTexture(GL_TEXTURE0);
}

Result<void> EnvironmentMap::precompute(GLuint equirectTexture, int cubemapSize)
{
    auto equirectProgram = iblProgram("ivf_ibl_equirect", kEquirectSource);
    auto irradianceProgram = iblProgram("ivf_ibl_irradiance", kIrradianceSource);
    auto prefilterProgram = iblProgram("ivf_ibl_prefilter", kPrefilterSource);
    auto brdfProgram = iblProgram("ivf_ibl_brdf", kBrdfSource);
    if (!equirectProgram || !irradianceProgram || !prefilterProgram || !brdfProgram)
        return makeError(ErrorCode::CompileFailed, "Image-based lighting shaders failed to compile", "EnvironmentMap");

    StateGuard guard;

    // Filtering across cube faces (seams would show in the blurred mips)
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    GLuint vao = 0, fbo = 0;
    glGenVertexArrays(1, &vao);
    glGenFramebuffers(1, &fbo);
    glBindVertexArray(vao);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glLabel(GL_FRAMEBUFFER, fbo, "ivf environment precompute");

    auto drawFaces = [&](const ProgramPtr &program, const CubemapPtr &target, int level) {
        const int size = std::max(target->size() >> level, 1);
        glViewport(0, 0, size, size);
        for (int face = 0; face < 6; ++face)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                   target->id(), level);
            program->uniformInt("face", face);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    };

    // 1. Equirectangular image -> environment cube map with mip levels
    const int levels = 1 + int(std::floor(std::log2(double(cubemapSize))));
    m_environment = Cubemap::create();
    m_environment->allocate(cubemapSize, GL_RGBA16F, levels);
    glLabel(GL_TEXTURE, m_environment->id(), "ivf environment");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                           m_environment->id(), 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteVertexArrays(1, &vao);
        return makeError(ErrorCode::NotSupported, "Cannot render into floating-point cube maps on this driver",
                         "EnvironmentMap");
    }
    equirectProgram->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, equirectTexture);
    equirectProgram->uniformInt("equirect", 0);
    drawFaces(equirectProgram, m_environment, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_environment->id());
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // 2. Diffuse irradiance, from a mip level about as coarse as the sample spacing
    m_irradiance = Cubemap::create();
    m_irradiance->allocate(kIrradianceSize, GL_RGBA16F, 1);
    glLabel(GL_TEXTURE, m_irradiance->id(), "ivf environment irradiance");
    irradianceProgram->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_environment->id());
    irradianceProgram->uniformInt("environment", 0);
    irradianceProgram->uniformFloat("sampleDelta", 0.08f);
    irradianceProgram->uniformFloat("sourceLod", std::max(0.0f, float(std::log2(cubemapSize / 32.0))));
    drawFaces(irradianceProgram, m_irradiance, 0);

    // 3. Prefiltered specular, roughness 0..1 over the mip levels
    m_prefiltered = Cubemap::create();
    m_prefiltered->allocate(kPrefilterSize, GL_RGBA16F, kPrefilterLevels);
    glLabel(GL_TEXTURE, m_prefiltered->id(), "ivf environment prefiltered");
    prefilterProgram->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_environment->id()); // allocate() unbound it
    prefilterProgram->uniformInt("environment", 0);
    prefilterProgram->uniformFloat("sourceResolution", float(cubemapSize));
    prefilterProgram->uniformInt("sampleCount", 256);
    for (int level = 0; level < kPrefilterLevels; ++level)
    {
        prefilterProgram->uniformFloat("roughness", float(level) / float(kPrefilterLevels - 1));
        drawFaces(prefilterProgram, m_prefiltered, level);
    }

    // 4. BRDF lookup table (independent of the environment)
    if (m_brdfLut == 0)
    {
        glGenTextures(1, &m_brdfLut);
        glBindTexture(GL_TEXTURE_2D, m_brdfLut);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, kBrdfLutSize, kBrdfLutSize, 0, GL_RG, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glLabel(GL_TEXTURE, m_brdfLut, "ivf BRDF LUT");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLut, 0);
        glViewport(0, 0, kBrdfLutSize, kBrdfLutSize);
        brdfProgram->use();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);

    m_valid = true;
    return {};
}
