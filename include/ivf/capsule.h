#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
        double radius = 1.0,
        double size = 0.5,
        int slices = 32,
        int segments = 4,
        int rings = 8,
        double start = 0.0,
        double sweep = gml::radians(360.0)
*/

namespace ivf {

/**
 * @class Capsule
 * @brief Node representing a 3D capsule mesh with configurable parameters.
 *
 * The Capsule class provides a 3D capsule geometry, which consists of a cylinder capped with hemispheres
 * at both ends. The capsule's radius, cylinder height (size), number of slices, segments, rings, and angular
 * sweep can be customized. Inherits from MeshNode, allowing transformation and integration into a scene graph.
 */
class Capsule : public MeshNode {
private:
    double m_radius; ///< Radius of the capsule (cylinder and hemispheres).
    double m_size;   ///< Height of the cylindrical part of the capsule.
    int m_slices;    ///< Number of slices around the capsule.
    int m_segments;  ///< Number of segments along the capsule's height.
    int m_rings;     ///< Number of rings for the hemispherical caps.
    double m_start;  ///< Start angle (in radians) for the sweep.
    double m_sweep;  ///< Sweep angle (in radians) for the capsule.

public:
    /**
     * @brief Constructor.
     * @param radius Radius of the capsule.
     * @param size Height of the cylindrical part.
     * @param slices Number of slices around the capsule.
     * @param segments Number of segments along the capsule's height.
     * @param rings Number of rings for the hemispherical caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the capsule.
     */
    Capsule(double radius = 1.0, double size = 0.5, int slices = 32, int segments = 4, int rings = 8,
            double start = 0.0, double sweep = gml::radians(360.0));

    /**
     * @brief Factory method to create a shared pointer to a Capsule instance.
     * @param radius Radius of the capsule.
     * @param size Height of the cylindrical part.
     * @param slices Number of slices around the capsule.
     * @param segments Number of segments along the capsule's height.
     * @param rings Number of rings for the hemispherical caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the capsule.
     * @return std::shared_ptr<Capsule> New Capsule instance.
     */
    static std::shared_ptr<Capsule> create(double radius = 1.0, double size = 0.5, int slices = 32, int segments = 4,
                                           int rings = 8, double start = 0.0, double sweep = gml::radians(360.0));

    /**
     * @brief Set the parameters of the capsule.
     * @param radius Radius of the capsule.
     * @param size Height of the cylindrical part.
     * @param slices Number of slices around the capsule.
     * @param segments Number of segments along the capsule's height.
     * @param rings Number of rings for the hemispherical caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the capsule.
     */
    void set(double radius = 1.0, double size = 0.5, int slices = 32, int segments = 4, int rings = 8,
             double start = 0.0, double sweep = gml::radians(360.0));

protected:
    /**
     * @brief Internal setup method for initializing the capsule geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef CapsulePtr
 * @brief Shared pointer type for Capsule.
 */
typedef std::shared_ptr<Capsule> CapsulePtr;

} // namespace ivf
