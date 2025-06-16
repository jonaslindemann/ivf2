#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

/**
 * @class SolidLine
 * @brief MeshNode representing a solid 3D line (cylinder) between two points.
 *
 * The SolidLine class generates a cylindrical mesh between two points in 3D space,
 * with a configurable radius. It is useful for visualizing connections, axes, or
 * geometric edges. Inherits from MeshNode, allowing transformation and integration
 * into a scene graph.
 */
class SolidLine : public MeshNode {
private:
    glm::vec3 m_p0;  ///< Start point of the line.
    glm::vec3 m_p1;  ///< End point of the line.
    double m_radius; ///< Radius of the solid line (cylinder).

public:
    /**
     * @brief Constructor.
     * @param p0 Start point of the line.
     * @param p1 End point of the line.
     * @param radius Radius of the solid line.
     */
    SolidLine(glm::vec3 p0, glm::vec3 p1, double radius = 0.1);

    /**
     * @brief Factory method to create a shared pointer to a SolidLine instance.
     * @param p0 Start point of the line.
     * @param p1 End point of the line.
     * @param radius Radius of the solid line.
     * @return std::shared_ptr<SolidLine> New SolidLine instance.
     */
    static std::shared_ptr<SolidLine> create(glm::vec3 p0, glm::vec3 p1, double radius = 0.1);

    /**
     * @brief Set the start and end points and radius of the line.
     * @param p0 Start point.
     * @param p1 End point.
     * @param radius Radius of the solid line.
     */
    void set(glm::vec3 p0, glm::vec3 p1, double radius = 0.1);

    /**
     * @brief Set the radius of the solid line.
     * @param radius New radius.
     */
    void setRadius(double radius);

    /**
     * @brief Set the start point of the line.
     * @param p New start point.
     */
    void setP0(glm::vec3 p);

    /**
     * @brief Set the end point of the line.
     * @param p New end point.
     */
    void setP1(glm::vec3 p);

    /**
     * @brief Set both start and end points of the line.
     * @param p0 Start point.
     * @param p1 End point.
     */
    void setPoints(glm::vec3 p0, glm::vec3 p1);

protected:
    /**
     * @brief Internal setup method for initializing the solid line geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef SolidLinePtr
 * @brief Shared pointer type for SolidLine.
 */
typedef std::shared_ptr<SolidLine> SolidLinePtr;

} // namespace ivf
