#include <ivf/flow_field.h>
#include <ivf/math_utils.h>
#include <algorithm>
#include <cmath>

using namespace ivf;

std::shared_ptr<FlowField> FlowField::create()
{
    return std::make_shared<FlowField>();
}

glm::vec3 FlowField::sampleVelocity(glm::vec3 pos) const
{
    // Transform to noise domain
    float px = (pos.x + m_offset.x) * m_scale;
    float py = (pos.y + m_offset.y) * m_scale;
    float pz = (pos.z + m_offset.z) * m_scale;
    float t  = m_time * m_timeScale;

    // Accumulate fBm-layered curl noise
    glm::vec3 result(0.0f);
    float amplitude  = 1.0f;
    float frequency  = 1.0f;
    float totalAmp   = 0.0f;

    for (int i = 0; i < m_octaves; ++i) {
        result   += amplitude * curlNoise3D(px * frequency, py * frequency, pz * frequency, t);
        totalAmp += amplitude;
        amplitude *= m_gain;
        frequency *= m_lacunarity;
    }

    if (totalAmp > 0.0f)
        result /= totalAmp;

    return result * m_strength;
}

glm::vec2 FlowField::sampleVelocity2D(float x, float z) const
{
    float px = (x + m_offset.x) * m_scale;
    float pz = (z + m_offset.z) * m_scale;
    float t  = m_time * m_timeScale;

    glm::vec2 result(0.0f);
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float totalAmp  = 0.0f;

    for (int i = 0; i < m_octaves; ++i) {
        result   += amplitude * curlNoise2D(px * frequency, pz * frequency, t);
        totalAmp += amplitude;
        amplitude *= m_gain;
        frequency *= m_lacunarity;
    }

    if (totalAmp > 0.0f)
        result /= totalAmp;

    return result * m_strength;
}

void FlowField::applyToParticleSystem(std::shared_ptr<ParticleSystem> ps, float blendRate)
{
    auto weakField = std::weak_ptr<FlowField>(shared_from_this());

    ps->setUpdateFunction([weakField, blendRate](ParticleSystem::Particle &p, float dt) {
        auto field = weakField.lock();
        if (!field) {
            p.position += p.velocity * dt;
            return;
        }
        glm::vec3 flowVel = field->sampleVelocity(p.position);
        float rate = std::min(1.0f, blendRate * dt);
        p.velocity = glm::mix(p.velocity, flowVel, rate);
        p.position += p.velocity * dt;  // custom updateFn replaces the else-branch in ParticleSystem::update
    });
}
