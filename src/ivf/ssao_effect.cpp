#include <ivf/ssao_effect.h>

#include <ivf/post_shaders.h>
#include <ivf/screen_space_shaders.h>

#include <algorithm>

using namespace ivf;

SsaoEffect::SsaoEffect()
{
    setName("SSAO");
}

std::shared_ptr<SsaoEffect> SsaoEffect::create()
{
    return std::make_shared<SsaoEffect>();
}

void SsaoEffect::setRadius(float radius)
{
    m_radius = std::max(radius, 0.001f);
}

float SsaoEffect::radius() const
{
    return m_radius;
}

void SsaoEffect::setBias(float bias)
{
    m_bias = std::max(bias, 0.0f);
}

float SsaoEffect::bias() const
{
    return m_bias;
}

void SsaoEffect::setIntensity(float intensity)
{
    m_intensity = std::clamp(intensity, 0.0f, 1.0f);
}

float SsaoEffect::intensity() const
{
    return m_intensity;
}

void SsaoEffect::setPower(float power)
{
    m_power = std::clamp(power, 0.1f, 8.0f);
}

float SsaoEffect::power() const
{
    return m_power;
}

void SsaoEffect::setSamples(int samples)
{
    m_samples = std::clamp(samples, 1, 64);
}

int SsaoEffect::samples() const
{
    return m_samples;
}

void SsaoEffect::setBlurRadius(int radius)
{
    m_blurRadius = std::clamp(radius, 0, 4);
}

int SsaoEffect::blurRadius() const
{
    return m_blurRadius;
}

void SsaoEffect::setShowOcclusion(bool flag)
{
    m_showOcclusion = flag;
}

bool SsaoEffect::showOcclusion() const
{
    return m_showOcclusion;
}

void SsaoEffect::doLoad()
{
    m_prePassProgram =
        compileProgram(ivf::render_to_texture_vert_shader_source_330, ivf::ssao_frag_shader_source, "ssao");
    m_program = compileProgram(ivf::render_to_texture_vert_shader_source_330, ivf::ssao_apply_frag_shader_source,
                               "ssao_apply");
}

void SsaoEffect::doUpdateParams()
{
    m_program->uniformFloat("ssaoIntensity", m_intensity);
    m_program->uniformInt("ssaoBlurRadius", m_blurRadius);
    m_program->uniformBool("ssaoShow", m_showOcclusion);

    // The setters act on the bound program: the pre-pass gets its parameters while bound
    if (m_prePassProgram)
    {
        m_prePassProgram->use();
        m_prePassProgram->uniformFloat("ssaoRadius", m_radius);
        m_prePassProgram->uniformFloat("ssaoBias", m_bias);
        m_prePassProgram->uniformFloat("ssaoPower", m_power);
        m_prePassProgram->uniformInt("ssaoSamples", m_samples);
        m_program->use();
    }
}

void SsaoEffect::setupProperties()
{
    addProperty("Radius", &m_radius, 0.01, 5.0, "SSAO");
    addProperty("Bias", &m_bias, 0.0, 0.5, "SSAO");
    addProperty("Intensity", &m_intensity, 0.0, 1.0, "SSAO");
    addProperty("Power", &m_power, 0.1, 8.0, "SSAO");
    addProperty("Samples", &m_samples, 1, 64, "SSAO");
    addProperty("Blur radius", &m_blurRadius, 0, 4, "SSAO");
    addProperty("Show occlusion", &m_showOcclusion, "SSAO");
}
