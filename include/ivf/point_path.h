#pragma once

#include <ivf/mesh_node.h>
#include <ivf/utils.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

#include <generator/ParametricPath.hpp>

namespace ivf {

/**
 * @class PointPath
 * @brief Utility class for representing a parametric path defined by control points.
 *
 * The PointPath class wraps a generator::ParametricPath and provides access to its
 * edges and vertices. It is useful for creating and manipulating paths defined by
 * a set of control points, supporting parametric evaluation and traversal.
 */
class PointPath {
private:
    int m_segments; ///< Number of segments in the path.

    using Impl = generator::ParametricPath;
    Impl parametricPath_; ///< Underlying parametric path implementation.

public:
    /**
     * @brief Constructor.
     * @param segments Number of segments in the path.
     * @param points Control points defining the path.
     */
    PointPath(int segments, std::vector<glm::vec3> points);

    /**
     * @brief Type alias for the edges generator.
     */
    using Edges = typename Impl::Edges;

    /**
     * @brief Get an edge generator for the path.
     * @return Edges Generator for path edges.
     */
    Edges edges() const noexcept
    {
        return parametricPath_.edges();
    }

    /**
     * @brief Type alias for the vertices generator.
     */
    using Vertices = typename Impl::Vertices;

    /**
     * @brief Get a vertex generator for the path.
     * @return Vertices Generator for path vertices.
     */
    Vertices vertices() const noexcept
    {
        return parametricPath_.vertices();
    }
};

} // namespace ivf
