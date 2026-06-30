#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>

namespace ivf {

/**
 * @class ExtrusionProfile
 * @brief A 2D cross-section (profile) to be swept along a 3D path.
 *
 * An ExtrusionProfile holds an ordered list of 2D points describing a cross-section in the
 * local (normal, binormal) plane of a path frame. The profile may be open or closed and can
 * optionally carry explicit per-point 2D normals. When no normals are supplied, smooth normals
 * are computed from adjacent edges.
 *
 * A set of static factory methods construct common profiles (circle, rectangle, rounded
 * rectangle, regular polygon, arbitrary polyline). This subsumes the hard-coded circular
 * cross-section previously used by the solid tube classes.
 */
class ExtrusionProfile {
private:
    std::vector<glm::vec2> m_points;  ///< Cross-section points in profile space.
    std::vector<glm::vec2> m_normals; ///< Optional explicit per-point 2D normals.
    bool m_closed{true};              ///< Whether the profile forms a closed loop.

public:
    ExtrusionProfile() = default;

    /**
     * @brief Construct a profile from a list of 2D points.
     * @param points Cross-section points.
     * @param closed Whether the profile is a closed loop.
     */
    explicit ExtrusionProfile(std::vector<glm::vec2> points, bool closed = true);

    /// @name Factory methods
    /// @{

    /**
     * @brief Create a circular profile.
     * @param radius Circle radius.
     * @param segments Number of points around the circle.
     * @param start Start angle in radians.
     * @param sweep Sweep angle in radians (2*pi for a full circle).
     */
    static ExtrusionProfile circle(float radius = 1.0f, int segments = 32, float start = 0.0f,
                                   float sweep = glm::two_pi<float>());

    /**
     * @brief Create a rectangular profile centered at the origin.
     */
    static ExtrusionProfile rectangle(float width = 1.0f, float height = 1.0f);

    /**
     * @brief Create a rounded rectangle profile centered at the origin.
     * @param width Total width.
     * @param height Total height.
     * @param radius Corner radius.
     * @param cornerSegments Number of segments per rounded corner.
     */
    static ExtrusionProfile roundedRect(float width, float height, float radius, int cornerSegments = 4);

    /**
     * @brief Create a regular polygon profile.
     * @param sides Number of sides.
     * @param radius Circumscribed radius.
     */
    static ExtrusionProfile regularPolygon(int sides, float radius = 1.0f);

    /**
     * @brief Create a profile from an arbitrary list of 2D points.
     */
    static ExtrusionProfile polyline(const std::vector<glm::vec2> &points, bool closed = true);

    /// @}

    void addPoint(const glm::vec2 &p);
    void setPoints(const std::vector<glm::vec2> &points);
    void clear();

    const std::vector<glm::vec2> &points() const;
    std::size_t pointCount() const;

    void setClosed(bool closed);
    bool closed() const;

    /**
     * @brief Set explicit per-point 2D normals (must match the number of points).
     */
    void setNormals(const std::vector<glm::vec2> &normals);
    const std::vector<glm::vec2> &normals() const;
    bool hasNormals() const;

    /**
     * @brief Geometric centroid of the profile points.
     */
    glm::vec2 centroid() const;

    /**
     * @brief Compute smooth per-point 2D normals (averaged adjacent edge normals, oriented
     *        outward from the centroid). Returns one normal per point.
     */
    std::vector<glm::vec2> computeSmoothNormals() const;
};

} // namespace ivf
