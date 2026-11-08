#include <ivf/water_material.h>

#include <ivf/logger.h>
#include <ivf/shader_manager.h>

#include <string>

using namespace ivf;

namespace {

const std::string kProgramName = "ivf_water";

const std::string water_vert_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;

void main()
{
    vec4 p = model * vec4(aPos, 1.0);
    worldPos = p.xyz;
    gl_Position = projection * view * p;
}
)";

const std::string water_frag_shader_source = R"(
#version 330 core
in vec3 worldPos;

uniform sampler2D reflectionTexture;
uniform bool hasReflection;
uniform vec2 viewportSize;
uniform vec3 viewPos;
uniform float time;

uniform vec3 waterColor;
uniform float reflectivity;
uniform float minReflectivity;
uniform float fresnelPower;
uniform float rippleStrength;
uniform float rippleScale;
uniform float rippleSpeed;
uniform float distortion;

out vec4 fragColor;

// Slope of a sum of directional waves; returns (dh/dx, dh/dz)
vec2 rippleSlope(vec2 p, float t)
{
    const vec2 dirs[4] = vec2[4](vec2(1.0, 0.3), vec2(-0.6, 1.0), vec2(0.2, -1.0), vec2(-1.0, -0.4));
    const float freqs[4] = float[4](1.0, 1.7, 2.9, 4.3);
    vec2 slope = vec2(0.0);
    for (int i = 0; i < 4; ++i) {
        vec2 d = normalize(dirs[i]);
        float k = freqs[i] * rippleScale * 6.2831853;
        float phase = dot(d, p) * k + t * rippleSpeed * freqs[i];
        slope += d * k * cos(phase) / freqs[i];
    }
    return slope * rippleStrength * 0.05;
}

void main()
{
    vec2 slope = rippleSlope(worldPos.xz, time);
    vec3 n = normalize(vec3(-slope.x, 1.0, -slope.y));
    vec3 v = normalize(viewPos - worldPos);

    float cosTheta = max(dot(n, v), 0.0);
    float fresnel = mix(minReflectivity, reflectivity, pow(1.0 - cosTheta, fresnelPower));

    vec3 reflection = waterColor;
    if (hasReflection) {
        vec2 uv = gl_FragCoord.xy / viewportSize + n.xz * distortion;
        reflection = texture(reflectionTexture, clamp(uv, vec2(0.001), vec2(0.999))).rgb;
    }

    fragColor = vec4(mix(waterColor, reflection, fresnel), 1.0);
}
)";

} // namespace

WaterMaterial::WaterMaterial(PlanarReflectionPtr reflection) : m_reflection(reflection)
{
    setWaterColor({0.02f, 0.05f, 0.08f});
    setReflectivity(1.0f);
    setMinReflectivity(0.3f);
    setFresnelPower(3.0f);
    setRippleStrength(1.0f);
    setRippleScale(0.5f);
    setRippleSpeed(1.0f);
    setDistortion(0.03f);
}

std::shared_ptr<WaterMaterial> WaterMaterial::create(PlanarReflectionPtr reflection)
{
    auto shaderMgr = ShaderManager::instance();

    // All water materials share one program
    auto program = shaderMgr->program(kProgramName);
    if (!program)
    {
        program = shaderMgr->loadProgramFromStrings(water_vert_shader_source, water_frag_shader_source, kProgramName,
                                                    false);
        if (shaderMgr->compileLinkErrors())
        {
            logError("Water shader failed to compile or link.", "WaterMaterial");
            return nullptr;
        }
    }

    auto material = std::make_shared<WaterMaterial>(reflection);
    material->setProgram(program);
    return material;
}

void WaterMaterial::setReflection(PlanarReflectionPtr reflection)
{
    m_reflection = reflection;
}

PlanarReflectionPtr WaterMaterial::reflection() const
{
    return m_reflection;
}

void WaterMaterial::setWaterColor(glm::vec3 color)
{
    setUniform("waterColor", color);
}

void WaterMaterial::setReflectivity(float reflectivity)
{
    setUniform("reflectivity", reflectivity);
}

void WaterMaterial::setMinReflectivity(float reflectivity)
{
    setUniform("minReflectivity", reflectivity);
}

void WaterMaterial::setFresnelPower(float power)
{
    setUniform("fresnelPower", power);
}

void WaterMaterial::setRippleStrength(float strength)
{
    setUniform("rippleStrength", strength);
}

void WaterMaterial::setRippleScale(float scale)
{
    setUniform("rippleScale", scale);
}

void WaterMaterial::setRippleSpeed(float speed)
{
    setUniform("rippleSpeed", speed);
}

void WaterMaterial::setDistortion(float distortion)
{
    setUniform("distortion", distortion);
}

void WaterMaterial::apply()
{
    // The reflection texture is recreated on resize, so bind it fresh every frame
    GLuint texture = m_reflection ? m_reflection->texture() : 0;
    if (texture != 0)
        setTexture("reflectionTexture", texture, 0);
    else
        removeTexture("reflectionTexture");
    setUniform("hasReflection", texture != 0);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    setUniform("viewportSize", glm::vec2(float(viewport[2]), float(viewport[3])));

    ShaderMaterial::apply();
}
