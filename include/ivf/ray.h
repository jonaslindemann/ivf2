#pragma once

/**
 * @file ray.h
 * @brief Rays for picking: camera rays from window coordinates, ray-box and ray-triangle tests.
 */

#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @brief A ray: origin + t * direction.
 *
 * Camera rays have a unit direction, so t is a distance. Transformed rays (into a node's
 * local space) keep their t: the direction is transformed without normalising.
 */
struct Ray {
    glm::vec3 origin{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};

    [[nodiscard]] glm::vec3 at(float t) const
    {
        return origin + t * direction;
    }

    /** @brief The ray in the space that @p matrix maps world space to (t values unchanged). */
    [[nodiscard]] Ray transformed(const glm::mat4 &matrix) const
    {
        return {glm::vec3(matrix * glm::vec4(origin, 1.0f)), glm::vec3(matrix * glm::vec4(direction, 0.0f))};
    }
};

/**
 * @brief The ray through a point of the window, from the camera into the scene.
 * @param x, y Window coordinates (origin at the top left, y down), as mouse positions.
 * @param width, height Window size in the same units.
 * @return World-space ray with a unit direction, starting on the near plane.
 */
[[nodiscard]] Ray rayFromScreen(float x, float y, float width, float height, const glm::mat4 &view,
                                const glm::mat4 &projection);

/**
 * @brief Intersect a ray with an axis-aligned box.
 * @param tNear, tFar Receive where the ray enters and leaves the box (tNear may be negative
 *        if the origin is inside).
 * @return True if the ray hits the box at some t >= 0.
 */
[[nodiscard]] bool intersectBox(const Ray &ray, const glm::vec3 &boxMin, const glm::vec3 &boxMax, float &tNear,
                                float &tFar);

/** @brief intersectBox() for a BoundingBox (false for an invalid box). */
[[nodiscard]] bool intersectBox(const Ray &ray, const BoundingBox &box, float &tNear, float &tFar);

/**
 * @brief Intersect a ray with a triangle (both sides; Möller-Trumbore).
 * @param t Receives the ray parameter of the hit (> 0).
 * @param u, v Receive the barycentric coordinates of the hit (weights of v1 and v2).
 */
[[nodiscard]] bool intersectTriangle(const Ray &ray, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2,
                                     float &t, float &u, float &v);

} // namespace ivf
