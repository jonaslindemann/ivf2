#include <ivf/gradient_sky_material.h>

#include <ivf/logger.h>
#include <ivf/shader_manager.h>

#include <string>

using namespace ivf;

namespace {

const std::string kProgramName = "ivf_gradient_sky";

// The dome is centered on the camera, so it never comes closer
const std::string sky_vert_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

out vec3 direction;

void main()
{
    direction = aPos;
    gl_Position = projection * view * vec4(viewPos + aPos, 1.0);
}
)";

const std::string sky_frag_shader_source = R"(
#version 330 core
in vec3 direction;

uniform float time;
uniform vec3 zenithColor;
uniform vec3 horizonColor;
uniform vec3 groundColor;
uniform float gradientExponent;
uniform vec3 glowColor;
uniform float glowSharpness;
uniform float starIntensity;

out vec4 fragColor;

float hash(vec3 p)
{
    p = fract(p * vec3(0.1031, 0.1030, 0.0973));
    p += dot(p, p.yxz + 33.33);
    return fract((p.x + p.y) * p.z);
}

// One candidate star per cell of a grid on the dome
vec3 stars(vec3 dir)
{
    const float cells = 160.0;
    vec3 cell = floor(dir * cells);
    float pick = hash(cell);
    if (pick < 0.985)
        return vec3(0.0);
    vec3 jitter = 0.35 * (vec3(hash(cell + 1.7), hash(cell + 4.1), hash(cell + 8.3)) - 0.5);
    float d = length(dir - normalize((cell + 0.5 + jitter) / cells)) * cells;
    float twinkle = 0.65 + 0.35 * sin(time * (1.0 + 3.0 * hash(cell + 2.3)) + pick * 60.0);
    float brightness = (pick - 0.985) / 0.015;
    return vec3(0.75, 0.85, 1.0) * exp(-d * d * 18.0) * brightness * twinkle;
}

void main()
{
    vec3 dir = normalize(direction);
    float h = dir.y;

    vec3 color = h >= 0.0 ? mix(horizonColor, zenithColor, pow(h, gradientExponent))
                          : mix(horizonColor, groundColor, smoothstep(0.0, 0.1, -h));
    color += glowColor * exp(-abs(h) * glowSharpness);

    if (starIntensity > 0.0)
        color += stars(dir) * starIntensity * smoothstep(0.03, 0.25, h);

    fragColor = vec4(color, 1.0);
}
)";

} // namespace

GradientSkyMaterial::GradientSkyMaterial(glm::vec3 zenithColor, glm::vec3 horizonColor)
    : BlendShaderMaterial(nullptr, BlendMode::Background), m_zenithColor(zenithColor), m_horizonColor(horizonColor),
      m_groundColor(horizonColor)
{
    setZenithColor(zenithColor);
    setHorizonColor(horizonColor);
    setGroundColor(horizonColor);
    setGradientExponent(m_gradientExponent);
    setGlowColor(m_glowColor);
    setGlowSharpness(m_glowSharpness);
    setStarIntensity(m_starIntensity);
}

std::shared_ptr<GradientSkyMaterial> GradientSkyMaterial::create(glm::vec3 zenithColor, glm::vec3 horizonColor)
{
    auto shaderMgr = ShaderManager::instance();

    // All sky materials share one program
    auto program = shaderMgr->program(kProgramName);
    if (!program)
    {
        program =
            shaderMgr->loadProgramFromStrings(sky_vert_shader_source, sky_frag_shader_source, kProgramName, false);
        if (shaderMgr->compileLinkErrors())
        {
            logError("Sky shader failed to compile or link.", "GradientSkyMaterial");
            return nullptr;
        }
    }

    auto material = std::make_shared<GradientSkyMaterial>(zenithColor, horizonColor);
    material->setProgram(program);
    return material;
}

void GradientSkyMaterial::setZenithColor(glm::vec3 color)
{
    m_zenithColor = color;
    setUniform("zenithColor", color);
}

glm::vec3 GradientSkyMaterial::zenithColor() const
{
    return m_zenithColor;
}

void GradientSkyMaterial::setHorizonColor(glm::vec3 color)
{
    m_horizonColor = color;
    setUniform("horizonColor", color);
}

glm::vec3 GradientSkyMaterial::horizonColor() const
{
    return m_horizonColor;
}

void GradientSkyMaterial::setGroundColor(glm::vec3 color)
{
    m_groundColor = color;
    setUniform("groundColor", color);
}

glm::vec3 GradientSkyMaterial::groundColor() const
{
    return m_groundColor;
}

void GradientSkyMaterial::setGradientExponent(float exponent)
{
    m_gradientExponent = exponent;
    setUniform("gradientExponent", exponent);
}

float GradientSkyMaterial::gradientExponent() const
{
    return m_gradientExponent;
}

void GradientSkyMaterial::setGlowColor(glm::vec3 color)
{
    m_glowColor = color;
    setUniform("glowColor", color);
}

glm::vec3 GradientSkyMaterial::glowColor() const
{
    return m_glowColor;
}

void GradientSkyMaterial::setGlowSharpness(float sharpness)
{
    m_glowSharpness = sharpness;
    setUniform("glowSharpness", sharpness);
}

float GradientSkyMaterial::glowSharpness() const
{
    return m_glowSharpness;
}

void GradientSkyMaterial::setStarIntensity(float intensity)
{
    m_starIntensity = intensity;
    setUniform("starIntensity", intensity);
}

float GradientSkyMaterial::starIntensity() const
{
    return m_starIntensity;
}
