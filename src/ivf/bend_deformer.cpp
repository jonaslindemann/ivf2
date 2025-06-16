#include <ivf/bend_deformer.h>
#include <ivf/utils.h>

using namespace ivf;

BendDeformer::BendDeformer(const glm::vec3 &axis, const glm::vec3 &center)
    : m_axis(glm::normalize(axis)), m_center(center), m_curvature(0.0f), m_startDistance(0.0f), m_endDistance(10.0f)
{}

std::shared_ptr<BendDeformer> BendDeformer::create(const glm::vec3 &axis, const glm::vec3 &center)
{
    return std::make_shared<BendDeformer>(axis, center);
}

void BendDeformer::setAxis(const glm::vec3 &axis)
{
    m_axis = glm::normalize(axis);
}

void BendDeformer::setCenter(const glm::vec3 &center)
{
    m_center = center;
}

void BendDeformer::setCurvature(float curvature)
{
    m_curvature = curvature;
}

void BendDeformer::setDistanceRange(float start, float end)
{
    m_startDistance = start;
    m_endDistance = end;
}

void BendDeformer::apply()
{
    if (!m_enabled || !m_originalVertices || !m_deformedVertices)
        return;

    // Create a perpendicular vector to the axis for bending direction
    glm::vec3 bendAxis =
        (std::abs(glm::dot(m_axis, glm::vec3(1, 0, 0))) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    bendAxis = glm::normalize(glm::cross(m_axis, bendAxis));

    for (GLuint i = 0; i < m_originalVertices->rows(); ++i)
    {
        glm::vec3 originalPos = m_originalVertices->vertex(i);
        glm::vec3 relativePos = originalPos - m_center;

        float axisDistance = glm::dot(relativePos, m_axis);
        float weight = calculateWeight(std::abs(axisDistance), m_startDistance, m_endDistance, 1.0f);
        weight *= m_weight;

        if (weight > 0.0f)
        {
            // Apply bending transformation
            float bendAmount = m_curvature * axisDistance * weight;
            glm::vec3 offset = bendAxis * bendAmount;
            glm::vec3 deformedPos = originalPos + offset;

            glm::vec3 finalPos = glm::mix(originalPos, deformedPos, weight);
            m_deformedVertices->setVertex(i, finalPos.x, finalPos.y, finalPos.z);
        }
        else
        {
            m_deformedVertices->setVertex(i, originalPos.x, originalPos.y, originalPos.z);
        }
    }
}

std::unique_ptr<Deformer> BendDeformer::clone() const
{
    auto cloned = std::make_unique<BendDeformer>(m_axis, m_center);
    cloned->setCurvature(m_curvature);
    cloned->setDistanceRange(m_startDistance, m_endDistance);
    cloned->setWeight(m_weight);
    cloned->setEnabled(m_enabled);
    return cloned;
}

void ivf::BendDeformer::setupProperties()
{
    Deformer::setupProperties();
    addProperty("Axis", &m_axis, "Bend");
    addProperty("Center", &m_center, "Bend");
    addProperty("Curvature", &m_curvature, "Bend");
    addProperty("Start distance", &m_startDistance, "Bend");
    addProperty("End distance", &m_endDistance, "Bend");
}
