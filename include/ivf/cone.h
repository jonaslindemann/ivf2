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
 * @class Cone
 * @brief Node representing a 3D cone mesh with configurable parameters.
 *
 * The Cone class provides a 3D cone geometry. The cone's base radius, height (size),
 * number of slices, segments, and angular sweep can be customized. Inherits from MeshNode,
 * allowing transformation and integration into a scene graph.
 */
class Cone : public MeshNode {
private:
    double m_radius; ///< Base radius of the cone.
    double m_size;   ///< Height of the cone.
    int m_slices;    ///< Number of slices around the cone.
    int m_segments;  ///< Number of segments along the cone's height.
    double m_start;  ///< Start angle (in radians) for the sweep.
    double m_sweep;  ///< Sweep angle (in radians) for the cone.

public:
    /**
     * @brief Constructor.
     * @param radius Base radius of the cone.
     * @param size Height of the cone.
     * @param slices Number of slices around the cone.
     * @param segments Number of segments along the cone's height.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cone.
     */
    Cone(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, double start = 0.0,
         double sweep = 2 * glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a Cone instance.
     * @param radius Base radius of the cone.
     * @param size Height of the cone.
     * @param slices Number of slices around the cone.
     * @param segments Number of segments along the cone's height.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cone.
     * @return std::shared_ptr<Cone> New Cone instance.
     */
    static std::shared_ptr<Cone> create(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8,
                                        double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Set the parameters of the cone.
     * @param radius Base radius of the cone.
     * @param size Height of the cone.
     * @param slices Number of slices around the cone.
     * @param segments Number of segments along the cone's height.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cone.
     */
    void set(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, double start = 0.0,
             double sweep = 2 * glm::pi<double>());

protected:
    /**
     * @brief Internal setup method for initializing the cone geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef ConePtr
 * @brief Shared pointer type for Cone.
 */
typedef std::shared_ptr<Cone> ConePtr;

} // namespace ivf
