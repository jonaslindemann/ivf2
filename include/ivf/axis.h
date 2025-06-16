#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

/**
 * @class Axis
 * @brief Node representing a 3D coordinate axis for visualization.
 *
 * The Axis class provides a simple 3D axis, typically used for orientation or debugging in a scene.
 * It inherits from MeshNode, allowing it to be transformed and rendered as part of a scene graph.
 * The axis size can be customized.
 */
class Axis : public MeshNode {
private:
    GLfloat m_size; ///< Length of each axis line.
public:
    /**
     * @brief Constructor.
     * @param size Length of the axis lines (default is 1.0).
     */
    Axis(GLfloat size = 1.0);

    /**
     * @brief Destructor.
     */
    virtual ~Axis();

    /**
     * @brief Factory method to create a shared pointer to an Axis instance.
     * @param size Length of the axis lines (default is 1.0).
     * @return std::shared_ptr<Axis> New Axis instance.
     */
    static std::shared_ptr<Axis> create(GLfloat size = 1.0);

    /**
     * @brief Set the length of the axis lines.
     * @param size New size for the axis.
     */
    void setSize(GLfloat size);

    /**
     * @brief Get the current length of the axis lines.
     * @return GLfloat The current axis size.
     */
    GLfloat size();

protected:
    /**
     * @brief Internal setup method for initializing the axis geometry.
     */
    virtual void doSetup();

    /**
     * @brief Called before drawing the axis node.
     */
    virtual void doPreDraw();

    /**
     * @brief Called after drawing the axis node.
     */
    virtual void doPostDraw();
};

/**
 * @typedef AxisPtr
 * @brief Shared pointer type for Axis.
 */
typedef std::shared_ptr<Axis> AxisPtr;

}; // namespace ivf
