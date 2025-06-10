#include <ivf/scale_deformer.h>
#include <ivf/utils.h>

using namespace ivf;

ScaleDeformer::ScaleDeformer(const glm::vec3& center, const glm::vec3& scale)
    : m_center(center), m_scale(scale), m_falloff(1.0f) {}

std::shared_ptr<ScaleDeformer> ScaleDeformer::create(const glm::vec3& center, 
                                                     const glm::vec3& scale) {
    return std::make_shared<ScaleDeformer>(center, scale);
}

void ScaleDeformer::setCenter(const glm::vec3& center) {
    m_center = center;
}

void ScaleDeformer::setScale(const glm::vec3& scale) {
    m_scale = scale;
}

void ScaleDeformer::setFalloff(float falloff) {
    m_falloff = falloff;
}

void ScaleDeformer::apply() {
    if (!m_enabled || !m_originalVertices || !m_deformedVertices) return;
    
    for (GLuint i = 0; i < m_originalVertices->rows(); ++i) {
        glm::vec3 originalPos = m_originalVertices->vertex(i);
        glm::vec3 relativePos = originalPos - m_center;
        
        float distance = glm::length(relativePos);
        float weight = std::exp(-distance * m_falloff) * m_weight;
        
        if (weight > 0.0f) {
            glm::vec3 scaledPos = relativePos * glm::mix(glm::vec3(1.0f), m_scale, weight);
            glm::vec3 deformedPos = scaledPos + m_center;
            
            m_deformedVertices->setVertex(i, deformedPos.x, deformedPos.y, deformedPos.z);
        } else {
            m_deformedVertices->setVertex(i, originalPos.x, originalPos.y, originalPos.z);
        }
    }
}

std::unique_ptr<Deformer> ScaleDeformer::clone() const {
    auto cloned = std::make_unique<ScaleDeformer>(m_center, m_scale);
    cloned->setFalloff(m_falloff);
    cloned->setWeight(m_weight);
    cloned->setEnabled(m_enabled);
    return cloned;
}