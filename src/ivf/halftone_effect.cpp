#include <ivf/halftone_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::HalftoneEffect::HalftoneEffect()
{
    this->setName("Halftone");
}

std::shared_ptr<HalftoneEffect> ivf::HalftoneEffect::create()
{
    return std::make_shared<HalftoneEffect>();
}

void ivf::HalftoneEffect::setDotSize(float dotSize)
{
    m_dotSize = dotSize;
}

float ivf::HalftoneEffect::dotSize() const
{
    return m_dotSize;
}

void ivf::HalftoneEffect::setAngle(float radians)
{
    m_angle = radians;
}

float ivf::HalftoneEffect::angle() const
{
    return m_angle;
}

void ivf::HalftoneEffect::setGrayscale(bool grayscale)
{
    m_grayscale = grayscale;
}

bool ivf::HalftoneEffect::grayscale() const
{
    return m_grayscale;
}

void ivf::HalftoneEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::halftone_frag_shader_source, "halftone", false);
}

void ivf::HalftoneEffect::doUpdateParams()
{
    m_program->uniformFloat("dotSize", m_dotSize);
    m_program->uniformFloat("angle", m_angle);
    m_program->uniformBool("grayscale", m_grayscale);
}

void ivf::HalftoneEffect::setupProperties()
{
    addProperty("dotSize", &m_dotSize, 2.0f, 32.0f, "Halftone");
    addProperty("angle", &m_angle, 0.0f, 1.5708f, "Halftone");
    addProperty("grayscale", &m_grayscale, "Halftone");
}
