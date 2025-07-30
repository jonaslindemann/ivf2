#include <ivf/dithering_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::DitheringEffect::DitheringEffect()
{
    setName("Dithering");
}

std::shared_ptr<DitheringEffect> ivf::DitheringEffect::create()
{
    return std::make_shared<DitheringEffect>();
}

void ivf::DitheringEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::dither_frag_shader_source,
                                         "dithering", false);
}

void ivf::DitheringEffect::doUpdateParams()
{}
