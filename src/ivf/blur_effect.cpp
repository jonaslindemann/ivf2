#include <ivf/blur_effect.h>

#include <ivf/stock_shaders.h>

using namespace ivf;

ivf::BlurEffect::BlurEffect()
{}

ivf::BlurEffect::~BlurEffect()
{}

std::shared_ptr<BlurEffect> ivf::BlurEffect::create()
{
    return std::make_shared<BlurEffect>();
}

void ivf::BlurEffect::setBlurRadius(float radius)
{
    m_blurRadius = radius;
}

float ivf::BlurEffect::blurRadius()
{
    return m_blurRadius;
}

void ivf::BlurEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::blur_frag_shader_source,
                                         "blur", false);
}

void ivf::BlurEffect::doUpdateParams()
{
    m_program->uniformFloat("blurRadius", m_blurRadius);
}
