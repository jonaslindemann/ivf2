#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class DepthOfFieldEffect
 * @brief Depth-of-field (bokeh) post-processing effect based on scene depth.
 *
 * Surfaces within focusRange() of the focus distance stay sharp; beyond that
 * the blur grows over blurRange() world units up to maxBlur() pixels. The blur
 * is gathered in a single pass with each sample weighted by its own blur size,
 * so sharp objects do not bleed into blurred surroundings while a blurred
 * foreground still spreads over what lies behind it.
 *
 * With auto focus enabled, the focus distance is taken from the surface at the
 * center of the screen every frame. showBlur() renders the blur amount instead
 * of the image (red in front of the focus, blue behind) for tuning.
 *
 * The blur uses the scene depth, so place it before effects that move pixels
 * (wave, swirl, kaleidoscope). Cost grows with maxBlur() squared divided by
 * radiusStep().
 *
 * @code
 * auto dof = DepthOfFieldEffect::create();
 * dof->setFocusDistance(8.0f);
 * dof->setFocusRange(1.5f);
 * dof->setBlurRange(6.0f);
 * dof->load();
 * addEffect(dof);
 * @endcode
 */
class DepthOfFieldEffect : public Effect {
private:
    float m_focusDistance{10.0f}; ///< Distance from the camera that is in focus.
    float m_focusRange{1.0f};     ///< Distance on either side of the focus that stays fully sharp.
    float m_blurRange{8.0f};      ///< Distance over which the blur ramps up to the maximum.
    float m_maxBlur{12.0f};       ///< Maximum blur radius in pixels.
    float m_radiusStep{0.75f};    ///< Sampling density; smaller is smoother but slower.
    bool m_autoFocus{false};      ///< Focus on the surface at the screen center.
    bool m_showBlur{false};       ///< Visualize the blur amount instead of the image.

public:
    DepthOfFieldEffect();

    static std::shared_ptr<DepthOfFieldEffect> create();

    void setFocusDistance(float distance);
    float focusDistance() const;

    void setFocusRange(float range);
    float focusRange() const;

    void setBlurRange(float range);
    float blurRange() const;

    void setMaxBlur(float pixels);
    float maxBlur() const;

    void setRadiusStep(float step);
    float radiusStep() const;

    void setAutoFocus(bool flag);
    bool autoFocus() const;

    void setShowBlur(bool flag);
    bool showBlur() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
    virtual void setupProperties() override;
};

typedef std::shared_ptr<DepthOfFieldEffect> DepthOfFieldEffectPtr;

} // namespace ivf
