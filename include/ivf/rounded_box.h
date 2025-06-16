#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

/**
 * @class RoundedBox
 * @brief MeshNode representing a box with rounded edges and corners.
 *
 * The RoundedBox class generates a 3D box mesh with rounded edges and corners.
 * The size, segment count, rounding radius, and slice count are configurable.
 * Inherits from MeshNode, allowing transformation and integration into a scene graph.
 */
class RoundedBox : public MeshNode {
private:
    glm::vec3 m_size;      ///< Size of the box (width, height, depth).
    glm::uvec3 m_segments; ///< Number of segments along each axis.
    double m_radius;       ///< Rounding radius for edges and corners.
    int m_slices;          ///< Number of slices for rounded corners.

public:
    /**
     * @brief Constructor.
     * @param size Size of the box (width, height, depth).
     * @param segments Number of segments along each axis.
     * @param radius Rounding radius.
     * @param slices Number of slices for rounded corners.
     */
    RoundedBox(glm::vec3 size = {0.75f, 0.75f, 0.75f}, glm::vec3 segments = {8, 8, 8}, double radius = 0.25,
               int slices = 4);

    /**
     * @brief Factory method to create a shared pointer to a RoundedBox instance.
     * @param size Size of the box (width, height, depth).
     * @param segments Number of segments along each axis.
     * @param radius Rounding radius.
     * @param slices Number of slices for rounded corners.
     * @return std::shared_ptr<RoundedBox> New RoundedBox instance.
     */
    static std::shared_ptr<RoundedBox> create(glm::vec3 size = {0.75f, 0.75f, 0.75f}, glm::vec3 segments = {8, 8, 8},
                                              double radius = 0.25, int slices = 4);

    /**
     * @brief Set all parameters of the rounded box.
     * @param size Size of the box (width, height, depth).
     * @param segments Number of segments along each axis.
     * @param radius Rounding radius.
     * @param slices Number of slices for rounded corners.
     */
    void set(glm::vec3 size = {0.75f, 0.75f, 0.75f}, glm::vec3 segments = {8, 8, 8}, double radius = 0.25,
             int slices = 4);

    /**
     * @brief Set the size of the box.
     * @param w Width.
     * @param h Height.
     * @param d Depth.
     */
    void setSize(double w, double h, double d);

    /**
     * @brief Set the size of the box using a vector.
     * @param size Size vector (width, height, depth).
     */
    void setSize(glm::vec3 size);

    /**
     * @brief Get the current size of the box.
     * @return glm::vec3 Size vector.
     */
    glm::vec3 size();

    /**
     * @brief Set the number of segments along each axis.
     * @param i Segments along X.
     * @param j Segments along Y.
     * @param k Segments along Z.
     */
    void setSegments(int i, int j, int k);

    /**
     * @brief Set the number of segments using a vector.
     * @param segments Segment count vector.
     */
    void setSegments(glm::uvec3 segments);

    /**
     * @brief Get the current segment count.
     * @return glm::uvec3 Segment count vector.
     */
    glm::uvec3 segments();

    /**
     * @brief Set the rounding radius.
     * @param radius Rounding radius.
     */
    void setRadius(double radius);

    /**
     * @brief Get the current rounding radius.
     * @return double Rounding radius.
     */
    double radius();

    /**
     * @brief Set the number of slices for rounded corners.
     * @param slices Number of slices.
     */
    void setSlices(int slices);

    /**
     * @brief Get the current number of slices for rounded corners.
     * @return int Number of slices.
     */
    int slices();

protected:
    /**
     * @brief Internal setup method for initializing the rounded box geometry.
     */
    virtual void doSetup();
};

/**
 * @typedef RoundedBoxPtr
 * @brief Shared pointer type for RoundedBox.
 */
typedef std::shared_ptr<RoundedBox> RoundedBoxPtr;

} // namespace ivf
