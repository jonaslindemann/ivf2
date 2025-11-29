#pragma once

#include <ivfmath/spline.h>
#include <ivf/glbase.h>
#include <ivf/transform_node.h>

namespace ivf {

/**
 * @enum SplineAnimMode
 * @brief Animation playback modes for spline animation.
 */
enum class SplineAnimMode {
    Once,    ///< Play once and stop at the end.
    Loop,    ///< Loop the animation.
    PingPong ///< Play forward and backward repeatedly.
};

/**
 * @enum SplineInterpolationMode
 * @brief Interpolation modes for spline animation.
 */
enum class SplineInterpolationMode {
    Time,    ///< Interpolate by time parameter.
    Distance ///< Interpolate by distance along the spline.
};

/**
 * @class SplineAnimation
 * @brief Animates a TransformNode along a spline path with configurable speed and mode.
 *
 * The SplineAnimation class animates a TransformNode along a spline path, supporting
 * different playback and interpolation modes. The animation can be controlled by time
 * or distance, and the speed, mode, and target node are configurable.
 */
class SplineAnimation : public GLBase {
private:
    ivfmath::SplinePtr m_spline;                                                ///< Spline path to animate along.
    double m_time{0.0};                                                         ///< Current animation time.
    double m_distance{0.0};                                                     ///< Current distance along the spline.
    double m_speed{1.0};                                                        ///< Animation speed.
    SplineAnimMode m_mode{SplineAnimMode::Once};                                ///< Playback mode.
    SplineInterpolationMode m_interpolationMode{SplineInterpolationMode::Time}; ///< Interpolation mode.

    TransformNodePtr m_animatedNode; ///< Node to animate along the spline.

public:
    /**
     * @brief Constructor.
     * @param spline Spline path to animate along.
     */
    SplineAnimation(ivfmath::SplinePtr spline = nullptr);

    /**
     * @brief Factory method to create a shared pointer to a SplineAnimation instance.
     * @param spline Spline path to animate along.
     * @return std::shared_ptr<SplineAnimation> New SplineAnimation instance.
     */
    static std::shared_ptr<SplineAnimation> create(ivfmath::SplinePtr spline = nullptr);

    /**
     * @brief Set the spline path to animate along.
     * @param spline Spline pointer.
     */
    void setSpline(ivfmath::SplinePtr spline);

    /**
     * @brief Get the current spline path.
     * @return ivfmath::SplinePtr Spline pointer.
     */
    ivfmath::SplinePtr spline();

    /**
     * @brief Set the node to animate.
     * @param node TransformNode pointer.
     */
    void setAnimatedNode(TransformNodePtr node);

    /**
     * @brief Get the currently animated node.
     * @return TransformNodePtr Animated node pointer.
     */
    TransformNodePtr animatedNode();

    /**
     * @brief Set the animation speed.
     * @param speed Speed value.
     */
    void setSpeed(double speed);

    /**
     * @brief Get the current animation speed.
     * @return double Speed value.
     */
    double speed();

    /**
     * @brief Set the animation playback mode.
     * @param mode SplineAnimMode value.
     */
    void setAnimMode(SplineAnimMode mode);

    /**
     * @brief Get the current animation playback mode.
     * @return SplineAnimMode Playback mode.
     */
    SplineAnimMode animMode();

    /**
     * @brief Set the interpolation mode (time or distance).
     * @param mode SplineInterpolationMode value.
     */
    void setInterpolationMode(SplineInterpolationMode mode);

    /**
     * @brief Get the current interpolation mode.
     * @return SplineInterpolationMode Interpolation mode.
     */
    SplineInterpolationMode interpolationMode();

    /**
     * @brief Set the current animation time.
     * @param time Time value.
     */
    void setTime(double time);

    /**
     * @brief Get the current animation time.
     * @return double Time value.
     */
    double time();

    /**
     * @brief Update the animation by a time step.
     * @param dt Time step (delta time).
     */
    void update(double dt);
};

/**
 * @typedef SplineAnimationPtr
 * @brief Shared pointer type for SplineAnimation.
 */
using SplineAnimationPtr = std::shared_ptr<SplineAnimation>;

}; // namespace ivf
