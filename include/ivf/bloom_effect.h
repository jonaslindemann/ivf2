#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class BloomEffect
 * @brief Post-processing effect that adds a bloom (glow) to bright areas of the rendered image.
 *
 * The BloomEffect class implements a bloom effect, which enhances the appearance of bright regions
 * in a scene by making them glow. The effect is controlled by a threshold (to select which pixels
 * are considered "bright") and an intensity (to control the strength of the glow).
 *
 * Inherits from Effect and can be used as part of a rendering pipeline.
 */
class BloomEffect : public Effect {
private:
    float m_threshold = 1.0; ///< Brightness threshold for bloom activation.
    float m_intensity = 1.0; ///< Intensity of the bloom effect.

public:
    /**
     * @brief Default constructor.
     */
    BloomEffect();

    /**
     * @brief Factory method to create a shared pointer to a BloomEffect instance.
     * @return std::shared_ptr<BloomEffect> New BloomEffect instance.
     */
    static std::shared_ptr<BloomEffect> create();

    /**
     * @brief Set the brightness threshold for the bloom effect.
     * @param threshold Pixels brighter than this value will contribute to the bloom.
     */
    void setThreshold(float threshold);

    /**
     * @brief Get the current brightness threshold.
     * @return float The current threshold value.
     */
    float threshold() const;

    /**
     * @brief Set the intensity of the bloom effect.
     * @param intensity The strength of the bloom glow.
     */
    void setIntensity(float intensity);

    /**
     * @brief Get the current intensity of the bloom effect.
     * @return float The current intensity value.
     */
    float intensity() const;

protected:
    /**
     * @brief Load and initialize the bloom effect resources (e.g., shaders).
     */
    virtual void doLoad();

    /**
     * @brief Update the bloom effect parameters (threshold, intensity) in the shader or effect pipeline.
     */
    virtual void doUpdateParams();

    /**
     * @brief Setup properties for the bloom effect (e.g., threshold, intensity).
     */
    virtual void setupProperties() override;
};

/**
 * @typedef BloomEffectPtr
 * @brief Shared pointer type for BloomEffect.
 */
typedef std::shared_ptr<BloomEffect> BloomEffectPtr;

} // namespace ivf
