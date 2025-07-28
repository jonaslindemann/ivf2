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
 * @class CappedCylinder
 * @brief Node representing a 3D capped cylinder mesh with configurable parameters.
 *
 * The CappedCylinder class provides a 3D cylinder geometry with caps on both ends. The cylinder's radius, height
 * (size), number of slices, segments, rings, and angular sweep can be customized. Inherits from MeshNode, allowing
 * transformation and integration into a scene graph.
 */
class CappedCylinder : public MeshNode {
private:
    double m_radius; ///< Radius of the cylinder.
    double m_size;   ///< Height of the cylinder.
    int m_slices;    ///< Number of slices around the cylinder.
    int m_segments;  ///< Number of segments along the cylinder's height.
    int m_rings;     ///< Number of rings for the caps.
    double m_start;  ///< Start angle (in radians) for the sweep.
    double m_sweep;  ///< Sweep angle (in radians) for the cylinder.

public:
    /**
     * @brief Constructor.
     * @param radius Radius of the cylinder.
     * @param size Height of the cylinder.
     * @param slices Number of slices around the cylinder.
     * @param segments Number of segments along the cylinder's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cylinder.
     */
    CappedCylinder(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4,
                   double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a CappedCylinder instance.
     * @param radius Radius of the cylinder.
     * @param size Height of the cylinder.
     * @param slices Number of slices around the cylinder.
     * @param segments Number of segments along the cylinder's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cylinder.
     * @return std::shared_ptr<CappedCylinder> New CappedCylinder instance.
     */
    static std::shared_ptr<CappedCylinder> create(double radius = 1.0, double size = 1.0, int slices = 32,
                                                  int segments = 8, int rings = 4, double start = 0.0,
                                                  double sweep = 2 * glm::pi<double>());

    /**
     * @brief Set the parameters of the capped cylinder.
     * @param radius Radius of the cylinder.
     * @param size Height of the cylinder.
     * @param slices Number of slices around the cylinder.
     * @param segments Number of segments along the cylinder's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cylinder.
     */
    void set(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4,
             double start = 0.0, double sweep = 2 * glm::pi<double>());

protected:
    /**
     * @brief Internal setup method for initializing the capped cylinder geometry.
     */
    virtual void doSetup();

    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef CappedCylinderPtr
 * @brief Shared pointer type for CappedCylinder.
 */
typedef std::shared_ptr<CappedCylinder> CappedCylinderPtr;

} // namespace ivf
