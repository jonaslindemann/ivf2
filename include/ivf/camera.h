#pragma once

#include <ivf/glbase.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class Camera
 * @brief Represents a 3D camera for viewing and projecting a scene.
 *
 * The Camera class encapsulates the parameters and operations for a 3D camera, including
 * position, orientation, projection (perspective or orthographic), and viewport settings.
 * It provides methods to configure the camera and apply its view/projection matrices.
 */
class Camera : public GLBase {
private:
    glm::vec3 m_position; ///< Camera position in world coordinates.
    glm::vec3 m_target;   ///< Point the camera is looking at.
    glm::vec3 m_up;       ///< Up direction for the camera.
    float m_fov;          ///< Field of view (in degrees) for perspective projection.
    float m_nearZ;        ///< Near clipping plane distance.
    float m_farZ;         ///< Far clipping plane distance.
    bool m_perspective;   ///< True if using perspective projection.
    bool m_ortho;         ///< True if using orthographic projection.
    int m_width;          ///< Viewport width in pixels.
    int m_height;         ///< Viewport height in pixels.

    /**
     * @brief Update the projection matrix based on current parameters.
     */
    void updateProjectMatrix();

    /**
     * @brief Update the view (look-at) matrix based on current parameters.
     */
    void updateLookAt();

public:
    /**
     * @brief Default constructor. Initializes camera with default parameters.
     */
    Camera();

    /**
     * @brief Factory method to create a shared pointer to a Camera instance.
     * @return std::shared_ptr<Camera> New Camera instance.
     */
    static std::shared_ptr<Camera> create();

    /**
     * @brief Apply the camera's view and projection matrices to the rendering context.
     */
    void apply();

    /**
     * @brief Set the camera position.
     * @param position New camera position.
     */
    void setPosition(const glm::vec3 &position);

    /**
     * @brief Set the camera target (look-at point).
     * @param target New target point.
     */
    void setTarget(const glm::vec3 &target);

    /**
     * @brief Set the camera up direction.
     * @param up New up vector.
     */
    void setUp(const glm::vec3 &up);

    /**
     * @brief Set the camera's look-at parameters.
     * @param eye Camera position.
     * @param center Target point.
     * @param up Up direction.
     */
    void setLookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up);

    /**
     * @brief Get the camera position.
     * @return glm::vec3 Current camera position.
     */
    glm::vec3 position() const;

    /**
     * @brief Get the camera target (look-at point).
     * @return glm::vec3 Current target point.
     */
    glm::vec3 target() const;

    /**
     * @brief Get the camera up direction.
     * @return glm::vec3 Current up vector.
     */
    glm::vec3 up() const;

    /**
     * @brief Set the field of view for perspective projection.
     * @param fov Field of view in degrees.
     */
    void setFov(float fov);

    /**
     * @brief Set the near clipping plane distance.
     * @param nearZ Near plane distance.
     */
    void setNearZ(float nearZ);

    /**
     * @brief Set the far clipping plane distance.
     * @param farZ Far plane distance.
     */
    void setFarZ(float farZ);

    /**
     * @brief Set perspective projection parameters.
     * @param fov Field of view in degrees.
     * @param nearZ Near plane distance.
     * @param farZ Far plane distance.
     */
    void setPerspective(float fov, float nearZ, float farZ);

    /**
     * @brief Enable or disable perspective projection.
     * @param perspective True to enable perspective, false otherwise.
     */
    void setPerspective(bool perspective);

    /**
     * @brief Enable or disable orthographic projection.
     * @param ortho True to enable orthographic, false otherwise.
     */
    void setOrtho(bool ortho);

    /**
     * @brief Get the current field of view.
     * @return float Field of view in degrees.
     */
    float fov() const;

    /**
     * @brief Get the near clipping plane distance.
     * @return float Near plane distance.
     */
    float nearZ() const;

    /**
     * @brief Get the far clipping plane distance.
     * @return float Far plane distance.
     */
    float farZ() const;

    /**
     * @brief Check if the camera is using perspective projection.
     * @return bool True if perspective projection is enabled.
     */
    bool isPerspective() const;

    /**
     * @brief Check if the camera is using orthographic projection.
     * @return bool True if orthographic projection is enabled.
     */
    bool isOrtho() const;

    /**
     * @brief Set the viewport size.
     * @param width Viewport width in pixels.
     * @param height Viewport height in pixels.
     */
    void setViewport(int width, int height);

    /**
     * @brief Get the viewport width.
     * @return int Viewport width in pixels.
     */
    int width() const;

    /**
     * @brief Get the viewport height.
     * @return int Viewport height in pixels.
     */
    int height() const;
};

/**
 * @typedef CameraPtr
 * @brief Shared pointer type for Camera.
 */
typedef std::shared_ptr<Camera> CameraPtr;

} // namespace ivf
