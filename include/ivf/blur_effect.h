#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class BlurEffect
 * @brief Post-processing effect that applies a blur to the rendered image.
 *
 * The BlurEffect class implements a blur effect, which softens the appearance of the scene
 * by averaging neighboring pixels. The strength of the blur is controlled by the blur radius.
 *
 * Inherits from Effect and can be used as part of a rendering pipeline.
 */
class BlurEffect : public Effect {
private:
    float m_blurRadius{0.0f}; ///< Radius of the blur effect.

public:
    /**
     * @brief Default constructor.
     */
    BlurEffect();

    /**
     * @brief Destructor.
     */
    virtual ~BlurEffect();

    /**
     * @brief Factory method to create a shared pointer to a BlurEffect instance.
     * @return std::shared_ptr<BlurEffect> New BlurEffect instance.
     */
    static std::shared_ptr<BlurEffect> create();

    /**
     * @brief Set the blur radius.
     * @param radius The radius of the blur effect.
     */
    void setBlurRadius(float radius);

    /**
     * @brief Get the current blur radius.
     * @return float The current blur radius.
     */
    float blurRadius();

protected:
    /**
     * @brief Load and initialize the blur effect resources (e.g., shaders).
     */
    virtual void doLoad();

    /**
     * @brief Update the blur effect parameters (e.g., blur radius) in the shader or effect pipeline.
     */
    virtual void doUpdateParams();

    /**
     * @brief Setup properties for the blur effect, such as blur radius.
     */
    virtual void setupProperties() override;
};

/**
 * @typedef BlurEffectPtr
 * @brief Shared pointer type for BlurEffect.
 */
typedef std::shared_ptr<BlurEffect> BlurEffectPtr;

} // namespace ivf
