#include <ivf/chromatic_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::ChromaticEffect::ChromaticEffect()
{}

ivf::ChromaticEffect::~ChromaticEffect()
{}

std::shared_ptr<ChromaticEffect> ivf::ChromaticEffect::create()
{
    return std::make_shared<ChromaticEffect>();
}

void ivf::ChromaticEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::chromatic_frag_shader_source, "chromatic", false);
}

void ivf::ChromaticEffect::doUpdateParams()
{
    // m_program->uniformFloat("blurRadius", m_blurRadius);
}
