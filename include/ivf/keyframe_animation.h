#pragma once

#include <memory>
#include <ivfmath/keyframe_interpolator.h>
#include <ivf/transform_node.h>

namespace ivf {

/**
 * @class KeyframeAnimation
 * @brief Animation system for interpolating a TransformNode using keyframes.
 *
 * The KeyframeAnimation class manages the animation of a TransformNode by interpolating
 * its transformation properties (position, rotation, scale) over time using a keyframe
 * interpolator. It supports play, stop, and time update operations.
 */
class KeyframeAnimation {
public:
    /**
     * @brief Construct a keyframe animation for a given node.
     * @param node The TransformNode to animate.
     */
    explicit KeyframeAnimation(TransformNodePtr node);

    /**
     * @brief Factory method to create a shared pointer to a KeyframeAnimation instance.
     * @param node The TransformNode to animate.
     * @return std::shared_ptr<KeyframeAnimation> New KeyframeAnimation instance.
     */
    static std::shared_ptr<KeyframeAnimation> create(TransformNodePtr node);

    /**
     * @brief Set the node to be animated.
     * @param node The TransformNode to animate.
     */
    void setNode(TransformNodePtr node);

    /**
     * @brief Get the currently animated node.
     * @return TransformNodePtr The animated node.
     */
    TransformNodePtr node() const;

    /**
     * @brief Set the keyframe interpolator.
     * @param interpolator The keyframe interpolator to use.
     */
    void setInterpolator(const ivfmath::KeyframeInterpolatorPtr &interpolator);

    /**
     * @brief Get the current keyframe interpolator.
     * @return ivfmath::KeyframeInterpolatorPtr The keyframe interpolator.
     */
    ivfmath::KeyframeInterpolatorPtr interpolator() const;

    /**
     * @brief Update the animation by advancing time.
     * @param deltaTime Time increment (in seconds).
     */
    void update(float deltaTime);

    /**
     * @brief Start playing the animation.
     */
    void play();

    /**
     * @brief Stop the animation.
     */
    void stop();

    /**
     * @brief Check if the animation is currently playing.
     * @return bool True if playing, false otherwise.
     */
    bool isPlaying() const;

    /**
     * @brief Get the current animation time.
     * @return float Current time in seconds.
     */
    float getCurrentTime() const;

private:
    bool m_playing; ///< Whether the animation is currently playing.
    TransformNodePtr m_node; ///< The node being animated.
    ivfmath::KeyframeInterpolatorPtr m_interpolator; ///< Keyframe interpolator.
    float m_currentTime; ///< Current animation time.
};

/**
 * @typedef KeyframeAnimationPtr
 * @brief Shared pointer type for KeyframeAnimation.
 */
typedef std::shared_ptr<KeyframeAnimation> KeyframeAnimationPtr;

} // namespace ivf
