#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class PixelationEffect
 * @brief Post-processing effect that applies a pixelation filter to the rendered image.
 *
 * The PixelationEffect class implements a post-processing effect that simulates a pixelated
 * look by rendering the scene at a lower resolution and upscaling it. The pixel size can be
 * configured to control the granularity of the effect. Inherits from Effect and overrides
 * resource loading and parameter update methods.
 */
class PixelationEffect : public Effect {
private:
    float m_pixelSize{4.0f}; ///< Size of the pixel blocks for the effect.

public:
    /**
     * @brief Default constructor.
     */
    PixelationEffect();

    /**
     * @brief Destructor.
     */
    virtual ~PixelationEffect();

    /**
     * @brief Factory method to create a shared pointer to a PixelationEffect instance.
     * @return std::shared_ptr<PixelationEffect> New PixelationEffect instance.
     */
    static std::shared_ptr<PixelationEffect> create();

    /**
     * @brief Set the pixel size for the effect.
     * @param size Size of the pixel blocks.
     */
    void setPixelSize(float size);

    /**
     * @brief Get the current pixel size for the effect.
     * @return float Pixel block size.
     */
    float pixelSize();

protected:
    /**
     * @brief Load effect-specific resources (e.g., shaders).
     */
    virtual void doLoad();

    /**
     * @brief Update effect-specific parameters (e.g., uniforms).
     */
    virtual void doUpdateParams();

    /**
     * @brief Setup properties for the effect, allowing configuration through a UI.
     */
    virtual void setupProperties() override;
};

/**
 * @typedef PixelationEffectPtr
 * @brief Shared pointer type for PixelationEffect.
 */
typedef std::shared_ptr<PixelationEffect> PixelationEffectPtr;

} // namespace ivf
