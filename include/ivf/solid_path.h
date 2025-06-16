#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

#include <vector>

namespace ivf {

/**
 * @class SolidPath
 * @brief MeshNode representing a solid 3D path (tube) through a sequence of points.
 *
 * The SolidPath class generates a tubular mesh along a sequence of 3D points,
 * with configurable radius and segment count. It is useful for visualizing paths,
 * splines, or polylines in 3D space. Inherits from MeshNode, allowing transformation
 * and integration into a scene graph.
 */
class SolidPath : public MeshNode {
private:
    std::vector<glm::vec3> m_points; ///< List of points defining the path.
    double m_radius;                 ///< Radius of the tube.
    int m_segments;                  ///< Number of segments around the tube.

public:
    /**
     * @brief Constructor.
     * @param radius Tube radius.
     * @param segments Number of segments around the tube.
     */
    SolidPath(double radius = 0.1, int segments = 64);

    /**
     * @brief Factory method to create a shared pointer to a SolidPath instance.
     * @param radius Tube radius.
     * @param segments Number of segments around the tube.
     * @return std::shared_ptr<SolidPath> New SolidPath instance.
     */
    static std::shared_ptr<SolidPath> create(double radius = 0.1, int segments = 64);

    /**
     * @brief Add a point to the path.
     * @param p Point to add.
     */
    void addPoint(glm::vec3 p);

    /**
     * @brief Remove all points from the path.
     */
    void clear();

    /**
     * @brief Set the tube radius.
     * @param radius New tube radius.
     */
    void setRadius(double radius);

protected:
    /**
     * @brief Internal setup method for initializing the solid path geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef SolidPathPtr
 * @brief Shared pointer type for SolidPath.
 */
typedef std::shared_ptr<SolidPath> SolidPathPtr;

} // namespace ivf
