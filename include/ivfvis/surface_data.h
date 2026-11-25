#pragma once

/**
 * @file surface_data.h
 * @brief Triangle surfaces extracted from data: isosurfaces, slices and their contour lines.
 */

#include <ivfvis/uniform_grid.h>

#include <glm/glm.hpp>

#include <span>
#include <string_view>
#include <vector>

namespace ivfvis {

/**
 * @brief An indexed triangle surface with a value per vertex (CPU data, no OpenGL).
 */
struct SurfaceData {
    std::vector<glm::vec3> positions; ///< Vertex positions.
    std::vector<glm::vec3> normals;   ///< Unit vertex normals.
    std::vector<float> values;        ///< A value per vertex (for colouring); may be empty.
    std::vector<glm::uvec3> indices;  ///< Triangles (counter-clockwise seen from the normals' side).

    [[nodiscard]] std::size_t triangleCount() const
    {
        return indices.size();
    }

    /** @brief Total area of the triangles. */
    [[nodiscard]] float area() const;
};

/**
 * @brief The isosurface where scalar array @p scalars of @p grid equals @p isoValue
 * (marching cubes).
 *
 * Vertices are shared between neighbouring cells, so the surface is a connected mesh;
 * inside the grid it is closed. Normals come from the field's gradient and point towards
 * lower values. Ambiguous cell faces are split the same way in both cells that share them,
 * so the surface has no cracks.
 *
 * @param colorBy Scalar array interpolated to the vertices into SurfaceData::values (empty:
 *        the values are the iso value).
 */
[[nodiscard]] SurfaceData extractIsosurface(const UniformGrid &grid, std::string_view scalars, float isoValue,
                                            std::string_view colorBy = {});

/**
 * @brief The part of the plane through @p origin with normal @p normal that lies inside the
 * grid, as triangles with the values of scalar array @p scalars.
 *
 * The plane is covered by a grid of about @p resolution by @p resolution squares (over the
 * slice's longer side), clipped to the grid's box.
 */
[[nodiscard]] SurfaceData extractSlice(const UniformGrid &grid, std::string_view scalars, const glm::vec3 &origin,
                                       const glm::vec3 &normal, int resolution = 128);

/**
 * @brief Contour lines of the vertex values of @p surface at each of @p levels, as line
 * segments (pairs of points).
 */
[[nodiscard]] std::vector<glm::vec3> contourLines(const SurfaceData &surface, std::span<const float> levels);

} // namespace ivfvis
