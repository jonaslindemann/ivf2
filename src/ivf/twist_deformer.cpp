#include <ivf/twist_deformer.h>
#include <ivf/utils.h>

using namespace ivf;

TwistDeformer::TwistDeformer(const glm::vec3 &axis, const glm::vec3 &center)
    : m_axis(glm::normalize(axis)), m_center(center), m_angle(0.0f), m_falloff(1.0f), m_startDistance(0.0f),
      m_endDistance(10.0f)
{}

std::shared_ptr<TwistDeformer> TwistDeformer::create(const glm::vec3 &axis, const glm::vec3 &center)
{
    return std::make_shared<TwistDeformer>(axis, center);
}

void TwistDeformer::setAxis(const glm::vec3 &axis)
{
    m_axis = glm::normalize(axis);
}

glm::vec3 TwistDeformer::axis() const
{
    return m_axis;
}

void TwistDeformer::setCenter(const glm::vec3 &center)
{
    m_center = center;
}

glm::vec3 TwistDeformer::center() const
{
    return m_center;
}

void TwistDeformer::setAngle(float angle)
{
    m_angle = angle;
}

float TwistDeformer::angle() const
{
    return m_angle;
}

void TwistDeformer::setFalloff(float falloff)
{
    m_falloff = falloff;
}

float TwistDeformer::falloff() const
{
    return m_falloff;
}

void TwistDeformer::setDistanceRange(float start, float end)
{
    m_startDistance = start;
    m_endDistance = end;
}

float TwistDeformer::startDistance() const
{
    return m_startDistance;
}

float TwistDeformer::endDistance() const
{
    return m_endDistance;
}

void TwistDeformer::apply()
{
    if (!m_enabled || !m_originalVertices || !m_deformedVertices)
        return;

    for (GLuint i = 0; i < m_originalVertices->rows(); ++i)
    {
        glm::vec3 originalPos = m_originalVertices->vertex(i);
        glm::vec3 relativePos = originalPos - m_center;

        // Project onto axis to get distance along axis
        float axisDistance = glm::dot(relativePos, m_axis);

        // Calculate weight based on distance
        float weight = calculateWeight(std::abs(axisDistance), m_startDistance, m_endDistance, m_falloff);
        weight *= m_weight;

        if (weight > 0.0f)
        {
            // Calculate twist angle based on distance and weight
            float twistAngle = m_angle * weight;

            // Rotate around the axis
            glm::vec3 deformedPos = glm::rotate(relativePos, twistAngle, m_axis) + m_center;

            // Blend with original position
            glm::vec3 finalPos = glm::mix(originalPos, deformedPos, weight);

            m_deformedVertices->setVertex(i, finalPos.x, finalPos.y, finalPos.z);
        }
        else
        {
            // No deformation, keep original
            m_deformedVertices->setVertex(i, originalPos.x, originalPos.y, originalPos.z);
        }
    }
}

std::unique_ptr<Deformer> TwistDeformer::clone() const
{
    auto cloned = std::make_unique<TwistDeformer>(m_axis, m_center);
    cloned->setAngle(m_angle);
    cloned->setFalloff(m_falloff);
    cloned->setDistanceRange(m_startDistance, m_endDistance);
    cloned->setWeight(m_weight);
    cloned->setEnabled(m_enabled);
    return cloned;
}

void TwistDeformer::setupProperties()
{
    Deformer::setupProperties();
    addProperty("Axis", &m_axis, "Twist");
    addProperty("Angle", &m_angle, "Twist");
    addProperty("Falloff", &m_falloff, "Twist");
    addProperty("Start distance", &m_startDistance, "Twist");
    addProperty("End distance", &m_endDistance, "Twist");
}
