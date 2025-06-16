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
 * @class Disk
 * @brief Node representing a 3D disk (circular or annular) mesh with configurable parameters.
 *
 * The Disk class provides a 3D disk geometry, which can be a filled circle or an annulus (ring).
 * The disk's outer radius, inner radius, number of slices, rings, and angular sweep can be customized.
 * Inherits from MeshNode, allowing transformation and integration into a scene graph.
 */
class Disk : public MeshNode {
private:
    double m_radius;      ///< Outer radius of the disk.
    double m_innerRadius; ///< Inner radius of the disk (0 for a filled disk).
    int m_slices;         ///< Number of slices around the disk.
    int m_rings;          ///< Number of concentric rings.
    double m_start;       ///< Start angle (in radians) for the sweep.
    double m_sweep;       ///< Sweep angle (in radians) for the disk.

public:
    /**
     * @brief Constructor.
     * @param radius Outer radius of the disk.
     * @param innerRadius Inner radius of the disk (0 for a filled disk).
     * @param slices Number of slices around the disk.
     * @param rings Number of concentric rings.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the disk.
     */
    Disk(double radius = 1.0, double innerRadius = 0.0, int slices = 32, int rings = 4, double start = 0.0,
         double sweep = 2 * glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a Disk instance.
     * @param radius Outer radius of the disk.
     * @param innerRadius Inner radius of the disk (0 for a filled disk).
     * @param slices Number of slices around the disk.
     * @param rings Number of concentric rings.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the disk.
     * @return std::shared_ptr<Disk> New Disk instance.
     */
    static std::shared_ptr<Disk> create(double radius = 1.0, double innerRadius = 0.0, int slices = 32, int rings = 4,
                                        double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Set the parameters of the disk.
     * @param radius Outer radius of the disk.
     * @param innerRadius Inner radius of the disk (0 for a filled disk).
     * @param slices Number of slices around the disk.
     * @param rings Number of concentric rings.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the disk.
     */
    void set(double radius = 1.0, double innerRadius = 0.0, int slices = 32, int rings = 4, double start = 0.0,
             double sweep = 2 * glm::pi<double>());

protected:
    /**
     * @brief Internal setup method for initializing the disk geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef DiskPtr
 * @brief Shared pointer type for Disk.
 */
typedef std::shared_ptr<Disk> DiskPtr;

} // namespace ivf
