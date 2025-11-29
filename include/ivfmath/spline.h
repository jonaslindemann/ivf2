#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace ivfmath {

class Spline {
public:
    Spline();

    static std::shared_ptr<Spline> create();

    // Add a point to the spline
    void addPoint(const glm::vec3 &point);

    // Get position on spline using parameter t
    // t ranges from 0 to (number of points - 1)
    glm::vec3 positionByT(float t) const;

    // Get position on spline using distance from start
    glm::vec3 positionByDistance(float distance) const;

    // Get total length of the spline
    float totalLength() const;

    // Get number of points in the spline
    size_t size() const;

    // Clear all points
    void clear();

private:
    std::vector<glm::vec3> m_points;
    std::vector<float> m_distances; // Cumulative distances from start

    // Recalculate cumulative distances when points change
    void updateDistances();

    // Convert distance to t parameter
    float distanceToT(float distance) const;

    // Cubic interpolation between points
    glm::vec3 interpolate(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
                          float t) const;
};

using SplinePtr = std::shared_ptr<Spline>;

} // namespace ivfmath
