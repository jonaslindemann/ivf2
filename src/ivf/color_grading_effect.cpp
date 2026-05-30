#include <ivf/color_grading_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

ColorGradingEffect::ColorGradingEffect()
{
    setName("ColorGrading");
}

std::shared_ptr<ColorGradingEffect> ColorGradingEffect::create()
{
    return std::make_shared<ColorGradingEffect>();
}

void ColorGradingEffect::setShadows(glm::vec3 shadows) { m_shadows = shadows; }
void ColorGradingEffect::setMidtones(glm::vec3 midtones) { m_midtones = midtones; }
void ColorGradingEffect::setHighlights(glm::vec3 highlights) { m_highlights = highlights; }
void ColorGradingEffect::setContrast(float contrast) { m_contrast = contrast; }
void ColorGradingEffect::setSaturation(float saturation) { m_saturation = saturation; }

glm::vec3 ColorGradingEffect::shadows() const { return m_shadows; }
glm::vec3 ColorGradingEffect::midtones() const { return m_midtones; }
glm::vec3 ColorGradingEffect::highlights() const { return m_highlights; }
float ColorGradingEffect::contrast() const { return m_contrast; }
float ColorGradingEffect::saturation() const { return m_saturation; }

void ColorGradingEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::color_grading_frag_shader_source, "color_grading", false);
}

void ColorGradingEffect::doUpdateParams()
{
    m_program->uniformVec3("shadows", m_shadows);
    m_program->uniformVec3("midtones", m_midtones);
    m_program->uniformVec3("highlights", m_highlights);
    m_program->uniformFloat("contrast", m_contrast);
    m_program->uniformFloat("saturation", m_saturation);
}

void ColorGradingEffect::setupProperties()
{
    addProperty("shadows", &m_shadows, "Color Grading");
    addProperty("midtones", &m_midtones, "Color Grading");
    addProperty("highlights", &m_highlights, "Color Grading");
    addProperty("contrast", &m_contrast, 0.0f, 3.0f, "Color Grading");
    addProperty("saturation", &m_saturation, 0.0f, 3.0f, "Color Grading");
}
