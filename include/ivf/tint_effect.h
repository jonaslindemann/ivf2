#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class TintEffect
 * @brief Effect that applies a color tint and optional grayscale to the rendered image.
 *
 * The TintEffect class implements a post-processing effect that blends a tint color
 * with the rendered image, with configurable strength and grayscale weights. It is
 * useful for stylized rendering, highlighting, or color grading. Inherits from Effect.
 */
class TintEffect : public Effect {
private:
    glm::vec3 m_tintColor;        ///< Tint color to blend with the image.
    float m_tintStrength{0.5};    ///< Strength of the tint effect (0.0-1.0).
    glm::vec3 m_grayScaleWeights; ///< Weights for converting to grayscale.

public:
    /**
     * @brief Constructor.
     */
    TintEffect();

    /**
     * @brief Factory method to create a shared pointer to a TintEffect instance.
     * @return std::shared_ptr<TintEffect> New TintEffect instance.
     */
    static std::shared_ptr<TintEffect> create();

    /**
     * @brief Set the tint color.
     * @param tintColor Color to use for tinting.
     */
    void setTintColor(glm::vec3 tintColor);

    /**
     * @brief Get the current tint color.
     * @return glm::vec3 Tint color.
     */
    glm::vec3 tintColor() const;

    /**
     * @brief Set the strength of the tint effect.
     * @param tintStrength Strength value (0.0-1.0).
     */
    void setTintStrength(float tintStrength);

    /**
     * @brief Get the current tint strength.
     * @return float Tint strength.
     */
    float tintStrength() const;

    /**
     * @brief Set the grayscale weights for the effect.
     * @param grayScaleWeights Weights for RGB channels.
     */
    void setGrayScaleWeights(glm::vec3 grayScaleWeights);

    /**
     * @brief Get the current grayscale weights.
     * @return glm::vec3 Grayscale weights.
     */
    glm::vec3 grayScaleWeights() const;

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
     * @brief Setup properties for the effect, such as tint color and strength.
     */
    virtual void setupProperties() override;
};

/**
 * @typedef TintEffectPtr
 * @brief Shared pointer type for TintEffect.
 */
typedef std::shared_ptr<TintEffect> TintEffectPtr;

} // namespace ivf
