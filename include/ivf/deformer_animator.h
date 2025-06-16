#pragma once

#include <ivf/deformer.h>

namespace ivf {

/**
 * @struct DeformerKeyframe
 * @brief Represents a keyframe for deformer animation.
 *
 * Each keyframe stores a time and a unique pointer to a Deformer instance.
 * Used by DeformerAnimator to interpolate and animate deformer parameters over time.
 */
struct DeformerKeyframe {
    float time;                         ///< Time of the keyframe.
    std::unique_ptr<Deformer> deformer; ///< Deformer state at this keyframe.

    /**
     * @brief Constructor.
     * @param t Time of the keyframe.
     * @param def Unique pointer to the deformer for this keyframe.
     */
    DeformerKeyframe(float t, std::unique_ptr<Deformer> def) : time(t), deformer(std::move(def))
    {}
};

/**
 * @class DeformerAnimator
 * @brief Animator for keyframe-based deformer animation.
 *
 * The DeformerAnimator class manages a sequence of DeformerKeyframes and applies interpolated
 * deformer states to a target Deformer based on the current animation time. Supports looping,
 * time control, and keyframe management for smooth mesh deformation animation.
 */
class DeformerAnimator : public Base {
private:
    std::vector<DeformerKeyframe> m_keyframes;  ///< List of keyframes for the animation.
    std::shared_ptr<Deformer> m_targetDeformer; ///< Target deformer to animate.
    float m_currentTime;                        ///< Current animation time.
    bool m_looping;                             ///< Whether the animation should loop.

public:
    /**
     * @brief Constructor.
     * @param target Shared pointer to the target deformer to animate.
     */
    DeformerAnimator(std::shared_ptr<Deformer> target);

    /**
     * @brief Factory method to create a shared pointer to a DeformerAnimator instance.
     * @param target Shared pointer to the target deformer to animate.
     * @return std::shared_ptr<DeformerAnimator> New DeformerAnimator instance.
     */
    static std::shared_ptr<DeformerAnimator> create(std::shared_ptr<Deformer> target);

    /**
     * @brief Add a keyframe at the specified time using the given deformer state.
     * @param time Time of the keyframe.
     * @param deformer Deformer state to store at this keyframe.
     */
    void addKeyframe(float time, const Deformer &deformer);

    /**
     * @brief Remove the keyframe at the specified index.
     * @param index Index of the keyframe to remove.
     */
    void removeKeyframe(size_t index);

    /**
     * @brief Remove all keyframes from the animation.
     */
    void clear();

    /**
     * @brief Set the current animation time.
     * @param time New animation time.
     */
    void setTime(float time);

    /**
     * @brief Get the current animation time.
     * @return float Current animation time.
     */
    float currentTime() const;

    /**
     * @brief Enable or disable looping of the animation.
     * @param loop True to enable looping, false to disable.
     */
    void setLooping(bool loop);

    /**
     * @brief Check if the animation is set to loop.
     * @return bool True if looping is enabled.
     */
    bool looping() const;

    /**
     * @brief Update the target deformer based on the current animation time.
     */
    void update();

    /**
     * @brief Get the total duration of the animation (time of the last keyframe).
     * @return float Animation duration.
     */
    float duration() const;

    /**
     * @brief Get the number of keyframes in the animation.
     * @return size_t Number of keyframes.
     */
    size_t keyframeCount() const;
};

}; // namespace ivf
