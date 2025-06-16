#include <ivf/function_deformer.h>
#include <cmath>
#include <algorithm>

using namespace ivf;

// FunctionDeformer Implementation
FunctionDeformer::FunctionDeformer() : m_scale(1.0f), m_offset(0.0f), m_useLocalSpace(true), m_time(0.0f)
{}

FunctionDeformer::FunctionDeformer(const DisplacementFunction &func)
    : m_displacementFunction(func), m_scale(1.0f), m_offset(0.0f), m_useLocalSpace(true), m_time(0.0f)
{}

std::shared_ptr<FunctionDeformer> FunctionDeformer::create()
{
    return std::make_shared<FunctionDeformer>();
}

std::shared_ptr<FunctionDeformer> FunctionDeformer::create(const DisplacementFunction &func)
{
    return std::make_shared<FunctionDeformer>(func);
}

void FunctionDeformer::setFunction(const DisplacementFunction &func)
{
    m_displacementFunction = func;
}

void FunctionDeformer::setScale(const glm::vec3 &scale)
{
    m_scale = scale;
}

void FunctionDeformer::setScale(float uniformScale)
{
    m_scale = glm::vec3(uniformScale);
}

glm::vec3 FunctionDeformer::scale() const
{
    return m_scale;
}

void FunctionDeformer::setOffset(const glm::vec3 &offset)
{
    m_offset = offset;
}

glm::vec3 FunctionDeformer::offset() const
{
    return m_offset;
}

void FunctionDeformer::setUseLocalSpace(bool useLocal)
{
    m_useLocalSpace = useLocal;
}

bool FunctionDeformer::useLocalSpace() const
{
    return m_useLocalSpace;
}

void FunctionDeformer::setTime(float time)
{
    m_time = time;
}

float FunctionDeformer::time() const
{
    return m_time;
}

void FunctionDeformer::apply()
{
    if (!m_enabled || !m_originalVertices || !m_deformedVertices || !m_displacementFunction)
    {
        return;
    }

    for (GLuint i = 0; i < m_originalVertices->rows(); ++i)
    {
        glm::vec3 originalPos = m_originalVertices->vertex(i);
        glm::vec3 inputPos = originalPos + m_offset;

        // Apply the displacement function
        glm::vec3 displacement = m_displacementFunction(inputPos);
        displacement *= m_scale * m_weight;

        glm::vec3 deformedPos = originalPos + displacement;
        m_deformedVertices->setVertex(i, deformedPos.x, deformedPos.y, deformedPos.z);
    }
}

std::unique_ptr<Deformer> FunctionDeformer::clone() const
{
    auto cloned = std::make_unique<FunctionDeformer>(m_displacementFunction);
    cloned->setScale(m_scale);
    cloned->setOffset(m_offset);
    cloned->setUseLocalSpace(m_useLocalSpace);
    cloned->setTime(m_time);
    cloned->setWeight(m_weight);
    cloned->setEnabled(m_enabled);
    return cloned;
}

FunctionDeformer::DisplacementFunction FunctionDeformer::createSineWave(float frequency, float amplitude,
                                                                        const glm::vec3 &direction)
{

    return [frequency, amplitude, direction](const glm::vec3 &pos) -> glm::vec3 {
        float wave = std::sin(glm::dot(pos, direction) * frequency) * amplitude;
        return direction * wave;
    };
}

FunctionDeformer::DisplacementFunction FunctionDeformer::createRipple(float frequency, float amplitude,
                                                                      const glm::vec3 &center)
{

    return [frequency, amplitude, center](const glm::vec3 &pos) -> glm::vec3 {
        glm::vec3 toCenter = pos - center;
        float distance = glm::length(toCenter);
        if (distance < 0.001f)
            return glm::vec3(0.0f);

        float ripple = std::sin(distance * frequency) * amplitude;
        glm::vec3 direction = glm::normalize(toCenter);
        return direction * ripple;
    };
}

FunctionDeformer::DisplacementFunction FunctionDeformer::createTurbulence(float scale, float intensity, int octaves)
{

    return [scale, intensity, octaves](const glm::vec3 &pos) -> glm::vec3 {
        glm::vec3 scaledPos = pos * scale;
        glm::vec3 displacement(0.0f);

        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; ++i)
        {
            // Simple pseudo-noise (replace with proper noise implementation)
            float x = std::sin(scaledPos.x * frequency + scaledPos.y * 0.1f) * amplitude;
            float y = std::sin(scaledPos.y * frequency + scaledPos.z * 0.1f) * amplitude;
            float z = std::sin(scaledPos.z * frequency + scaledPos.x * 0.1f) * amplitude;

            displacement += glm::vec3(x, y, z);

            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return displacement * intensity;
    };
}

FunctionDeformer::DisplacementFunction FunctionDeformer::createSpiral(float pitch, float radius)
{
    return [pitch, radius](const glm::vec3 &pos) -> glm::vec3 {
        float angle = pos.y * pitch;
        float spiralRadius = radius * (1.0f + pos.y * 0.1f);

        glm::vec3 spiralOffset(std::cos(angle) * spiralRadius - pos.x, 0.0f, std::sin(angle) * spiralRadius - pos.z);

        return spiralOffset * 0.1f; // Scale down the effect
    };
}

FunctionDeformer::DisplacementFunction FunctionDeformer::createTwirl(const glm::vec3 &center, float strength,
                                                                     float falloff)
{

    return [center, strength, falloff](const glm::vec3 &pos) -> glm::vec3 {
        glm::vec3 toCenter = pos - center;
        float distance = glm::length(toCenter);

        if (distance < 0.001f)
            return glm::vec3(0.0f);

        float weight = std::exp(-distance * falloff);
        float angle = strength * weight;

        // Rotate around Y-axis
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);

        glm::vec3 rotated(toCenter.x * cosA - toCenter.z * sinA, toCenter.y, toCenter.x * sinA + toCenter.z * cosA);

        return (rotated - toCenter) * weight;
    };
}

FunctionDeformer::DisplacementFunction FunctionDeformer::createGravityWell(const glm::vec3 &center, float strength,
                                                                           float radius)
{

    return [center, strength, radius](const glm::vec3 &pos) -> glm::vec3 {
        glm::vec3 toCenter = center - pos;
        float distance = glm::length(toCenter);

        if (distance < 0.001f || distance > radius)
            return glm::vec3(0.0f);

        float weight = 1.0f - (distance / radius);
        weight = weight * weight; // Quadratic falloff

        return glm::normalize(toCenter) * strength * weight;
    };
}

void ivf::FunctionDeformer::setupProperties()
{
    Deformer::setupProperties();
    addProperty("Scale", &m_scale, "Function");
    addProperty("Offset", &m_offset, "Function");
    addProperty("Use local space", &m_useLocalSpace, "Function");
    // addProperty("Time", &m_time, "Time variable for time-dependent functions");
}
