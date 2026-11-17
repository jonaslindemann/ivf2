#include <ivf/mist_material.h>

#include <ivf/logger.h>
#include <ivf/shader_manager.h>

#include <algorithm>
#include <string>

using namespace ivf;

namespace {

const std::string kProgramName = "ivf_mist";

const std::string mist_vert_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 localPos;

void main()
{
    localPos = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Densest at the node's origin, fading out towards mistRadius
const std::string mist_frag_shader_source = R"(
#version 330 core
in vec3 localPos;

uniform vec3 mistColor;
uniform float mistOpacity;
uniform float mistRadius;
uniform float mistSoftness;

out vec4 fragColor;

void main()
{
    float r = length(localPos) / max(mistRadius, 1e-4);
    float alpha = mistOpacity * (1.0 - smoothstep(1.0 - mistSoftness, 1.0, r));
    fragColor = vec4(mistColor, alpha);
}
)";

} // namespace

MistMaterial::MistMaterial(glm::vec3 color, float opacity, float radius)
    : BlendShaderMaterial(nullptr, BlendMode::Alpha), m_color(color), m_opacity(opacity), m_radius(radius)
{
    setColor(color);
    setOpacity(opacity);
    setRadius(radius);
    setSoftness(m_softness);
}

std::shared_ptr<MistMaterial> MistMaterial::create(glm::vec3 color, float opacity, float radius)
{
    auto shaderMgr = ShaderManager::instance();

    // All mist materials share one program
    auto program = shaderMgr->program(kProgramName);
    if (!program)
    {
        program =
            shaderMgr->loadProgramFromStrings(mist_vert_shader_source, mist_frag_shader_source, kProgramName, false);
        if (shaderMgr->compileLinkErrors())
        {
            logError("Mist shader failed to compile or link.", "MistMaterial");
            return nullptr;
        }
    }

    auto material = std::make_shared<MistMaterial>(color, opacity, radius);
    material->setProgram(program);
    return material;
}

void MistMaterial::setColor(glm::vec3 color)
{
    m_color = color;
    setUniform("mistColor", color);
}

glm::vec3 MistMaterial::color() const
{
    return m_color;
}

void MistMaterial::setOpacity(float opacity)
{
    m_opacity = opacity;
    setUniform("mistOpacity", opacity);
}

float MistMaterial::opacity() const
{
    return m_opacity;
}

void MistMaterial::setRadius(float radius)
{
    m_radius = radius;
    setUniform("mistRadius", radius);
}

float MistMaterial::radius() const
{
    return m_radius;
}

void MistMaterial::setSoftness(float softness)
{
    m_softness = std::clamp(softness, 0.0f, 1.0f);
    setUniform("mistSoftness", m_softness);
}

float MistMaterial::softness() const
{
    return m_softness;
}
