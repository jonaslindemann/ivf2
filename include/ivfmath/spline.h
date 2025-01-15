#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <stdexcept>

namespace ivfmath {

class Spline {
private:
    std::vector<glm::vec3> m_controlPoints;
    std::vector<float> m_segmentLengths; // Length of each segment
    float m_totalLength;                 // Total spline length

    // Number of steps used for length approximation
    static constexpr int LENGTH_SAMPLES = 100;

    glm::vec3 interpolate(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
                          float t) const;

    void calculateLengths();

public:
    Spline();

    static std::shared_ptr<Spline> create();

    void addPoint(const glm::vec3 &point);

    // Get point by parameter t (0 <= t <= n-1)
    glm::vec3 getPointByT(float t) const;

    // Get point by distance along the spline (0 <= distance <= totalLength)
    glm::vec3 getPointByDistance(float distance) const;

    // Get total spline length
    float getLength() const;

    // Get number of control points
    size_t size() const;
};

typedef std::shared_ptr<Spline> SplinePtr;

}; // namespace ivfmath
