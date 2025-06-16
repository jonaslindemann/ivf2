#pragma once

/**
 * @file vignette_effect.h
 * @brief Declares the VignetteEffect class for applying a vignette post-processing effect in the ivf library.
 */

#include <ivf/effect.h>

namespace ivf {

/**
 * @class VignetteEffect
 * @brief Post-processing effect that applies a vignette (darkening at the image edges).
 *
 * The VignetteEffect class implements a configurable vignette effect for rendered images.
 * It allows control over the vignette size and smoothness, and integrates with the ivf
 * effect and property inspection system. Inherits from Effect.
 */
class VignetteEffect : public Effect {
private:
    float m_vignetteSize{1.5f};       ///< Size (radius) of the vignette effect.
    float m_vignetteSmoothness{1.0f}; ///< Smoothness (falloff) of the vignette edge.

public:
    /**
     * @brief Default constructor. Initializes the vignette effect with default parameters.
     */
    VignetteEffect();

    /**
     * @brief Factory method to create a shared pointer to a VignetteEffect instance.
     * @return std::shared_ptr<VignetteEffect> New VignetteEffect instance.
     */
    static std::shared_ptr<VignetteEffect> create();

    /**
     * @brief Set the size (radius) of the vignette effect.
     * @param size Vignette size value.
     */
    void setSize(float size);

    /**
     * @brief Get the current size (radius) of the vignette effect.
     * @return float Vignette size value.
     */
    float size();

    /**
     * @brief Set the smoothness (falloff) of the vignette edge.
     * @param smoothness Smoothness value.
     */
    void setSmoothness(float smoothness);

    /**
     * @brief Get the current smoothness (falloff) of the vignette edge.
     * @return float Smoothness value.
     */
    float smoothness();

protected:
    /**
     * @brief Load effect-specific resources (shaders, etc.).
     */
    virtual void doLoad();

    /**
     * @brief Update effect-specific parameters (uniforms, etc.).
     */
    virtual void doUpdateParams();

    /**
     * @brief Register properties for inspection (editor integration).
     */
    virtual void setupProperties() override;
};

/**
 * @typedef VignetteEffectPtr
 * @brief Shared pointer type for VignetteEffect.
 */
typedef std::shared_ptr<VignetteEffect> VignetteEffectPtr;

} // namespace ivf
