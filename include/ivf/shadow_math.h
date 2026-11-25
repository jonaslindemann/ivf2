#pragma once

/**
 * @file shadow_math.h
 * @brief Shadow map matrices: cascades for directional lights, cube faces for point lights.
 *
 * Pure math without OpenGL, used by LightManager::renderShadowMaps() (and the unit tests).
 */

#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

#include <array>
#include <vector>

namespace ivf {

/// How shadow map samples are filtered (Light::setShadowFilter()).
enum class ShadowFilter {
    Hard, ///< One sample: sharp, aliased edges.
    Pcf,  ///< Percentage-closer filtering: a square of (2r + 1)^2 samples (r: Light::setShadowFilterRadius()).
    Pcss  ///< Percentage-closer soft shadows: penumbras that widen with the distance to the caster.
};

/// Number of ShadowFilter values.
inline constexpr int ShadowFilterCount = 3;

/** @brief Name of a filter ("Hard", "PCF", "PCSS"). */
[[nodiscard]] const char *shadowFilterName(ShadowFilter filter);

/**
 * @brief Near and far plane distances of a perspective or orthographic projection matrix.
 */
[[nodiscard]] glm::vec2 projectionNearFar(const glm::mat4 &projection);

/**
 * @brief Far distances of @p count cascades between @p nearDist and @p farDist.
 *
 * Mixes logarithmic (@p lambda = 1, even resolution per distance) and uniform (@p lambda
 * = 0) splits ("practical split scheme"). The last value is @p farDist.
 */
[[nodiscard]] std::vector<float> cascadeSplits(float nearDist, float farDist, int count, float lambda);

/**
 * @brief Corners of the part of the camera frustum between view distances @p fromDist and @p toDist.
 *
 * The first four corners lie at @p fromDist, the last four at @p toDist (world space).
 */
[[nodiscard]] std::array<glm::vec3, 8> frustumSliceCorners(const glm::mat4 &view, const glm::mat4 &projection,
                                                           float fromDist, float toDist);

/**
 * @brief Light matrix (projection * view) of one cascade of a directional light.
 *
 * The orthographic volume is the bounding sphere of @p corners (so its size does not change
 * when the camera turns), extended towards the light to include @p casters, and moved in
 * whole texels of a @p resolution sized map (so shadow edges do not shimmer when the
 * camera moves).
 *
 * @param corners Corners of the frustum slice (frustumSliceCorners()).
 * @param lightDir Direction the light shines in.
 * @param casters Bounds of everything that may cast a shadow into the slice (may be invalid).
 * @param resolution Size of the cascade's tile in texels.
 */
[[nodiscard]] glm::mat4 fitCascade(const std::array<glm::vec3, 8> &corners, const glm::vec3 &lightDir,
                                   const BoundingBox &casters, int resolution);

/**
 * @brief Light matrices of the six cube faces of a point light (+X, -X, +Y, -Y, +Z, -Z).
 *
 * @param tanHalfFov Tangent of half the field of view of a face: 1 is exactly 90 degrees;
 *        slightly more leaves a border for filtering near the face edges.
 */
[[nodiscard]] std::array<glm::mat4, 6> pointShadowMatrices(const glm::vec3 &position, float nearDist, float farDist,
                                                           float tanHalfFov = 1.0f);

/** @brief Index of the cube face (+X, -X, +Y, -Y, +Z, -Z) that direction @p v points into. */
[[nodiscard]] int cubeFace(const glm::vec3 &v);

} // namespace ivf
