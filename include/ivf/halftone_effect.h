#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class HalftoneEffect
 * @brief Comic / print-style halftone (dot-screen) post-processing effect.
 *
 * The HalftoneEffect reproduces the rendered image as a grid of dots whose size varies
 * with local brightness, mimicking newspaper/comic printing. The dot grid can be rotated,
 * the cell size adjusted, and the effect rendered either as a monochrome screen or as
 * rotated per-channel screens (CMYK-style).
 *
 * Single-pass effect with no temporal dependency. Inherits from Effect.
 */
class HalftoneEffect : public Effect {
private:
    float m_dotSize = 6.0f;    ///< Cell size in pixels.
    float m_angle = 0.0f;      ///< Screen rotation in radians.
    bool m_grayscale = false;  ///< true = monochrome dots, false = per-channel screens.

public:
    /**
     * @brief Default constructor.
     */
    HalftoneEffect();

    /**
     * @brief Factory method to create a shared pointer to a HalftoneEffect instance.
     * @return std::shared_ptr<HalftoneEffect> New HalftoneEffect instance.
     */
    static std::shared_ptr<HalftoneEffect> create();

    void setDotSize(float dotSize); ///< Set the halftone cell size in pixels.
    float dotSize() const;

    void setAngle(float radians); ///< Set the screen rotation in radians.
    float angle() const;

    void setGrayscale(bool grayscale); ///< Toggle monochrome vs per-channel screens.
    bool grayscale() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
    virtual void setupProperties() override;
};

/**
 * @typedef HalftoneEffectPtr
 * @brief Shared pointer type for HalftoneEffect.
 */
typedef std::shared_ptr<HalftoneEffect> HalftoneEffectPtr;

} // namespace ivf
