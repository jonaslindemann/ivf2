#pragma once

/**
 * @file camera_window.h
 * @brief Declares the CameraWindow class for camera control UI in the ivfui library.
 */

#include <ivfui/ui_window.h>
#include <ivfui/camera_manipulator.h>

namespace ivfui {

/**
 * @class CameraWindow
 * @brief UI window for interactive camera control and visualization.
 *
 * The CameraWindow class provides a user interface for viewing and manipulating camera parameters
 * such as position, target, field of view, and clipping planes. It integrates with a CameraManipulator
 * for real-time camera updates and supports UI-driven changes via the doDraw and doUpdate methods.
 */
class CameraWindow : public ivfui::UiWindow {
private:
    CameraManipulatorPtr m_cameraManipulator; ///< Associated camera manipulator.
    ivf::CompositeNodePtr m_scene;             ///< Scene reference for extent calculation.

    float m_cameraTarget[3];      ///< Current camera target (look-at point).
    float m_newCameraTarget[3];   ///< Pending camera target (for UI changes).
    float m_cameraPosition[3];    ///< Current camera position.
    float m_newCameraPosition[3]; ///< Pending camera position (for UI changes).

    float m_fov;      ///< Current field of view.
    float m_newFov;   ///< Pending field of view (for UI changes).
    float m_nearZ;    ///< Current near clipping plane.
    float m_newNearZ; ///< Pending near clipping plane (for UI changes).
    float m_farZ;     ///< Current far clipping plane.
    float m_newFarZ;  ///< Pending far clipping plane (for UI changes).

    bool m_isDirty; ///< True if camera parameters have been modified via the UI.
    bool m_includeInvisible; ///< Include invisible nodes in extent calculation.

    /**
     * @brief Update UI controls to reflect the current camera state.
     */
    void updateControls();

    /**
     * @brief Apply UI changes to the camera manipulator.
     */
    void updateCamera();

public:
    /**
     * @brief Construct a CameraWindow with the given camera manipulator and caption.
     * @param camManip Shared pointer to the camera manipulator.
     * @param caption Window caption.
     */
    CameraWindow(CameraManipulatorPtr camManip, std::string caption);

    /**
     * @brief Factory method to create a shared pointer to a CameraWindow instance.
     * @param camManip Shared pointer to the camera manipulator.
     * @param caption Window caption.
     * @return std::shared_ptr<CameraWindow> New CameraWindow instance.
     */
    static std::shared_ptr<CameraWindow> create(CameraManipulatorPtr camManip, std::string caption);

    /**
     * @brief Set the scene reference for extent calculation.
     * @param scene Shared pointer to the scene.
     */
    void setScene(ivf::CompositeNodePtr scene);

    /**
     * @brief Get the scene reference.
     * @return ivf::CompositeNodePtr Scene reference.
     */
    ivf::CompositeNodePtr scene() const;

    /**
     * @brief Draw the camera window UI (called per frame).
     */
    void doDraw() override;

    /**
     * @brief Update the camera window state (called per frame).
     */
    void doUpdate() override;

    /**
     * @brief Check if the camera parameters have been modified via the UI.
     * @return bool True if dirty, false otherwise.
     */
    bool isDirty();
};

/**
 * @typedef CameraWindowPtr
 * @brief Shared pointer type for CameraWindow.
 */
typedef std::shared_ptr<CameraWindow> CameraWindowPtr;

} // namespace ivfui
