#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace ivf {

/**
 * @class Extrusion
 * @brief Node representing a mesh generated by extruding a 2D contour along a 3D path.
 *
 * The Extrusion class allows the creation of complex 3D shapes by sweeping a 2D section (contour)
 * along a 3D path. The path and section can be defined point-by-point, and the class provides
 * methods for generating circular sections and clearing or modifying the path and section.
 * Inherits from MeshNode, allowing transformation and integration into a scene graph.
 *
 * Based on code by Song Ho Ahn: http://www.songho.ca/opengl/gl_cylinder.html#example_pipe
 */
class Extrusion : public MeshNode {
private:
    std::vector<glm::vec3> m_path;                  ///< List of points defining the extrusion path.
    std::vector<glm::vec3> m_contour;               ///< List of points defining the section contour.
    std::vector<std::vector<glm::vec3>> m_contours; ///< Generated contours along the path.
    std::vector<std::vector<glm::vec3>> m_normals;  ///< Normals for each contour.

    /**
     * @brief Generate contours along the path based on the section.
     */
    void generateContours();

    /**
     * @brief Transform the first contour to align with the path.
     */
    void transformFirstContour();

    /**
     * @brief Project a contour from one path index to another.
     * @param fromIndex Index of the starting path point.
     * @param toIndex Index of the target path point.
     * @return std::vector<glm::vec3> Projected contour.
     */
    std::vector<glm::vec3> projectContour(int fromIndex, int toIndex);

    /**
     * @brief Compute normals for a contour at a given path index.
     * @param pathIndex Index of the path point.
     * @return std::vector<glm::vec3> Normals for the contour.
     */
    std::vector<glm::vec3> computeContourNormal(int pathIndex);

    /**
     * @brief Intersect a ray with a plane.
     * @param pos Ray origin.
     * @param dir Ray direction.
     * @param planePos Point on the plane.
     * @param planeDir Plane normal or direction.
     * @return glm::vec3 Intersection point.
     */
    glm::vec3 intersectPlane(glm::vec3 &pos, glm::vec3 &dir, glm::vec3 &planePos, glm::vec3 &planeDir);

public:
    /**
     * @brief Default constructor.
     */
    Extrusion();

    /**
     * @brief Factory method to create a shared pointer to an Extrusion instance.
     * @return std::shared_ptr<Extrusion> New Extrusion instance.
     */
    static std::shared_ptr<Extrusion> create();

    /**
     * @brief Add a point to the extrusion path.
     * @param p 3D point to add to the path.
     */
    void addPathPoint(glm::vec3 p);

    /**
     * @brief Clear all points from the extrusion path.
     */
    void clearPath();

    /**
     * @brief Add a point to the section (contour) in 2D.
     * @param p 2D point to add to the section.
     */
    void addSectionPoint(glm::vec2 p);

    /**
     * @brief Clear all points from the section (contour).
     */
    void clearSection();

    /**
     * @brief Create a circular section (contour) with the given radius and number of segments.
     * @param r Radius of the circle.
     * @param segments Number of segments (default is 36).
     */
    void createCircleSection(float r, int segments = 36);

protected:
    /**
     * @brief Internal setup method for initializing the extrusion geometry.
     */
    virtual void doSetup();

    /**
     * @brief Called before drawing the extrusion node.
     */
    virtual void doPreDraw();

    /**
     * @brief Called after drawing the extrusion node.
     */
    virtual void doPostDraw();
};

/**
 * @typedef ExtrusionPtr
 * @brief Shared pointer type for Extrusion.
 */
typedef std::shared_ptr<Extrusion> ExtrusionPtr;

}; // namespace ivf

//
// Based on code by Song ho Ahn
//
// http://www.songho.ca/opengl/gl_cylinder.html#example_pipe
//
