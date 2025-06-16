#pragma once

/**
 * @file tube.h
 * @brief Declares the Tube class for generating and rendering tubular mesh geometry in the ivf library.
 */

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

/**
 * @class Tube
 * @brief Scene node representing a 3D tube or hollow cylinder mesh.
 *
 * The Tube class generates and manages a mesh representing a tube (hollow cylinder) with configurable
 * radius, inner radius, height (size), number of slices, segments, and sweep angle. It inherits from MeshNode,
 * allowing it to be used as a drawable and transformable scene node.
 */
class Tube : public MeshNode {
private:
    double m_radius;      ///< Outer radius of the tube.
    double m_innerRadius; ///< Inner radius of the tube.
    double m_size;        ///< Height or length of the tube.
    int m_slices;         ///< Number of slices (angular subdivisions).
    int m_segments;       ///< Number of segments along the tube.
    int m_rings;          ///< Number of rings (not exposed in constructor).
    double m_start;       ///< Start angle (in radians).
    double m_sweep;       ///< Sweep angle (in radians).

public:
    /**
     * @brief Construct a Tube with the given parameters.
     * @param radius Outer radius of the tube (default 1.0).
     * @param innerRadius Inner radius of the tube (default 0.75).
     * @param size Height or length of the tube (default 1.0).
     * @param slices Number of angular slices (default 32).
     * @param segments Number of segments along the tube (default 8).
     * @param start Start angle in radians (default 0.0).
     * @param sweep Sweep angle in radians (default 2*glm::pi<double>()).
     */
    Tube(double radius = 1.0, double innerRadius = 0.75, double size = 1.0, int slices = 32, int segments = 8,
         double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a Tube instance.
     * @param radius Outer radius of the tube (default 1.0).
     * @param innerRadius Inner radius of the tube (default 0.75).
     * @param size Height or length of the tube (default 1.0).
     * @param slices Number of angular slices (default 32).
     * @param segments Number of segments along the tube (default 8).
     * @param start Start angle in radians (default 0.0).
     * @param sweep Sweep angle in radians (default 2*glm::pi<double>()).
     * @return std::shared_ptr<Tube> New Tube instance.
     */
    static std::shared_ptr<Tube> create(double radius = 1.0, double innerRadius = 0.75, double size = 1.0,
                                        int slices = 32, int segments = 8, double start = 0.0,
                                        double sweep = 2 * glm::pi<double>());

    /**
     * @brief Set the tube parameters and update the mesh.
     * @param radius Outer radius of the tube (default 1.0).
     * @param innerRadius Inner radius of the tube (default 0.75).
     * @param size Height or length of the tube (default 1.0).
     * @param slices Number of angular slices (default 32).
     * @param segments Number of segments along the tube (default 8).
     * @param start Start angle in radians (default 0.0).
     * @param sweep Sweep angle in radians (default 2*glm::pi<double>()).
     */
    void set(double radius = 1.0, double innerRadius = 0.75, double size = 1.0, int slices = 32, int segments = 8,
             double start = 0.0, double sweep = 2 * glm::pi<double>());

protected:
    /**
     * @brief Internal setup method for initializing the tube mesh.
     */
    virtual void doSetup();
};

/**
 * @typedef TubePtr
 * @brief Shared pointer type for Tube.
 */
typedef std::shared_ptr<Tube> TubePtr;

} // namespace ivf
