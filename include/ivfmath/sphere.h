#pragma once

#include <memory>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ivfmath {

class Sphere {
private:
    glm::vec3 m_center{0.0, 0.0, 0.0};
    float m_radius;

public:
    Sphere() = default;
    virtual ~Sphere() = default;

    void setCenter(const glm::vec3 &center);
    void setCenter(float x, float y, float z);
    void setRadius(float radius);

    glm::vec3 center() const;
    float radius() const;

    bool contains(const glm::vec3 &point) const;
    bool contains(float x, float y, float z) const;
    bool contains(const Sphere &sphere) const;
};
}; // namespace ivf
