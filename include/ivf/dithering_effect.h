#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class DitheringEffect
 * @brief Post-processing effect that applies dithering to the rendered image.
 *
 * The DitheringEffect class implements a dithering effect, which reduces color banding
 * and simulates higher color depth by applying a dithering pattern to the output image.
 * Inherits from Effect and can be used as part of a rendering pipeline.
 */
class DitheringEffect : public Effect {
private:
    // No additional members for this effect.
public:
    /**
     * @brief Default constructor.
     */
    DitheringEffect();

    /**
     * @brief Factory method to create a shared pointer to a DitheringEffect instance.
     * @return std::shared_ptr<DitheringEffect> New DitheringEffect instance.
     */
    static std::shared_ptr<DitheringEffect> create();

protected:
    /**
     * @brief Load and initialize the dithering effect resources (e.g., shaders).
     */
    virtual void doLoad();

    /**
     * @brief Update the dithering effect parameters in the shader or effect pipeline.
     */
    virtual void doUpdateParams();
};

/**
 * @typedef DitheringEffectPtr
 * @brief Shared pointer type for DitheringEffect.
 */
typedef std::shared_ptr<DitheringEffect> DitheringEffectPtr;

} // namespace ivf
