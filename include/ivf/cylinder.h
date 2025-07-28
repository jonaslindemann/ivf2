#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
double radius = 1.0,
double size = 1.0,
int slices = 32,
int segments = 8,
int rings = 4,
double start = 0.0,
double sweep = gml::radians(360.0)
*/

namespace ivf {

/**
 * @class Cylinder
 * @brief Node representing a 3D cylinder mesh with configurable parameters.
 *
 * The Cylinder class provides a 3D cylinder geometry. The cylinder's base radius, height (size),
 * number of slices, segments, and angular sweep can be customized. Inherits from MeshNode,
 * allowing transformation and integration into a scene graph.
 */
class Cylinder : public MeshNode {
private:
    double m_radius; ///< Base radius of the cylinder.
    double m_size;   ///< Height of the cylinder.
    int m_slices;    ///< Number of slices around the cylinder.
    int m_segments;  ///< Number of segments along the cylinder's height.
    double m_start;  ///< Start angle (in radians) for the sweep.
    double m_sweep;  ///< Sweep angle (in radians) for the cylinder.

public:
    /**
     * @brief Constructor.
     * @param radius Base radius of the cylinder.
     * @param size Height of the cylinder.
     * @param slices Number of slices around the cylinder.
     * @param segments Number of segments along the cylinder's height.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cylinder.
     */
    Cylinder(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, double start = 0.0,
             double sweep = 2 * glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a Cylinder instance.
     * @param radius Base radius of the cylinder.
     * @param size Height of the cylinder.
     * @param slices Number of slices around the cylinder.
     * @param segments Number of segments along the cylinder's height.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cylinder.
     * @return std::shared_ptr<Cylinder> New Cylinder instance.
     */
    static std::shared_ptr<Cylinder> create(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8,
                                            double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Set the parameters of the cylinder.
     * @param radius Base radius of the cylinder.
     * @param size Height of the cylinder.
     * @param slices Number of slices around the cylinder.
     * @param segments Number of segments along the cylinder's height.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cylinder.
     */
    void set(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, double start = 0.0,
             double sweep = 2 * glm::pi<double>());

protected:
    /**
     * @brief Internal setup method for initializing the cylinder geometry.
     */
    virtual void doSetup();

    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef CylinderPtr
 * @brief Shared pointer type for Cylinder.
 */
typedef std::shared_ptr<Cylinder> CylinderPtr;

} // namespace ivf
