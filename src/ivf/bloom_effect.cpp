#include <ivf/bloom_effect.h>

#include <ivf/post_shaders.h>

using namespace ivf;

ivf::BloomEffect::BloomEffect() : Effect(), m_threshold(1.0), m_intensity(1.0)
{}

std::shared_ptr<BloomEffect> ivf::BloomEffect::create()
{
    return std::make_shared<BloomEffect>();
}

void ivf::BloomEffect::setThreshold(float threshold)
{
    m_threshold = threshold;
}

float ivf::BloomEffect::threshold() const
{
    return m_threshold;
}

void ivf::BloomEffect::setIntensity(float intensity)
{
    m_intensity = intensity;
}

float ivf::BloomEffect::intensity() const
{
    return m_intensity;
}

void ivf::BloomEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, ivf::bloom_frag_shader_source,
                                         "bloom", false);
}

void ivf::BloomEffect::doUpdateParams()
{
    m_program->uniformFloat("threshold", m_threshold);
    m_program->uniformFloat("intensity", m_intensity);
}
