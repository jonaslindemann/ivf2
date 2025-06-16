#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

#include <generator/PolyLinePath.hpp>

namespace ivf {

/**
 * @class SolidPolyLine
 * @brief MeshNode representing a solid 3D polyline (tube) through a sequence of points.
 *
 * The SolidPolyLine class generates a tubular mesh along a polyline defined by a sequence
 * of 3D points. The tube's radius and segment count are configurable. This is useful for
 * visualizing polylines, paths, or splines in 3D space. Inherits from MeshNode, allowing
 * transformation and integration into a scene graph.
 */
class SolidPolyLine : public MeshNode {
private:
    double m_radius;                    ///< Tube radius.
    generator::PolyLinePath m_polyPath; ///< Polyline path generator.
    int m_segments;                     ///< Number of segments around the tube.

public:
    /**
     * @brief Constructor.
     * @param radius Tube radius.
     * @param segments Number of segments around the tube.
     */
    SolidPolyLine(double radius = 0.1, int segments = 8);

    /**
     * @brief Factory method to create a shared pointer to a SolidPolyLine instance.
     * @param radius Tube radius.
     * @param segments Number of segments around the tube.
     * @return std::shared_ptr<SolidPolyLine> New SolidPolyLine instance.
     */
    static std::shared_ptr<SolidPolyLine> create(double radius = 0.1, int segments = 8);

    /**
     * @brief Set the tube radius.
     * @param radius New tube radius.
     */
    void setRadius(double radius);

    /**
     * @brief Add a point to the polyline.
     * @param point Point to add.
     */
    void addPoint(const gml::dvec3 &point);

    /**
     * @brief Add a point to the polyline by coordinates.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     */
    void addPoint(double x, double y, double z);

    /**
     * @brief Set all points of the polyline.
     * @param points Vector of points.
     */
    void setPoints(const std::vector<gml::dvec3> &points);

    /**
     * @brief Remove all points from the polyline.
     */
    void clearPoints();

    /**
     * @brief Set the number of segments around the tube.
     * @param segments Segment count.
     */
    void setSegments(int segments);

    /**
     * @brief Set a specific point in the polyline.
     * @param index Index of the point.
     * @param point New point value.
     */
    void setPoint(int index, const gml::dvec3 &point);

    /**
     * @brief Get a specific point from the polyline.
     * @param index Index of the point.
     * @return gml::dvec3 Point value.
     */
    gml::dvec3 point(int index) const;

    /**
     * @brief Get the number of points in the polyline.
     * @return int Point count.
     */
    int pointCount();

protected:
    /**
     * @brief Internal setup method for initializing the solid polyline geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef SolidPolyLinePtr
 * @brief Shared pointer type for SolidPolyLine.
 */
typedef std::shared_ptr<SolidPolyLine> SolidPolyLinePtr;

} // namespace ivf
