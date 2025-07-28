#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

/**
 * @class Sphere
 * @brief MeshNode representing a UV sphere with configurable slices, segments, and partial sweeps.
 *
 * The Sphere class generates a UV sphere mesh with configurable radius, slice/segment counts,
 * and angular sweeps for partial spheres. Inherits from MeshNode, allowing transformation
 * and integration into a scene graph.
 */
class Sphere : public MeshNode {
private:
    double m_radius;       ///< Sphere radius.
    int m_slices;          ///< Number of slices (longitude divisions).
    int m_segments;        ///< Number of segments (latitude divisions).
    double m_sliceStart;   ///< Starting angle for slices (in radians).
    double m_sliceSweep;   ///< Sweep angle for slices (in radians).
    double m_segmentStart; ///< Starting angle for segments (in radians).
    double m_segmentSweep; ///< Sweep angle for segments (in radians).

public:
    /**
     * @brief Constructor.
     * @param radius Sphere radius.
     * @param slices Number of slices (longitude divisions).
     * @param segments Number of segments (latitude divisions).
     * @param sliceStart Starting angle for slices (radians).
     * @param sliceSweep Sweep angle for slices (radians).
     * @param segmentStart Starting angle for segments (radians).
     * @param segmentSweep Sweep angle for segments (radians).
     */
    Sphere(double radius = 1.0, int slices = 32, int segments = 16, double sliceStart = 0.0,
           double sliceSweep = 2 * glm::pi<double>(), double segmentStart = 0.0,
           double segmentSweep = glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a Sphere instance.
     * @param radius Sphere radius.
     * @param slices Number of slices (longitude divisions).
     * @param segments Number of segments (latitude divisions).
     * @param sliceStart Starting angle for slices (radians).
     * @param sliceSweep Sweep angle for slices (radians).
     * @param segmentStart Starting angle for segments (radians).
     * @param segmentSweep Sweep angle for segments (radians).
     * @return std::shared_ptr<Sphere> New Sphere instance.
     */
    static std::shared_ptr<Sphere> create(double radius = 1.0, int slices = 32, int segments = 16,
                                          double sliceStart = 0.0, double sliceSweep = 2 * glm::pi<double>(),
                                          double segmentStart = 0.0, double segmentSweep = glm::pi<double>());

    /**
     * @brief Set all sphere parameters.
     * @param radius Sphere radius.
     * @param slices Number of slices (longitude divisions).
     * @param segments Number of segments (latitude divisions).
     * @param sliceStart Starting angle for slices (radians).
     * @param sliceSweep Sweep angle for slices (radians).
     * @param segmentStart Starting angle for segments (radians).
     * @param segmentSweep Sweep angle for segments (radians).
     */
    void set(double radius = 1.0, int slices = 32, int segments = 16, double sliceStart = 0.0,
             double sliceSweep = 2 * glm::pi<double>(), double segmentStart = 0.0,
             double segmentSweep = glm::pi<double>());

    /**
     * @brief Set the sphere radius.
     * @param radius New radius.
     */
    void setRadius(double radius);

    /**
     * @brief Set the number of slices (longitude divisions).
     * @param slices Number of slices.
     */
    void setSlices(int slices);

    /**
     * @brief Set the number of segments (latitude divisions).
     * @param segments Number of segments.
     */
    void setSegments(int segments);

    /**
     * @brief Set the starting angle for slices (longitude, radians).
     * @param sliceStart Starting angle.
     */
    void setSliceStart(double sliceStart);

    /**
     * @brief Set the sweep angle for slices (longitude, radians).
     * @param sliceSweep Sweep angle.
     */
    void setSliceSweep(double sliceSweep);

    /**
     * @brief Set the starting angle for segments (latitude, radians).
     * @param segmentStart Starting angle.
     */
    void setSegmentStart(double segmentStart);

    /**
     * @brief Set the sweep angle for segments (latitude, radians).
     * @param segmentSweep Sweep angle.
     */
    void setSegmentSweep(double segmentSweep);

protected:
    /**
     * @brief Internal setup method for initializing the sphere geometry.
     */
    virtual void doSetup();

    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef SpherePtr
 * @brief Shared pointer type for Sphere.
 */
typedef std::shared_ptr<Sphere> SpherePtr;

} // namespace ivf
