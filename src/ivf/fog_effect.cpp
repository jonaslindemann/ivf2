#include <ivf/fog_effect.h>

#include <ivf/post_shaders.h>

#include <algorithm>

using namespace ivf;

FogEffect::FogEffect()
{
    setName("Fog");
}

std::shared_ptr<FogEffect> FogEffect::create()
{
    return std::make_shared<FogEffect>();
}

void FogEffect::setColor(glm::vec3 color)
{
    m_color = color;
}

glm::vec3 FogEffect::color() const
{
    return m_color;
}

void FogEffect::setDensity(float density)
{
    m_density = std::max(density, 0.0f);
}

float FogEffect::density() const
{
    return m_density;
}

void FogEffect::setStart(float start)
{
    m_start = std::max(start, 0.0f);
}

float FogEffect::start() const
{
    return m_start;
}

void FogEffect::setHeightFalloff(float falloff)
{
    m_heightFalloff = std::max(falloff, 0.0f);
}

float FogEffect::heightFalloff() const
{
    return m_heightFalloff;
}

void FogEffect::setBaseHeight(float height)
{
    m_baseHeight = height;
}

float FogEffect::baseHeight() const
{
    return m_baseHeight;
}

void FogEffect::setMaxOpacity(float opacity)
{
    m_maxOpacity = std::clamp(opacity, 0.0f, 1.0f);
}

float FogEffect::maxOpacity() const
{
    return m_maxOpacity;
}

void FogEffect::setSkyAmount(float amount)
{
    m_skyAmount = std::clamp(amount, 0.0f, 1.0f);
}

float FogEffect::skyAmount() const
{
    return m_skyAmount;
}

void FogEffect::setGroundBelowHorizon(bool enabled)
{
    m_groundBelowHorizon = enabled;
}

bool FogEffect::groundBelowHorizon() const
{
    return m_groundBelowHorizon;
}

void FogEffect::setHorizonBlend(float blend)
{
    m_horizonBlend = std::clamp(blend, 0.0f, 1.0f);
}

float FogEffect::horizonBlend() const
{
    return m_horizonBlend;
}

void FogEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::fog_frag_shader_source,
                                         "fog", false);
}

void FogEffect::doUpdateParams()
{
    m_program->uniformVec3("fogColor", m_color);
    m_program->uniformFloat("fogDensity", m_density);
    m_program->uniformFloat("fogStart", m_start);
    m_program->uniformFloat("fogHeightFalloff", m_heightFalloff);
    m_program->uniformFloat("fogBaseHeight", m_baseHeight);
    m_program->uniformFloat("fogMaxOpacity", m_maxOpacity);
    m_program->uniformFloat("fogSkyAmount", m_skyAmount);
    m_program->uniformBool("fogGroundBelowHorizon", m_groundBelowHorizon);
    m_program->uniformFloat("fogHorizonBlend", m_horizonBlend);
}

void FogEffect::setupProperties()
{
    addProperty("Color", &m_color, "Fog");
    addProperty("Density", &m_density, 0.0, 1.0, "Fog");
    addProperty("Start", &m_start, 0.0, 100.0, "Fog");
    addProperty("Height falloff", &m_heightFalloff, 0.0, 5.0, "Fog");
    addProperty("Base height", &m_baseHeight, -20.0, 20.0, "Fog");
    addProperty("Max opacity", &m_maxOpacity, 0.0, 1.0, "Fog");
    addProperty("Sky amount", &m_skyAmount, 0.0, 1.0, "Fog");
    addProperty("Ground below horizon", &m_groundBelowHorizon, "Fog");
    addProperty("Horizon blend", &m_horizonBlend, 0.0, 0.5, "Fog");
}
