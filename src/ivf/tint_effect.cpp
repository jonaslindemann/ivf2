#include <ivf/tint_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::TintEffect::TintEffect()
{}

std::shared_ptr<TintEffect> ivf::TintEffect::create()
{
    return std::make_shared<TintEffect>();
}

void ivf::TintEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::tint_frag_shader_source,
                                         "tint", false);
}

void ivf::TintEffect::doUpdateParams()
{
    // m_program->uniformFloat("blurRadius", m_blurRadius);
}
