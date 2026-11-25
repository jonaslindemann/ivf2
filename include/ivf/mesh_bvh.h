#pragma once

/**
 * @file mesh_bvh.h
 * @brief Bounding volume hierarchy over a mesh's triangles, for fast ray casts.
 */

#include <ivf/bounding_box.h>
#include <ivf/ray.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace ivf {

/**
 * @class MeshBvh
 * @brief Triangles sorted into a tree of boxes, so a ray only tests the triangles near it.
 *
 * Mesh::bvh() builds one for a mesh when it is first needed (and again after its vertices
 * changed); pick() uses it. Building takes O(n log n), a ray cast O(log n) for typical meshes.
 */
class MeshBvh {
public:
    /// A ray hit: ray parameter, triangle and barycentric coordinates.
    struct Hit {
        float t{0.0f};
        std::uint32_t triangle{0};
        float u{0.0f}; ///< Weight of the triangle's second vertex.
        float v{0.0f}; ///< Weight of the third vertex.
    };

    MeshBvh() = default;

    /** @brief Build the tree for @p triangles (indices into @p positions). */
    MeshBvh(std::vector<glm::vec3> positions, std::vector<glm::uvec3> triangles);

    /** @brief The nearest hit with t in (0, @p maxT), if any. */
    [[nodiscard]] std::optional<Hit> intersect(const Ray &ray,
                                               float maxT = std::numeric_limits<float>::infinity()) const;

    /** @brief True if anything is hit with t in (0, @p maxT) (stops at the first hit). */
    [[nodiscard]] bool occluded(const Ray &ray, float maxT = std::numeric_limits<float>::infinity()) const;

    [[nodiscard]] std::size_t triangleCount() const
    {
        return m_triangles.size();
    }
    [[nodiscard]] std::size_t nodeCount() const
    {
        return m_nodes.size();
    }
    /** @brief Depth of the tree (1 for a single leaf, 0 if empty). */
    [[nodiscard]] int depth() const
    {
        return m_depth;
    }
    [[nodiscard]] BoundingBox bounds() const;

    /** @brief Vertex indices of triangle @p index (as given to the constructor). */
    [[nodiscard]] glm::uvec3 triangle(std::uint32_t index) const
    {
        return m_triangles[index];
    }
    /** @brief Position of vertex @p index. */
    [[nodiscard]] const glm::vec3 &position(std::uint32_t index) const
    {
        return m_positions[index];
    }
    /** @brief Geometric (unnormalised) normal of triangle @p index, by its winding. */
    [[nodiscard]] glm::vec3 faceNormal(std::uint32_t index) const;

private:
    struct Node {
        glm::vec3 min;
        std::uint32_t offset; ///< Leaf: first entry of m_order; inner: left child (the right one follows it).
        glm::vec3 max;
        std::uint32_t count; ///< Triangles in a leaf, 0 for inner nodes.
    };

    std::vector<glm::vec3> m_positions;
    std::vector<glm::uvec3> m_triangles;
    std::vector<std::uint32_t> m_order; ///< Triangle indices, grouped by leaf.
    std::vector<Node> m_nodes;
    int m_depth{0};

    template <typename Visit> void traverse(const Ray &ray, float &maxT, Visit &&visit) const;
};

using MeshBvhPtr = std::shared_ptr<const MeshBvh>;

} // namespace ivf
