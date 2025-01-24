#include <ivf/filmgrain_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::FilmgrainEffect::FilmgrainEffect()
{}

std::shared_ptr<FilmgrainEffect> ivf::FilmgrainEffect::create()
{
    return std::make_shared<FilmgrainEffect>();
}

void ivf::FilmgrainEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::filmgrain_frag_shader_source, "filmgrain", false);
}

void ivf::FilmgrainEffect::doUpdateParams()
{
    // m_program->uniformFloat("blurRadius", m_blurRadius);
}
