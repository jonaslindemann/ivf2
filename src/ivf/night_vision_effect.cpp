#include <ivf/night_vision_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

NightVisionEffect::NightVisionEffect()
{
    setName("NightVision");
}

std::shared_ptr<NightVisionEffect> NightVisionEffect::create()
{
    return std::make_shared<NightVisionEffect>();
}

void NightVisionEffect::setNoiseIntensity(float noiseIntensity) { m_noiseIntensity = noiseIntensity; }
void NightVisionEffect::setGlowStrength(float glowStrength) { m_glowStrength = glowStrength; }
void NightVisionEffect::setPhosphorColor(glm::vec3 phosphorColor) { m_phosphorColor = phosphorColor; }

float NightVisionEffect::noiseIntensity() const { return m_noiseIntensity; }
float NightVisionEffect::glowStrength() const { return m_glowStrength; }
glm::vec3 NightVisionEffect::phosphorColor() const { return m_phosphorColor; }

void NightVisionEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::night_vision_frag_shader_source, "night_vision", false);
}

void NightVisionEffect::doUpdateParams()
{
    m_program->uniformFloat("noiseIntensity", m_noiseIntensity);
    m_program->uniformFloat("glowStrength", m_glowStrength);
    m_program->uniformVec3("phosphorColor", m_phosphorColor);
}

void NightVisionEffect::setupProperties()
{
    addProperty("noiseIntensity", &m_noiseIntensity, 0.0f, 0.5f, "Night Vision");
    addProperty("glowStrength", &m_glowStrength, 0.0f, 2.0f, "Night Vision");
    addProperty("phosphorColor", &m_phosphorColor, "Night Vision");
}
