#include <ivf/posterize_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

PosterizeEffect::PosterizeEffect()
{
    setName("Posterize");
}

std::shared_ptr<PosterizeEffect> PosterizeEffect::create()
{
    return std::make_shared<PosterizeEffect>();
}

void PosterizeEffect::setLevels(float levels) { m_levels = levels; }
float PosterizeEffect::levels() const { return m_levels; }

void PosterizeEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::posterize_frag_shader_source, "posterize", false);
}

void PosterizeEffect::doUpdateParams()
{
    m_program->uniformFloat("levels", m_levels);
}

void PosterizeEffect::setupProperties()
{
    addProperty("levels", &m_levels, 2.0f, 32.0f, "Posterize");
}
