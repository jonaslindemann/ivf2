#pragma once

/**
 * @file camera_animation.h
 * @brief Declares CameraAnimation for spline-based camera fly-through and shake effects.
 */

#include <memory>
#include <glm/glm.hpp>
#include <ivfmath/spline.h>
#include <ivfui/camera_manipulator.h>
#include <ivf/spline_animation.h>  // for SplineAnimMode

namespace ivfui {

/**
 * @class CameraAnimation
 * @brief Animates the camera along a spline path with an optional shake effect.
 *
 * Define a fly-through by adding (position, target) keyframes. The camera eye and
 * look-at point each follow their own CatmullRom spline at constant speed. When
 * `setAutoLookForward(true)` the target spline is ignored and the camera automatically
 * looks along the tangent of the position path.
 *
 * Typical use:
 * @code
 * auto anim = CameraAnimation::create(cameraManipulator());
 * anim->addKeyframe({0, 5, 15}, {0, 0, 0});
 * anim->addKeyframe({8, 3,  5}, {0, 1, 0});
 * anim->addKeyframe({0, 6, -12}, {0, 0, 0});
 * anim->setDuration(8.0f);
 * anim->setMode(ivf::SplineAnimMode::Loop);
 * anim->play();
 *
 * // in onUpdate():
 * anim->update(TimeController::instance()->delta());
 * @endcode
 */
class CameraAnimation {
private:
    CameraManipulatorPtr m_cam;
    ivfmath::SplinePtr m_posSpline;
    ivfmath::SplinePtr m_targetSpline;

    float m_duration{5.0f};
    double m_elapsed{0.0};
    bool m_playing{false};
    bool m_forward{true};           // ping-pong direction
    ivf::SplineAnimMode m_mode{ivf::SplineAnimMode::Once};

    bool m_autoLookForward{false};  // when true, ignore target spline
    bool m_blockManipulator{true};  // block orbit/pan while playing

    // Shake state
    float m_shakeIntensity{0.0f};
    float m_shakeDuration{0.0f};
    double m_shakeElapsed{0.0};

    glm::vec3 shakeOffset() const;
    void applyToCamera();

public:
    CameraAnimation(CameraManipulatorPtr cam);

    static std::shared_ptr<CameraAnimation> create(CameraManipulatorPtr cam);

    /** Add a paired eye-position and look-at-target keyframe. */
    void addKeyframe(glm::vec3 pos, glm::vec3 target);

    /** Remove all keyframes and reset elapsed time. */
    void clearKeyframes();

    /** Duration in seconds to traverse the full path once. */
    void setDuration(float seconds);
    float duration() const;

    /** Playback mode: Once, Loop, or PingPong. */
    void setMode(ivf::SplineAnimMode mode);
    ivf::SplineAnimMode mode() const;

    /** When true the camera looks along the path tangent; target keyframes are ignored. */
    void setAutoLookForward(bool autoLook);
    bool autoLookForward() const;

    /** When true, orbit/pan input is blocked while the animation is playing. */
    void setBlockManipulatorDuringPlayback(bool block);

    void play();
    void stop();
    void rewind();

    bool isPlaying() const;

    /** 0=start, 1=end of one traversal. */
    float progress() const;

    /**
     * @brief Apply a decaying position jitter for the given duration.
     * @param intensity  Maximum random offset in world units.
     * @param duration   How long the shake lasts in seconds.
     */
    void shake(float intensity, float duration = 0.3f);

    /** Call once per frame with the scaled frame delta (e.g. TimeController::instance()->delta()). */
    void update(double dt);
};

using CameraAnimationPtr = std::shared_ptr<CameraAnimation>;

} // namespace ivfui
