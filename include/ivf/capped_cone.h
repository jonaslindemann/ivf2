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
 * @class CappedCone
 * @brief Node representing a 3D capped cone mesh with configurable parameters.
 *
 * The CappedCone class provides a 3D cone geometry with caps on both ends. The cone's radius, height (size),
 * number of slices, segments, rings, and angular sweep can be customized. Inherits from MeshNode, allowing
 * transformation and integration into a scene graph.
 */
class CappedCone : public MeshNode {
private:
    double m_radius{1.0};                  ///< Base radius of the cone.
    double m_size{1.0};                    ///< Height of the cone.
    int m_slices{32};                      ///< Number of slices around the cone.
    int m_segments{8};                     ///< Number of segments along the cone's height.
    int m_rings{4};                        ///< Number of rings for the caps.
    double m_start{0.0};                   ///< Start angle (in radians) for the sweep.
    double m_sweep{2 * glm::pi<double>()}; ///< Sweep angle (in radians) for the cone.

public:
    /**
     * @brief Constructor.
     * @param radius Base radius of the cone.
     * @param size Height of the cone.
     * @param slices Number of slices around the cone.
     * @param segments Number of segments along the cone's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cone.
     */
    CappedCone(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4,
               double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Factory method to create a shared pointer to a CappedCone instance.
     * @param radius Base radius of the cone.
     * @param size Height of the cone.
     * @param slices Number of slices around the cone.
     * @param segments Number of segments along the cone's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cone.
     * @return std::shared_ptr<CappedCone> New CappedCone instance.
     */
    static std::shared_ptr<CappedCone> create(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8,
                                              int rings = 4, double start = 0.0, double sweep = 2 * glm::pi<double>());

    /**
     * @brief Set the parameters of the capped cone.
     * @param radius Base radius of the cone.
     * @param size Height of the cone.
     * @param slices Number of slices around the cone.
     * @param segments Number of segments along the cone's height.
     * @param rings Number of rings for the caps.
     * @param start Start angle (in radians) for the sweep.
     * @param sweep Sweep angle (in radians) for the cone.
     */
    void set(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4,
             double start = 0.0, double sweep = 2 * glm::pi<double>());

protected:
    /**
     * @brief Internal setup method for initializing the capped cone geometry.
     */
    virtual void doSetup();
    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef CappedConePtr
 * @brief Shared pointer type for CappedCone.
 */
typedef std::shared_ptr<CappedCone> CappedConePtr;

} // namespace ivf
