#pragma once

#include <ivf/node.h>
#include <glm/glm.hpp>

namespace ivf {

/**
 * @class TransformNode
 * @brief Node that can be transformed in 3D space (translation, rotation, scaling).
 *
 * The TransformNode class provides translation, rotation, and scaling for scene nodes.
 * It supports setting position, rotation (axis/angle or Euler), and scale, and can
 * compute local and global transformation matrices. Inherits from Node.
 */
class TransformNode : public Node {
private:
    glm::vec3 m_pos;         ///< Position of the node in 3D space
    glm::vec3 m_rotAxis;     ///< Axis of rotation
    float m_rotAngle;        ///< Angle of rotation (in radians or degrees, depending on implementation)
    glm::vec3 m_scale;       ///< Scale factors along each axis
    bool m_useTransform;     ///< Flag to enable/disable transformation
    glm::vec3 m_vecRot;      ///< Vector rotation (for advanced rotation control)
    glm::vec3 m_eulerAngles; ///< Euler angles for rotation
    glm::vec3 m_storedPos;   ///< Stored position for temporary transformations

public:
    /**
     * @brief Default constructor.
     */
    TransformNode();

    /**
     * @brief Set the position of the node.
     * @param pos The new position as a glm::vec3.
     */
    void setPos(glm::vec3 pos);

    /**
     * @brief Get the position of the node.
     * @return The current position as a glm::vec3.
     */
    glm::vec3 pos() const;

    /**
     * @brief Enable or disable the use of transformation.
     * @param flag True to enable, false to disable.
     */
    void setUseTransform(bool flag);

    /**
     * @brief Check if transformation is enabled.
     * @return True if enabled, false otherwise.
     */
    bool useTransform() const;

    /**
     * @brief Set the axis of rotation.
     * @param axis The rotation axis as a glm::vec3.
     */
    void setRotAxis(glm::vec3 axis);

    /**
     * @brief Get the axis of rotation.
     * @return The rotation axis as a glm::vec3.
     */
    glm::vec3 rotAxis() const;

    /**
     * @brief Set the rotation angle.
     * @param angle The rotation angle.
     */
    void setRotAngle(float angle);

    /**
     * @brief Get the rotation angle.
     * @return The rotation angle.
     */
    float rotAngle() const;

    /**
     * @brief Set the Euler angles for rotation.
     * @param ax Rotation around X axis.
     * @param ay Rotation around Y axis.
     * @param az Rotation around Z axis.
     */
    void setEulerAngles(float ax, float ay, float az);

    /**
     * @brief Set the Euler angles for rotation.
     * @param angles The Euler angles as a glm::vec3.
     */
    void setEulerAngles(glm::vec3 angles);

    /**
     * @brief Rotate the node to face a target point.
     * @param target The target position as a glm::vec3.
     */
    void rotateTowards(glm::vec3 target);

    /**
     * @brief Align the node with a given axis and angle.
     * @param axis The axis to align with.
     * @param angle The angle to align with.
     */
    void alignWithAxisAngle(glm::vec3 axis, float angle);

    /**
     * @brief Rotate the node to align with a given vector.
     * @param v The vector to align with.
     */
    void rotateToVector(glm::vec3 v);

    /**
     * @brief Store the current position of the node.
     */
    void storePos();

    /**
     * @brief Restore the previously stored position of the node.
     */
    void restorePos();

    /**
     * @brief Get the stored position.
     * @return The stored position as a glm::vec3.
     */
    glm::vec3 storedPos() const;

    /**
     * @brief Set the scale of the node.
     * @param scale The scale factors as a glm::vec3.
     */
    void setScale(glm::vec3 scale);

    /**
     * @brief Get the scale of the node.
     * @return The scale factors as a glm::vec3.
     */
    glm::vec3 scale() const;

    /**
     * @brief Get the local transformation matrix.
     * @return The local transformation as a glm::mat4.
     */
    glm::mat4 localTransform() const;

    /**
     * @brief Get the global transformation matrix.
     * @return The global transformation as a glm::mat4.
     */
    glm::mat4 globalTransform() const;

    /**
     * @brief Get the world position of the node.
     * @return The world position as a glm::vec3.
     */
    glm::vec3 worldPos() const;

protected:
    /**
     * @brief Called before drawing the node. Override to perform actions before drawing.
     */
    virtual void doPreDraw();

    /**
     * @brief Called after drawing the node. Override to perform actions after drawing.
     */
    virtual void doPostDraw();

    /**
     * @brief Register properties for inspection.
     */
    virtual void setupProperties() override;
};

/**
 * @typedef TransformNodePtr
 * @brief Shared pointer to a TransformNode.
 */
typedef std::shared_ptr<TransformNode> TransformNodePtr;

}; // namespace ivf
