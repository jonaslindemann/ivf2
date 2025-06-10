#pragma once

#include <ivf/deformer.h>

namespace ivf {

class ScaleDeformer : public Deformer {
private:
    glm::vec3 m_center;
    glm::vec3 m_scale;
    float m_falloff;
    
public:
    ScaleDeformer(const glm::vec3& center = glm::vec3(0, 0, 0),
                  const glm::vec3& scale = glm::vec3(1, 1, 1));
    
    static std::shared_ptr<ScaleDeformer> create(
        const glm::vec3& center = glm::vec3(0, 0, 0),
        const glm::vec3& scale = glm::vec3(1, 1, 1));
    
    void setCenter(const glm::vec3& center);
    void setScale(const glm::vec3& scale);
    void setFalloff(float falloff);
    
    virtual void apply() override;
    virtual std::unique_ptr<Deformer> clone() const override;
};

};