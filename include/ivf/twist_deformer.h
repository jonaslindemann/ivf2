#pragma once

#include <ivf/deformer.h>

namespace ivf {

// Twist deformer implementation
class TwistDeformer : public Deformer {
private:
    glm::vec3 m_axis;      // Twist axis (normalized)
    glm::vec3 m_center;    // Center point of twist
    float m_angle;         // Twist angle in radians
    float m_falloff;       // Distance falloff factor
    float m_startDistance; // Distance where twist starts
    float m_endDistance;   // Distance where twist ends

public:
    TwistDeformer(const glm::vec3 &axis = glm::vec3(0, 1, 0), const glm::vec3 &center = glm::vec3(0, 0, 0));

    static std::shared_ptr<TwistDeformer> create(const glm::vec3 &axis = glm::vec3(0, 1, 0),
                                                 const glm::vec3 &center = glm::vec3(0, 0, 0));

    // Twist-specific parameters
    void setAxis(const glm::vec3 &axis);
    glm::vec3 axis() const;
    void setCenter(const glm::vec3 &center);
    glm::vec3 center() const;
    void setAngle(float angle);
    float angle() const;
    void setFalloff(float falloff);
    float falloff() const;
    void setDistanceRange(float start, float end);
    float startDistance() const;
    float endDistance() const;

    virtual void apply() override;
    virtual std::unique_ptr<Deformer> clone() const override;
};

typedef std::shared_ptr<TwistDeformer> TwistDeformerPtr;

}; // namespace ivf
