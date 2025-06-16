#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class ChromaticEffect
 * @brief Post-processing effect that simulates chromatic aberration.
 *
 * The ChromaticEffect class implements a chromatic aberration effect, which offsets the color channels
 * to create a visual distortion similar to that seen in real-world lenses. The strength of the effect
 * is controlled by the offset parameter.
 *
 * Inherits from Effect and can be used as part of a rendering pipeline.
 */
class ChromaticEffect : public Effect {
private:
    float m_offset{0.01}; ///< Offset value for the chromatic aberration effect.

public:
    /**
     * @brief Default constructor.
     */
    ChromaticEffect();

    /**
     * @brief Destructor.
     */
    virtual ~ChromaticEffect();

    /**
     * @brief Factory method to create a shared pointer to a ChromaticEffect instance.
     * @return std::shared_ptr<ChromaticEffect> New ChromaticEffect instance.
     */
    static std::shared_ptr<ChromaticEffect> create();

    /**
     * @brief Set the offset for the chromatic aberration effect.
     * @param offset The offset value for color channel separation.
     */
    void setOffset(float offset);

    /**
     * @brief Get the current offset value.
     * @return float The current offset value.
     */
    float offset() const;

protected:
    /**
     * @brief Load and initialize the chromatic effect resources (e.g., shaders).
     */
    virtual void doLoad();

    /**
     * @brief Update the chromatic effect parameters (e.g., offset) in the shader or effect pipeline.
     */
    virtual void doUpdateParams();

    /**
     * @brief Setup properties for the chromatic effect (e.g., offset).
     */
    virtual void setupProperties() override;
};

/**
 * @typedef ChromaticEffectPtr
 * @brief Shared pointer type for ChromaticEffect.
 */
typedef std::shared_ptr<ChromaticEffect> ChromaticEffectPtr;

} // namespace ivf
