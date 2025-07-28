#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
        double radius = 1.0,
        double innerRadius = 0.75,
        double size = 1.0,
        int slices = 32,
        int segments = 8,
        int rings = 1,
        double start = 0.0,
        double sweep = gml::radians(360.0)
*/

namespace ivf {

/**
 * @class CappedTube
 * @brief Node representing a 3D capped tube (cylinder with inner radius) mesh with configurable parameters.
 *
 * The CappedTube class provides a 3D tube geometry with caps on both ends. The tube's outer radius, inner radius,
 * height (size), number of slices, segments, rings, and angular sweep can be customized. Inherits from MeshNode,
 * allowing transformation and integration into a scene graph.
 */
class CappedTube : public MeshNode {
private:
    double m_radius;      ///< Outer radius of the tube.
    double m_innerRadius; ///< Inner radius of the tube (hole radius).
    double m_size;        ///< Height of the tube.
    int m_slices;         ///< Number of slices around the tube.
    int m_segments;       ///< Number of segments along the tube's height.
    int m_rings;          ///< Number of rings for the caps.
    double m_start;       ///< Start angle (in radians) for the sweep.
    double m_sweep;       ///< Sweep angle (in radians) for the tube.

public:
    /**
     * @brief Constructor.
     * @param radius Outer radius of the tube.
     * @param innerRadius Inner radius of the tube.
     * @param size Height of the tube.
     * @param slices Number of slices around the tube.
     * @param segments Number of segments along the tube's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the tube.
     */
    CappedTube(double radius = 1.0, double innerRadius = 0.75, double size = 1.0, int slices = 32, int segments = 8,
               int rings = 1, double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a CappedTube instance.
     * @param radius Outer radius of the tube.
     * @param innerRadius Inner radius of the tube.
     * @param size Height of the tube.
     * @param slices Number of slices around the tube.
     * @param segments Number of segments along the tube's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the tube.
     * @return std::shared_ptr<CappedTube> New CappedTube instance.
     */
    static std::shared_ptr<CappedTube> create(double radius = 1.0, double innerRadius = 0.75, double size = 1.0,
                                              int slices = 32, int segments = 8, int rings = 1, double start = 0.0,
                                              double sweep = 2 * glm::pi<double>());

    /**
     * @brief Set the parameters of the capped tube.
     * @param radius Outer radius of the tube.
     * @param innerRadius Inner radius of the tube.
     * @param size Height of the tube.
     * @param slices Number of slices around the tube.
     * @param segments Number of segments along the tube's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the tube.
     */
    void set(double radius = 1.0, double innerRadius = 0.75, double size = 1.0, int slices = 32, int segments = 8,
             int rings = 1, double start = 0.0, double sweep = 2 * glm::pi<double>());

protected:
    /**
     * @brief Internal setup method for initializing the capped tube geometry.
     */
    virtual void doSetup();

    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef CappedTubePtr
 * @brief Shared pointer type for CappedTube.
 */
typedef std::shared_ptr<CappedTube> CappedTubePtr;

} // namespace ivf
