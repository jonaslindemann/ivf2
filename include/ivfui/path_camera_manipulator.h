#pragma once

/**
 * @file path_camera_manipulator.h
 * @brief Camera that follows an authored path, with limited freedom for the viewer.
 *
 * @code
 * // Members of a GLFWSceneWindow subclass
 * ivfui::PathCameraManipulatorPtr m_pathCamera = ivfui::PathCameraManipulator::create();
 *
 * void onUpdate() override
 * {
 *     auto path = ivf::TimelineCamera::lookAt(pathPosition(t), {0, 0, 0});
 *     m_pathCamera->update(frameTime());
 *     m_pathCamera->applyTo(cameraManipulator(), path);
 * }
 *
 * void onMouseDrag(double x, double y, double dx, double dy, int button) override
 * {
 *     m_pathCamera->handleMouseDrag(dx, dy, button);
 * }
 *
 * void onScroll(double xoffset, double yoffset) override
 * {
 *     m_pathCamera->handleScroll(yoffset);
 * }
 * @endcode
 *
 * Controls (when the events are forwarded):
 *   LMB drag — look around (limited yaw and pitch)
 *   RMB drag — float sideways / up and down from the path
 *   Wheel    — float forward / backward along the view direction
 */

#include <ivf/scene_timeline.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivfui {

class CameraManipulator;

/**
 * @class PathCameraManipulator
 * @brief Adds viewer input on top of an authored camera path.
 *
 * The path itself is supplied every frame as an ivf::TimelineCamera, typically a
 * pure function of time, so renders and frame exports are reproducible. While
 * watching, the viewer can look around and float away from the path within
 * limits; after a moment without input the camera drifts back onto the path.
 *
 * The manipulator does not read input itself: forward mouse events with
 * handleMouseDrag() and handleScroll(), or call look() and move() directly.
 */
class PathCameraManipulator {
public:
    struct Settings {
        float maxYaw{0.6f};      ///< Look left/right limit, radians.
        float maxPitch{0.35f};   ///< Look up/down limit, radians.
        float maxOffset{3.0f};   ///< How far the viewer can float from the path.
        float minHeight{0.15f};  ///< Lowest camera height (y), keeps it above the ground or water.
        float lookSpeed{0.004f}; ///< Radians per pixel of mouse movement.
        float moveSpeed{0.01f};  ///< World units per pixel of mouse movement.
        float scrollSpeed{0.4f}; ///< World units per scroll step.
        float returnDelay{1.5f}; ///< Seconds without input before returning to the path.
        float returnRate{0.8f};  ///< Return speed (1/s); about 3/rate seconds to settle.
    };

    PathCameraManipulator() = default;

    static std::shared_ptr<PathCameraManipulator> create();

    [[nodiscard]] Settings &settings();
    [[nodiscard]] const Settings &settings() const;

    /** @brief Turn the view; mouse movement in pixels. */
    void look(float dx, float dy);

    /**
     * @brief Float away from the path, relative to the camera.
     * @param delta (right, up, forward) in world units.
     */
    void move(glm::vec3 delta);

    /**
     * @brief Forward a mouse drag: left button looks, right button floats sideways and up/down.
     * @param dx, dy Mouse movement in pixels.
     * @param button GLFW mouse button.
     */
    void handleMouseDrag(double dx, double dy, int button);

    /** @brief Forward a scroll step: floats forward or backward. */
    void handleScroll(double yoffset);

    /** @brief Advance the return to the path. Call once per frame. */
    void update(double dt);

    /** @brief Back onto the path immediately. */
    void reset();

    /** @brief When disabled, apply() returns the path unchanged (e.g. while exporting frames). */
    void setEnabled(bool enabled);
    [[nodiscard]] bool enabled() const;

    /**
     * @brief The path camera with the viewer's offset and look direction applied.
     *
     * The path's up vector is used as the yaw axis; fov and clip planes are passed through.
     */
    [[nodiscard]] ivf::TimelineCamera apply(const ivf::TimelineCamera &path) const;

    /**
     * @brief Apply the path camera to a window's camera manipulator.
     *
     * Sets only the camera position and target, so the projection (fov, clip
     * planes) configured on the manipulator is left as it is.
     */
    void applyTo(const std::shared_ptr<CameraManipulator> &manipulator, const ivf::TimelineCamera &path) const;

private:
    Settings m_settings;
    bool m_enabled{true};
    float m_yaw{0.0f};
    float m_pitch{0.0f};
    glm::vec3 m_offset{0.0f}; ///< (right, up, forward)
    double m_idle{0.0};
};

using PathCameraManipulatorPtr = std::shared_ptr<PathCameraManipulator>;

} // namespace ivfui
