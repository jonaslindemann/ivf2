#include <ivf/pixelation_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::PixelationEffect::PixelationEffect() : Effect(), m_pixelSize(4.0f)
{}

ivf::PixelationEffect::~PixelationEffect()
{}

std::shared_ptr<PixelationEffect> ivf::PixelationEffect::create()
{
    return std::make_shared<PixelationEffect>();
}

void ivf::PixelationEffect::setPixelSize(float size)
{
    m_pixelSize = size;
}

float ivf::PixelationEffect::pixelSize()
{
    return m_pixelSize;
}

void ivf::PixelationEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::pixelate_frag_shader_source, "pixelation", false);
}

void ivf::PixelationEffect::doUpdateParams()
{
    m_program->uniformFloat("pixelSize", m_pixelSize);
}

void ivf::PixelationEffect::setupProperties()
{
    addProperty("pixelSize", &m_pixelSize, "Pixelation");
}
