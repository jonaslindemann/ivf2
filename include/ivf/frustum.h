#pragma once
/**
 * @file frustum.h
 * @brief View frustum for culling bounding boxes.
 */

#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

#include <array>

namespace ivf {

/**
 * @brief The six planes of a view volume, extracted from a view-projection matrix.
 *
 * Works for perspective, orthographic (shadow maps) and oblique (planar reflection)
 * projections. A default-constructed frustum contains everything.
 */
class Frustum {
public:
    /** @brief A frustum that contains everything. */
    Frustum() = default;

    /**
     * @brief Frustum of the clip volume of @p viewProjection (projection * view).
     * @param viewProjection Matrix from world to clip coordinates (OpenGL conventions).
     */
    explicit Frustum(const glm::mat4 &viewProjection);

    /**
     * @brief True if @p box may be visible: it is not completely outside any plane.
     *
     * Conservative: boxes near the corners of the frustum can pass although they are
     * outside. Invalid boxes are always reported as visible.
     * @param box World-space bounding box.
     */
    [[nodiscard]] bool intersects(const BoundingBox &box) const;

    /** @brief True if constructed from a matrix (false: contains everything). */
    [[nodiscard]] bool isValid() const
    {
        return m_valid;
    }

private:
    std::array<glm::vec4, 6> m_planes{}; ///< (normal, distance), normals point inwards.
    bool m_valid{false};
};

} // namespace ivf
