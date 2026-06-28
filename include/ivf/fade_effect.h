#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class FadeEffect
 * @brief Effect that fades the rendered image toward a solid color.
 *
 * The FadeEffect blends the rendered image with a configurable fade color by a
 * fade amount in the range [0, 1], where 0 leaves the image untouched and 1
 * replaces it entirely with the fade color. It is primarily intended for
 * fade-in / fade-out (dip-to-black) scene transitions. Inherits from Effect.
 */
class FadeEffect : public Effect {
private:
    glm::vec3 m_fadeColor{0.0f, 0.0f, 0.0f}; ///< Color to fade toward (default black).
    float m_fadeAmount{0.0f};                ///< Fade amount (0 = image, 1 = fade color).

public:
    /**
     * @brief Constructor.
     */
    FadeEffect();

    /**
     * @brief Factory method to create a shared pointer to a FadeEffect instance.
     * @return std::shared_ptr<FadeEffect> New FadeEffect instance.
     */
    static std::shared_ptr<FadeEffect> create();

    /**
     * @brief Set the color the image fades toward.
     * @param fadeColor Fade color (typically black).
     */
    void setFadeColor(glm::vec3 fadeColor);

    /**
     * @brief Get the current fade color.
     * @return glm::vec3 Fade color.
     */
    glm::vec3 fadeColor() const;

    /**
     * @brief Set the fade amount.
     * @param fadeAmount Amount in [0, 1]; 0 = image, 1 = fully faded.
     */
    void setFadeAmount(float fadeAmount);

    /**
     * @brief Get the current fade amount.
     * @return float Fade amount.
     */
    float fadeAmount() const;

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
     * @brief Setup properties for the effect (fade color and amount).
     */
    virtual void setupProperties() override;
};

/**
 * @typedef FadeEffectPtr
 * @brief Shared pointer type for FadeEffect.
 */
typedef std::shared_ptr<FadeEffect> FadeEffectPtr;

} // namespace ivf
