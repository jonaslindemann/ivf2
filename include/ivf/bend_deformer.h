#pragma once

#include <ivf/deformer.h>

namespace ivf {

class BendDeformer : public Deformer {
private:
    glm::vec3 m_axis;
    glm::vec3 m_center;
    float m_curvature;
    float m_startDistance;
    float m_endDistance;
    
public:
    BendDeformer(const glm::vec3& axis = glm::vec3(0, 1, 0),
                 const glm::vec3& center = glm::vec3(0, 0, 0));
    
    static std::shared_ptr<BendDeformer> create(
        const glm::vec3& axis = glm::vec3(0, 1, 0),
        const glm::vec3& center = glm::vec3(0, 0, 0));
    
    void setAxis(const glm::vec3& axis);
    void setCenter(const glm::vec3& center);
    void setCurvature(float curvature);
    void setDistanceRange(float start, float end);
    
    virtual void apply() override;
    virtual std::unique_ptr<Deformer> clone() const override;
};

};