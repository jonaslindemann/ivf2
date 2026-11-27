#pragma once

// Private to ivfphysics: convex hulls with planar polygon faces.

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace ivfphysics::detail {

/** A convex polyhedron: corner vertices and planar faces (counter-clockwise seen from outside). */
struct Polyhedron {
    std::vector<glm::vec3> vertices;
    std::vector<std::vector<std::uint32_t>> faces;
};

/**
 * Convex hull of @p points (incremental, in double precision).
 *
 * Points closer than a small tolerance (relative to the extent) to the hull are treated as
 * inside, coplanar triangles are merged into polygons and points on edges are dropped, so
 * the result has only true corners: what the engine's polyhedron collision needs. Inputs
 * of more than @p maxInputPoints points are first reduced to their extreme points in that
 * many evenly spread directions (a close approximation).
 *
 * @return False with @p error set if the points are too few or all in one plane.
 */
bool convexHull(const std::vector<glm::vec3> &points, Polyhedron &hull, std::string &error,
                std::size_t maxInputPoints = 2048);

} // namespace ivfphysics::detail
