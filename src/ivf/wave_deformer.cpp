#include <ivf/wave_deformer.h>
#include <cmath>
#include <algorithm>

using namespace ivf;

WaveDeformer::WaveDeformer(float amplitude, float frequency, float speed, const glm::vec3 &direction,
                           const glm::vec3 &waveVector)
    : m_amplitude(amplitude), m_frequency(frequency), m_speed(speed), m_direction(glm::normalize(direction)),
      m_waveVector(glm::normalize(waveVector))
{
    updateFunction();
}

std::shared_ptr<WaveDeformer> WaveDeformer::create(float amplitude, float frequency, float speed,
                                                   const glm::vec3 &direction, const glm::vec3 &waveVector)
{
    return std::make_shared<WaveDeformer>(amplitude, frequency, speed, direction, waveVector);
}

void WaveDeformer::setAmplitude(float amplitude)
{
    m_amplitude = amplitude;
    updateFunction();
}

float WaveDeformer::amplitude() const
{
    return m_amplitude;
}

void WaveDeformer::setFrequency(float frequency)
{
    m_frequency = frequency;
    updateFunction();
}

float WaveDeformer::frequency() const
{
    return m_frequency;
}

void WaveDeformer::setSpeed(float speed)
{
    m_speed = speed;
    updateFunction();
}

float WaveDeformer::speed() const
{
    return m_speed;
}

void WaveDeformer::setDirection(const glm::vec3 &direction)
{
    m_direction = glm::normalize(direction);
    updateFunction();
}

glm::vec3 WaveDeformer::direction() const
{
    return m_direction;
}

void WaveDeformer::setWaveVector(const glm::vec3 &waveVector)
{
    m_waveVector = glm::normalize(waveVector);
    updateFunction();
}

glm::vec3 WaveDeformer::waveVector() const
{
    return m_waveVector;
}

std::unique_ptr<Deformer> WaveDeformer::clone() const
{
    auto cloned = std::make_unique<WaveDeformer>(m_amplitude, m_frequency, m_speed, m_direction, m_waveVector);
    cloned->setWeight(m_weight);
    cloned->setEnabled(m_enabled);
    cloned->setTime(time());
    return cloned;
}

void WaveDeformer::updateFunction()
{
    setFunction([this](const glm::vec3 &pos) -> glm::vec3 {
        float phase = glm::dot(pos, m_direction) * m_frequency - time() * m_speed;
        float wave = std::sin(phase) * m_amplitude;
        return m_waveVector * wave;
    });
}

void WaveDeformer::setupProperties()
{
    FunctionDeformer::setupProperties();
    addProperty("Amplitude", &m_amplitude, "Wave");
    addProperty("Frequency", &m_frequency, "Wave");
    addProperty("Speed", &m_speed, "Wave");
    addProperty("Direction", &m_direction, "Wave");
    addProperty("Wave Vector", &m_waveVector, "Wave");
}
