#include <ivf/fade_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::FadeEffect::FadeEffect()
{
    setName("Fade");
}

std::shared_ptr<FadeEffect> ivf::FadeEffect::create()
{
    return std::make_shared<FadeEffect>();
}

void ivf::FadeEffect::setFadeColor(glm::vec3 fadeColor)
{
    m_fadeColor = fadeColor;
}

glm::vec3 ivf::FadeEffect::fadeColor() const
{
    return m_fadeColor;
}

void ivf::FadeEffect::setFadeAmount(float fadeAmount)
{
    m_fadeAmount = fadeAmount;
}

float ivf::FadeEffect::fadeAmount() const
{
    return m_fadeAmount;
}

void ivf::FadeEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::fade_frag_shader_source,
                                         "fade", false);
}

void ivf::FadeEffect::doUpdateParams()
{
    m_program->uniformVec3("fadeColor", m_fadeColor);
    m_program->uniformFloat("fadeAmount", m_fadeAmount);
}

void ivf::FadeEffect::setupProperties()
{
    addProperty("Fade color", &m_fadeColor, "Fade");
    addProperty("Fade amount", &m_fadeAmount, "Fade");
}
