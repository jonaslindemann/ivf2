#pragma once

#include <ivf/effect.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class FeedbackEffect
 * @brief Video-feedback / trails post-processing effect.
 *
 * The FeedbackEffect blends the current frame with a transformed copy of the previous
 * frame's final composite (sampled from the PostProcessor's "previousFrame" texture).
 * By applying a per-frame zoom, rotation and offset to the fed-back image and decaying
 * its color, it produces classic creative-coding looks: trails, smears and infinite
 * zoom/rotation tunnels.
 *
 * Requires the PostProcessor previous-frame path (texture unit 1). Inherits from Effect.
 */
class FeedbackEffect : public Effect {
private:
    float m_feedbackAmount = 0.9f;                    ///< 0 = current only, 1 = previous only.
    float m_zoom = 1.0f;                              ///< Per-frame zoom of the fed-back image.
    float m_rotation = 0.0f;                          ///< Per-frame rotation (radians).
    float m_offsetX = 0.0f;                           ///< Horizontal pan of the fed-back image.
    float m_offsetY = 0.0f;                           ///< Vertical pan of the fed-back image.
    glm::vec3 m_colorDecay = glm::vec3(0.98f);        ///< Per-channel decay of the fed-back color.

public:
    /**
     * @brief Default constructor.
     */
    FeedbackEffect();

    /**
     * @brief Factory method to create a shared pointer to a FeedbackEffect instance.
     * @return std::shared_ptr<FeedbackEffect> New FeedbackEffect instance.
     */
    static std::shared_ptr<FeedbackEffect> create();

    void setFeedbackAmount(float amount); ///< Set how much of the previous frame persists [0..1].
    float feedbackAmount() const;

    void setZoom(float zoom); ///< Set the per-frame zoom of the fed-back image.
    float zoom() const;

    void setRotation(float radians); ///< Set the per-frame rotation of the fed-back image.
    float rotation() const;

    void setOffset(float x, float y); ///< Set the per-frame pan of the fed-back image.
    float offsetX() const;
    float offsetY() const;

    void setColorDecay(glm::vec3 decay); ///< Set the per-channel decay of the fed-back color.
    glm::vec3 colorDecay() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
    virtual void setupProperties() override;
};

/**
 * @typedef FeedbackEffectPtr
 * @brief Shared pointer type for FeedbackEffect.
 */
typedef std::shared_ptr<FeedbackEffect> FeedbackEffectPtr;

} // namespace ivf
