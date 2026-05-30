#include <ivf/scanline_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

ScanlineEffect::ScanlineEffect()
{
    setName("Scanline");
}

std::shared_ptr<ScanlineEffect> ScanlineEffect::create()
{
    return std::make_shared<ScanlineEffect>();
}

void ScanlineEffect::setLineSpacing(float lineSpacing) { m_lineSpacing = lineSpacing; }
void ScanlineEffect::setLineIntensity(float lineIntensity) { m_lineIntensity = lineIntensity; }
void ScanlineEffect::setScrollSpeed(float scrollSpeed) { m_scrollSpeed = scrollSpeed; }

float ScanlineEffect::lineSpacing() const { return m_lineSpacing; }
float ScanlineEffect::lineIntensity() const { return m_lineIntensity; }
float ScanlineEffect::scrollSpeed() const { return m_scrollSpeed; }

void ScanlineEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::scanline_frag_shader_source, "scanline", false);
}

void ScanlineEffect::doUpdateParams()
{
    m_program->uniformFloat("lineSpacing", m_lineSpacing);
    m_program->uniformFloat("lineIntensity", m_lineIntensity);
    m_program->uniformFloat("scrollSpeed", m_scrollSpeed);
}

void ScanlineEffect::setupProperties()
{
    addProperty("lineSpacing", &m_lineSpacing, 1.0f, 20.0f, "Scanline");
    addProperty("lineIntensity", &m_lineIntensity, 0.0f, 1.0f, "Scanline");
    addProperty("scrollSpeed", &m_scrollSpeed, 0.0f, 10.0f, "Scanline");
}
