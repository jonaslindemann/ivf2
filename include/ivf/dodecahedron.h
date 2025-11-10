#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
 * double radius = 1.0, int segments = 1, int rings = 1
 */

namespace ivf {

/**
 * @class Dodecahedron
 * @brief Node representing a 3D dodecahedron mesh with configurable parameters.
 *
 * The Dodecahedron class provides a 3D dodecahedron geometry. The dodecahedron's radius,
 * number of segments, and number of rings can be customized. Inherits from MeshNode,
 * allowing transformation and integration into a scene graph.
 */
class Dodecahedron : public MeshNode {
private:
    double m_radius; ///< Radius of the dodecahedron.
    int m_segments;  ///< Number of segments for mesh refinement.
    int m_rings;     ///< Number of rings for mesh refinement.

public:
    /**
     * @brief Constructor.
     * @param radius Radius of the dodecahedron.
     * @param segments Number of segments for mesh refinement.
     * @param rings Number of rings for mesh refinement.
     */
    Dodecahedron(double radius = 1.0, int segments = 1, int rings = 1);

    /**
     * @brief Factory method to create a shared pointer to a Dodecahedron instance.
     * @param radius Radius of the dodecahedron.
     * @param segments Number of segments for mesh refinement.
     * @param rings Number of rings for mesh refinement.
     * @return std::shared_ptr<Dodecahedron> New Dodecahedron instance.
     */
    static std::shared_ptr<Dodecahedron> create(double radius = 1.0, int segments = 1, int rings = 1);

    /**
     * @brief Set the parameters of the dodecahedron.
     * @param radius Radius of the dodecahedron.
     * @param segments Number of segments for mesh refinement.
     * @param rings Number of rings for mesh refinement.
     */
    void set(double radius = 1.0, int segments = 1, int rings = 1);

    void setRadius(double radius);
    void setSegments(int segments);
    void setRings(int rings);

    double radius() const;
    int segments() const;
    int rings() const;

protected:
    /**
     * @brief Internal setup method for initializing the dodecahedron geometry.
     */
    virtual void doSetup();

    virtual void setupProperties() override;
    virtual void onPropertyChanged(const std::string &name) override;
};

/**
 * @typedef DodecahedronPtr
 * @brief Shared pointer type for Dodecahedron.
 */
typedef std::shared_ptr<Dodecahedron> DodecahedronPtr;

} // namespace ivf
