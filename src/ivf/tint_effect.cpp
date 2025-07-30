#include <ivf/tint_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::TintEffect::TintEffect()
    : m_tintColor(glm::vec3(1.2, 0.9, 0.7)), m_tintStrength(0.5), m_grayScaleWeights(glm::vec3(0.299, 0.587, 0.114))
{
    setName("Tint");
}

std::shared_ptr<TintEffect> ivf::TintEffect::create()
{
    return std::make_shared<TintEffect>();
}

void ivf::TintEffect::setTintColor(glm::vec3 tintColor)
{
    m_tintColor = tintColor;
}

glm::vec3 ivf::TintEffect::tintColor() const
{
    return m_tintColor;
}

void ivf::TintEffect::setTintStrength(float tintStrength)
{
    m_tintStrength = tintStrength;
}

float ivf::TintEffect::tintStrength() const
{
    return m_tintStrength;
}

void ivf::TintEffect::setGrayScaleWeights(glm::vec3 grayScaleWeights)
{
    m_grayScaleWeights = grayScaleWeights;
}

glm::vec3 ivf::TintEffect::grayScaleWeights() const
{
    return m_grayScaleWeights;
}

void ivf::TintEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::tint_frag_shader_source,
                                         "tint", false);
}

void ivf::TintEffect::doUpdateParams()
{
    m_program->uniformVec3("tintColor", m_tintColor);
    m_program->uniformFloat("tintStrength", m_tintStrength);
    m_program->uniformVec3("grayScaleWeights", m_grayScaleWeights);
}

void ivf::TintEffect::setupProperties()
{
    addProperty("Tint color", &m_tintColor, "Tint");
    addProperty("Tint strength", &m_tintStrength, "Tint");
    addProperty("Gray scale weights", &m_grayScaleWeights, "Tint");
}
