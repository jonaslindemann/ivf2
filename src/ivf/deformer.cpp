#include <ivf/deformer.h>
#include <ivf/utils.h>

using namespace ivf;

// Base Deformer Implementation
Deformer::Deformer() : m_enabled(true), m_weight(1.0f)
{}

void Deformer::setInput(std::shared_ptr<Vertices> vertices)
{
    m_originalVertices = vertices;
    if (vertices)
    {
        // Create a copy for deformed vertices
        m_deformedVertices = std::make_shared<Vertices>(vertices->rows());
        // Copy original data
        std::memcpy(m_deformedVertices->data(), vertices->data(), vertices->memSize());
    }
}

std::shared_ptr<Vertices> Deformer::getOutput()
{
    return m_deformedVertices;
}

void Deformer::reset()
{
    if (m_originalVertices && m_deformedVertices)
    {
        std::memcpy(m_deformedVertices->data(), m_originalVertices->data(), m_originalVertices->memSize());
    }
}

void Deformer::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool Deformer::enabled() const
{
    return m_enabled;
}

void Deformer::setWeight(float weight)
{
    m_weight = glm::clamp(weight, 0.0f, 1.0f);
}

float Deformer::weight() const
{
    return m_weight;
}

void ivf::Deformer::setupProperties()
{
    addProperty("Enabled", &m_enabled, "Deformer");
    addProperty("Weight", &m_weight, 0.0f, 1.0f, "Deformer");
}
