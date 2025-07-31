#pragma once

/**
 * @file camera_manipulator.h
 * @brief Declares the CameraManipulator class for interactive camera control in the ivfui library.
 */

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <ivf/light_manager.h>

namespace ivfui {

/**
 * @class CameraManipulator
 * @brief Interactive camera controller for 3D navigation and manipulation.
 *
 * The CameraManipulator class provides mouse and keyboard-based camera control for 3D scenes.
 * It supports orbit, pan, zoom, and state management, and can be integrated with a headlight
 * (directional light) for scene illumination. Designed for use with GLFW windows.
 */
class CameraManipulator {
private:
    GLFWwindow *m_window;                 ///< Associated GLFW window.
    int m_width;                          ///< Window width.
    int m_height;                         ///< Window height.
    bool m_firstClick;                    ///< True if the next mouse click is the first.
    bool m_firstTime;                     ///< True if the manipulator is being used for the first time.
    bool m_leftMouseButton;               ///< Left mouse button state.
    bool m_middleMouseButton;             ///< Middle mouse button state.
    bool m_rightMouseButton;              ///< Right mouse button state.
    bool m_anyMouseButton;                ///< Any mouse button state.
    bool m_shiftKey;                      ///< Shift key state.
    bool m_ctrlKey;                       ///< Control key state.
    bool m_altKey;                        ///< Alt key state.
    double m_mouseStartX;                 ///< Mouse X position at drag start.
    double m_mouseStartY;                 ///< Mouse Y position at drag start.
    double m_mouseX;                      ///< Current mouse X position.
    double m_mouseY;                      ///< Current mouse Y position.
    double m_mouseScaleX;                 ///< Mouse X scaling factor.
    double m_mouseScaleY;                 ///< Mouse Y scaling factor.
    glm::vec3 m_cameraTarget;             ///< Camera look-at target.
    glm::vec3 m_cameraPosition;           ///< Camera position.
    glm::vec3 m_cameraNewPos;             ///< New camera position (for smooth transitions).
    glm::vec3 m_cameraNewTarget;          ///< New camera target (for smooth transitions).
    glm::vec3 m_savedCameraTarget;        ///< Saved camera target (for restore).
    glm::vec3 m_savedCameraPosition;      ///< Saved camera position (for restore).
    ivf::DirectionalLightPtr m_headlight; ///< Optional headlight (directional light).
    double m_fov;                         ///< Field of view (degrees).
    double m_nearZ;                       ///< Near clipping plane.
    double m_farZ;                        ///< Far clipping plane.

    // Multi-slot view storage
    struct ViewSlot {
        glm::vec3 position;
        glm::vec3 target;
        double fov;
        double nearZ;
        double farZ;
        bool hasData;

        ViewSlot() : position(0.0f), target(0.0f), fov(45.0), nearZ(1.0), farZ(100.0), hasData(false) {}
    };
    ViewSlot m_viewSlots[10]; ///< Storage for 10 view slots (0-9)

private:
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
     * @brief Construct a CameraManipulator for the given GLFW window.
     * @param window Pointer to the GLFW window.
     */
    CameraManipulator(GLFWwindow *window);

    /**
     * @brief Factory method to create a shared pointer to a CameraManipulator instance.
     * @param window Pointer to the GLFW window.
     * @return std::shared_ptr<CameraManipulator> New CameraManipulator instance.
     */
    static std::shared_ptr<CameraManipulator> create(GLFWwindow *window);

    /**
     * @brief Update the camera manipulator state (should be called per frame).
     */
    void update();

    /**
     * @brief Get the current camera target (look-at point).
     * @return glm::vec3 Camera target.
     */
    glm::vec3 cameraTarget();

    /**
     * @brief Get the current camera position.
     * @return glm::vec3 Camera position.
     */
    glm::vec3 cameraPosition();

    /**
     * @brief Set the camera target (look-at point).
     * @param target New camera target.
     */
    void setCameraTarget(glm::vec3 target);

    /**
     * @brief Set the camera position.
     * @param position New camera position.
     */
    void setCameraPosition(glm::vec3 position);

    /**
     * @brief Set the camera field of view (FOV).
     * @param fov Field of view in degrees.
     */
    void setFov(double fov);

    /**
     * @brief Set the near clipping plane.
     * @param nearZ Near plane distance.
     */
    void setNearZ(double nearZ);

    /**
     * @brief Set the far clipping plane.
     * @param farZ Far plane distance.
     */
    void setFarZ(double farZ);

    /**
     * @brief Get the camera field of view (FOV).
     * @return double Field of view in degrees.
     */
    double fov();

    /**
     * @brief Get the near clipping plane distance.
     * @return double Near plane distance.
     */
    double nearZ();

    /**
     * @brief Get the far clipping plane distance.
     * @return double Far plane distance.
     */
    double farZ();

    /**
     * @brief Reset the camera to its default state.
     */
    void reset();

    /**
     * @brief Save the current camera state (position and target).
     */
    void saveState();

    /**
     * @brief Restore the previously saved camera state.
     */
    void restoreState();

    /**
     * @brief Set the mouse scaling factors for camera manipulation.
     * @param sx X scaling factor.
     * @param sy Y scaling factor.
     */
    void setMouseScaling(double sx, double sy);

    /**
     * @brief Set the headlight (directional light) for the camera.
     * @param dirLight Shared pointer to the directional light.
     */
    void setHeadlight(ivf::DirectionalLightPtr dirLight);

    /**
     * @brief Get the current headlight (directional light).
     * @return ivf::DirectionalLightPtr Shared pointer to the headlight.
     */
    ivf::DirectionalLightPtr headlight();

    /**
     * @brief Zoom the camera to fit the entire scene extent.
     * @param scene The scene to compute extent for.
     * @param includeInvisible Whether to include invisible nodes in extent calculation.
     */
    void zoomToExtent(ivf::CompositeNodePtr scene, bool includeInvisible = false);

    /**
     * @brief Save camera state to a specific slot.
     * @param slot Slot number (0-9).
     */
    void saveStateToSlot(int slot);

    /**
     * @brief Restore camera state from a specific slot.
     * @param slot Slot number (0-9).
     */
    void restoreStateFromSlot(int slot);

    /**
     * @brief Check if a view slot has saved data.
     * @param slot Slot number (0-9).
     * @return bool True if slot has saved data.
     */
    bool hasSlotData(int slot) const;
};

/**
 * @typedef CameraManipulatorPtr
 * @brief Shared pointer type for CameraManipulator.
 */
typedef std::shared_ptr<CameraManipulator> CameraManipulatorPtr;

} // namespace ivfui
