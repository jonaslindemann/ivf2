#include <ivf/turbulence_deformer.h>
#include <cmath>
#include <algorithm>

using namespace ivf;

TurbulenceDeformer::TurbulenceDeformer(float scale, float intensity, int octaves, float persistence,
                                       float animationSpeed)
    : m_scale(scale), m_intensity(intensity), m_octaves(octaves), m_persistence(persistence),
      m_animationSpeed(animationSpeed), m_seed(12345)
{
    updateFunction();
}

std::shared_ptr<TurbulenceDeformer> TurbulenceDeformer::create(float scale, float intensity, int octaves,
                                                               float persistence, float animationSpeed)
{
    return std::make_shared<TurbulenceDeformer>(scale, intensity, octaves, persistence, animationSpeed);
}

void TurbulenceDeformer::setScale(float scale)
{
    m_scale = scale;
    updateFunction();
}

float TurbulenceDeformer::scale() const
{
    return m_scale;
}

void TurbulenceDeformer::setIntensity(float intensity)
{
    m_intensity = intensity;
    updateFunction();
}

float TurbulenceDeformer::intensity() const
{
    return m_intensity;
}

void TurbulenceDeformer::setOctaves(int octaves)
{
    m_octaves = std::max(1, octaves);
    updateFunction();
}

int TurbulenceDeformer::octaves() const
{
    return m_octaves;
}

void TurbulenceDeformer::setPersistence(float persistence)
{
    m_persistence = persistence;
    updateFunction();
}

float TurbulenceDeformer::persistence() const
{
    return m_persistence;
}

void TurbulenceDeformer::setAnimationSpeed(float speed)
{
    m_animationSpeed = speed;
}

float TurbulenceDeformer::animationSpeed() const
{
    return m_animationSpeed;
}

void TurbulenceDeformer::setSeed(unsigned int seed)
{
    m_seed = seed;
    updateFunction();
}

unsigned int TurbulenceDeformer::seed() const
{
    return m_seed;
}

std::unique_ptr<Deformer> TurbulenceDeformer::clone() const
{
    auto cloned =
        std::make_unique<TurbulenceDeformer>(m_scale, m_intensity, m_octaves, m_persistence, m_animationSpeed);
    cloned->setSeed(m_seed);
    cloned->setWeight(m_weight);
    cloned->setEnabled(m_enabled);
    cloned->setTime(time());
    return cloned;
}

void TurbulenceDeformer::updateFunction()
{
    setFunction([this](const glm::vec3 &pos) -> glm::vec3 {
        glm::vec3 animatedPos = pos + glm::vec3(time() * m_animationSpeed);

        float turbX = turbulence(animatedPos * m_scale, m_octaves, m_persistence, m_seed);
        float turbY = turbulence(animatedPos * m_scale + glm::vec3(100, 0, 0), m_octaves, m_persistence, m_seed);
        float turbZ = turbulence(animatedPos * m_scale + glm::vec3(0, 100, 0), m_octaves, m_persistence, m_seed);

        return glm::vec3(turbX, turbY, turbZ) * m_intensity;
    });
}

float TurbulenceDeformer::turbulence(const glm::vec3 &position, int octaves, float persistence, unsigned int seed)
{
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;

    for (int i = 0; i < octaves; ++i)
    {
        // Simple noise function
        glm::vec3 p = position * frequency;
        unsigned int hash = ((int(p.x) * 73856093) ^ (int(p.y) * 19349663) ^ (int(p.z) * 83492791) ^ seed) & 0x7fffffff;
        float noise = (hash % 1000) / 500.0f - 1.0f; // -1 to 1

        value += std::abs(noise) * amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return value;
}

void TurbulenceDeformer::setupProperties()
{
    FunctionDeformer::setupProperties();
    addProperty("Scale", &m_scale, "Turbulence");
    addProperty("Intensity", &m_intensity, "Turbulence");
    addProperty("Octaves", &m_octaves, "Turbulence");
    addProperty("Persistence", &m_persistence, "Turbulence");
    addProperty("Animation Speed", &m_animationSpeed, "Turbulence");
    // addProperty("Seed", &m_seed, "Turbulence");
}
