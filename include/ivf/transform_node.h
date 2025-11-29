#pragma once

#include <ivf/node.h>
#include <ivf/bounding_box.h>
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
    BoundingBox m_localBbox; ///< Local bounding box of this node
    bool m_autoUpdateBoundingBox; ///< Flag to enable/disable automatic bounding box updates

public:
    /**
     * @brief Default constructor.
     */
    TransformNode();

    /**
     * @brief Set the position of the node.
     * @param pos The new position as a glm::vec3.
     */
    inline void setPos(glm::vec3 pos) noexcept { m_pos = pos; }

    /**
     * @brief Get the position of the node.
     * @return The current position as a glm::vec3.
     */
    [[nodiscard]] inline glm::vec3 pos() const noexcept { return m_pos; }

    /**
     * @brief Enable or disable the use of transformation.
     * @param flag True to enable, false to disable.
     */
    inline void setUseTransform(bool flag) noexcept { m_useTransform = flag; }

    /**
     * @brief Check if transformation is enabled.
     * @return True if enabled, false otherwise.
     */
    [[nodiscard]] inline bool useTransform() const noexcept { return m_useTransform; }

    /**
     * @brief Set the axis of rotation.
     * @param axis The rotation axis as a glm::vec3.
     */
    inline void setRotAxis(glm::vec3 axis) noexcept { m_rotAxis = axis; }

    /**
     * @brief Get the axis of rotation.
     * @return The rotation axis as a glm::vec3.
     */
    [[nodiscard]] inline glm::vec3 rotAxis() const noexcept { return m_rotAxis; }

    /**
     * @brief Set the rotation angle.
     * @param angle The rotation angle.
     */
    inline void setRotAngle(float angle) noexcept { m_rotAngle = angle; }

    /**
     * @brief Get the rotation angle.
     * @return The rotation angle.
     */
    [[nodiscard]] inline float rotAngle() const noexcept { return m_rotAngle; }

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
    inline void setEulerAngles(glm::vec3 angles) noexcept { m_eulerAngles = angles; }

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
    inline void storePos() noexcept { m_storedPos = m_pos; }

    /**
     * @brief Restore the previously stored position of the node.
     */
    inline void restorePos() noexcept { m_pos = m_storedPos; }

    /**
     * @brief Get the stored position.
     * @return The stored position as a glm::vec3.
     */
    [[nodiscard]] inline glm::vec3 storedPos() const noexcept { return m_storedPos; }

    /**
     * @brief Set the scale of the node.
     * @param scale The scale factors as a glm::vec3.
     */
    inline void setScale(glm::vec3 scale) noexcept { m_scale = scale; }

    /**
     * @brief Get the scale of the node.
     * @return The scale factors as a glm::vec3.
     */
    [[nodiscard]] inline glm::vec3 scale() const noexcept { return m_scale; }

    /**
     * @brief Get the local transformation matrix.
     * @return The local transformation as a glm::mat4.
     */
    [[nodiscard]] glm::mat4 localTransform() const;

    /**
     * @brief Get the global transformation matrix.
     * @return The global transformation as a glm::mat4.
     */
    [[nodiscard]] glm::mat4 globalTransform() const;

    /**
     * @brief Get the world position of the node.
     * @return glm::vec3 The world position.
     */
    [[nodiscard]] glm::vec3 worldPos() const;

    /**
     * @brief Get the local bounding box of this node.
     * @return BoundingBox The local bounding box.
     */
    [[nodiscard]] virtual BoundingBox localBoundingBox() const;

    /**
     * @brief Get the world-space bounding box of this node.
     * @return BoundingBox The world-space bounding box.
     */
    [[nodiscard]] virtual BoundingBox worldBoundingBox() const;

    /**
     * @brief Set the local bounding box for this node.
     * @param bbox The local bounding box.
     */
    inline void setLocalBoundingBox(const BoundingBox& bbox) noexcept { m_localBbox = bbox; }

    /**
     * @brief Check if this node has a valid local bounding box.
     * @return bool True if the node has a valid bounding box.
     */
    [[nodiscard]] inline bool hasValidBoundingBox() const noexcept { return m_localBbox.isValid(); }

    /**
     * @brief Enable or disable automatic bounding box updates.
     * @param autoUpdate True to enable automatic updates, false to disable.
     */
    inline void setAutoUpdateBoundingBox(bool autoUpdate) noexcept { m_autoUpdateBoundingBox = autoUpdate; }

    /**
     * @brief Check if automatic bounding box updates are enabled.
     * @return bool True if auto-update is enabled.
     */
    [[nodiscard]] inline bool autoUpdateBoundingBox() const noexcept { return m_autoUpdateBoundingBox; }

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
using TransformNodePtr = std::shared_ptr<TransformNode>;

}; // namespace ivf
