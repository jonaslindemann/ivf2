#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
        double radius = 1.0,
        double innerRadius = 0.0,
        int slices = 32,
        int rings = 4,
        double start = 0.0,
        double sweep = gml::radians(360.0)
*/

namespace ivf {

/**
 * @class Plane
 * @brief MeshNode representing a subdivided rectangular plane in 3D space.
 *
 * The Plane class generates a rectangular mesh subdivided into rows and columns.
 * It is useful for grids, ground planes, or as a base for further geometry.
 * Inherits from MeshNode, allowing transformation and integration into a scene graph.
 */
class Plane : public MeshNode {
private:
    double m_width; ///< Width of the plane.
    double m_depth; ///< Depth of the plane.
    int m_rows;     ///< Number of subdivisions along the width.
    int m_cols;     ///< Number of subdivisions along the depth.

public:
    /**
     * @brief Constructor.
     * @param width Width of the plane.
     * @param depth Depth of the plane.
     * @param rows Number of subdivisions along the width.
     * @param cols Number of subdivisions along the depth.
     */
    Plane(double width = 1.0, double depth = 1.0, int rows = 8, int cols = 8);

    /**
     * @brief Factory method to create a shared pointer to a Plane instance.
     * @param width Width of the plane.
     * @param depth Depth of the plane.
     * @param rows Number of subdivisions along the width.
     * @param cols Number of subdivisions along the depth.
     * @return std::shared_ptr<Plane> New Plane instance.
     */
    static std::shared_ptr<Plane> create(double width = 1.0, double depth = 1.0, int rows = 8, int cols = 8);

    /**
     * @brief Set the plane parameters.
     * @param width Width of the plane.
     * @param depth Depth of the plane.
     * @param rows Number of subdivisions along the width.
     * @param cols Number of subdivisions along the depth.
     */
    void set(double width = 1.0, double depth = 1.0, int rows = 8, int cols = 8);

protected:
    /**
     * @brief Internal setup method for initializing the plane geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef PlanePtr
 * @brief Shared pointer type for Plane.
 */
typedef std::shared_ptr<Plane> PlanePtr;

} // namespace ivf
