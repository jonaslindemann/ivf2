#include <ivf/glitch_effect.h>
#include <ivf/post_shaders.h>

using namespace ivf;

GlitchEffect::GlitchEffect()
{
    setName("Glitch");
}

std::shared_ptr<GlitchEffect> GlitchEffect::create()
{
    return std::make_shared<GlitchEffect>();
}

void GlitchEffect::setIntensity(float intensity) { m_intensity = intensity; }
void GlitchEffect::setBlockSize(float blockSize) { m_blockSize = blockSize; }
void GlitchEffect::setSpeed(float speed) { m_speed = speed; }

float GlitchEffect::intensity() const { return m_intensity; }
float GlitchEffect::blockSize() const { return m_blockSize; }
float GlitchEffect::speed() const { return m_speed; }

void GlitchEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330,
                                         ivf::glitch_frag_shader_source, "glitch", false);
}

void GlitchEffect::doUpdateParams()
{
    m_program->uniformFloat("intensity", m_intensity);
    m_program->uniformFloat("blockSize", m_blockSize);
    m_program->uniformFloat("speed", m_speed);
}

void GlitchEffect::setupProperties()
{
    addProperty("intensity", &m_intensity, 0.0f, 0.5f, "Glitch");
    addProperty("blockSize", &m_blockSize, 0.01f, 0.5f, "Glitch");
    addProperty("speed", &m_speed, 0.0f, 20.0f, "Glitch");
}
