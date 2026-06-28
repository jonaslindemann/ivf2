#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class MotionBlurEffect
 * @brief Temporal motion-blur post-processing effect.
 *
 * The MotionBlurEffect accumulates the previous frame's final composite into the current
 * frame, producing smeared trails behind moving content. It is a simple temporal blend
 * (mix of current and previous frame) controlled by a single blend factor.
 *
 * Requires the PostProcessor previous-frame path (texture unit 1). Inherits from Effect.
 */
class MotionBlurEffect : public Effect {
private:
    float m_blendFactor = 0.5f; ///< Fraction of the previous frame retained (0 = none, ->1 = long trails).

public:
    /**
     * @brief Default constructor.
     */
    MotionBlurEffect();

    /**
     * @brief Factory method to create a shared pointer to a MotionBlurEffect instance.
     * @return std::shared_ptr<MotionBlurEffect> New MotionBlurEffect instance.
     */
    static std::shared_ptr<MotionBlurEffect> create();

    void setBlendFactor(float blendFactor); ///< Set the temporal blend factor [0..1].
    float blendFactor() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
    virtual void setupProperties() override;
};

/**
 * @typedef MotionBlurEffectPtr
 * @brief Shared pointer type for MotionBlurEffect.
 */
typedef std::shared_ptr<MotionBlurEffect> MotionBlurEffectPtr;

} // namespace ivf
