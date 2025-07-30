#include <ivf/filmgrain_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::FilmgrainEffect::FilmgrainEffect() : m_noiseIntensity(0.1), m_grainBlending(0.1)
{
    setName("Grain");
}

std::shared_ptr<FilmgrainEffect> ivf::FilmgrainEffect::create()
{
    return std::make_shared<FilmgrainEffect>();
}

void ivf::FilmgrainEffect::setNoiseIntensity(float noiseIntensity)
{
    m_noiseIntensity = noiseIntensity;
}

float ivf::FilmgrainEffect::noiseIntensity() const
{
    return m_noiseIntensity;
}

void ivf::FilmgrainEffect::setGrainBlending(float grainBlending)
{
    m_grainBlending = grainBlending;
}

float ivf::FilmgrainEffect::grainBlending() const
{
    return m_noiseIntensity;
}

void ivf::FilmgrainEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::filmgrain_frag_shader_source, "filmgrain", false);
}

void ivf::FilmgrainEffect::doUpdateParams()
{
    m_program->uniformFloat("noiseIntensity", m_noiseIntensity);
    m_program->uniformFloat("grainBlending", m_grainBlending);
}

void ivf::FilmgrainEffect::setupProperties()
{
    addProperty("noiseIntensity", &m_noiseIntensity, "Film Grain");
    addProperty("grainBlending", &m_grainBlending, "Film Grain");
}
