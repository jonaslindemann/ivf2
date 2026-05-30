#include <ivf/wave_distortion_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

WaveDistortionEffect::WaveDistortionEffect()
{
    setName("WaveDistortion");
}

std::shared_ptr<WaveDistortionEffect> WaveDistortionEffect::create()
{
    return std::make_shared<WaveDistortionEffect>();
}

void WaveDistortionEffect::setFrequency(float frequency) { m_frequency = frequency; }
void WaveDistortionEffect::setAmplitude(float amplitude) { m_amplitude = amplitude; }
void WaveDistortionEffect::setSpeed(float speed) { m_speed = speed; }

float WaveDistortionEffect::frequency() const { return m_frequency; }
float WaveDistortionEffect::amplitude() const { return m_amplitude; }
float WaveDistortionEffect::speed() const { return m_speed; }

void WaveDistortionEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::wave_distortion_frag_shader_source, "wave_distortion", false);
}

void WaveDistortionEffect::doUpdateParams()
{
    m_program->uniformFloat("frequency", m_frequency);
    m_program->uniformFloat("amplitude", m_amplitude);
    m_program->uniformFloat("speed", m_speed);
}

void WaveDistortionEffect::setupProperties()
{
    addProperty("frequency", &m_frequency, 0.1f, 50.0f, "Wave Distortion");
    addProperty("amplitude", &m_amplitude, 0.0f, 0.1f, "Wave Distortion");
    addProperty("speed", &m_speed, 0.0f, 10.0f, "Wave Distortion");
}
