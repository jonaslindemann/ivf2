#include <ivf/glow_material.h>

#include <ivf/logger.h>
#include <ivf/shader_manager.h>

#include <string>

using namespace ivf;

namespace {

const std::string kProgramName = "ivf_glow";

const std::string glow_vert_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;
out vec3 worldNormal;

void main()
{
    vec4 p = model * vec4(aPos, 1.0);
    worldPos = p.xyz;
    worldNormal = mat3(model) * aNormal;
    gl_Position = projection * view * p;
}
)";

// Bright where the surface faces the viewer, fading towards the silhouette
const std::string glow_frag_shader_source = R"(
#version 330 core
in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 viewPos;
uniform vec3 glowColor;
uniform float glowBrightness;
uniform float glowFalloff;

out vec4 fragColor;

void main()
{
    vec3 v = normalize(viewPos - worldPos);
    // Back faces add nothing, so a shape cut by other geometry keeps an even glow
    float facing = max(dot(normalize(worldNormal), v), 0.0);
    float core = pow(facing, glowFalloff);
    fragColor = vec4(glowColor * core * glowBrightness, core);
}
)";

} // namespace

GlowMaterial::GlowMaterial(glm::vec3 color, float brightness, BlendMode mode)
    : BlendShaderMaterial(nullptr, mode), m_color(color), m_brightness(brightness)
{
    setColor(color);
    setBrightness(brightness);
    setFalloff(m_falloff);
}

std::shared_ptr<GlowMaterial> GlowMaterial::create(glm::vec3 color, float brightness, BlendMode mode)
{
    auto shaderMgr = ShaderManager::instance();

    // All glow materials share one program
    auto program = shaderMgr->program(kProgramName);
    if (!program)
    {
        program =
            shaderMgr->loadProgramFromStrings(glow_vert_shader_source, glow_frag_shader_source, kProgramName, false);
        if (shaderMgr->compileLinkErrors())
        {
            logError("Glow shader failed to compile or link.", "GlowMaterial");
            return nullptr;
        }
    }

    auto material = std::make_shared<GlowMaterial>(color, brightness, mode);
    material->setProgram(program);
    return material;
}

void GlowMaterial::setColor(glm::vec3 color)
{
    m_color = color;
    setUniform("glowColor", color);
}

glm::vec3 GlowMaterial::color() const
{
    return m_color;
}

void GlowMaterial::setBrightness(float brightness)
{
    m_brightness = brightness;
    setUniform("glowBrightness", brightness);
}

float GlowMaterial::brightness() const
{
    return m_brightness;
}

void GlowMaterial::setFalloff(float falloff)
{
    m_falloff = falloff;
    setUniform("glowFalloff", falloff);
}

float GlowMaterial::falloff() const
{
    return m_falloff;
}
