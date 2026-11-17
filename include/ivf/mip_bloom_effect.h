#pragma once

/**
 * @file mip_bloom_effect.h
 * @brief Wide bloom from the image's mip chain, with soft tone mapping.
 */

#include <ivf/effect.h>

namespace ivf {

/**
 * @class MipBloomEffect
 * @brief Bloom that spreads bright light into a soft, wide halo, then tone maps.
 *
 * Bright parts of the image (above the threshold, with a soft knee) are taken
 * from several mip levels of the input (blurs of increasing size), each smoothed
 * with a small tent filter, and added back. The result is tone mapped with a
 * soft shoulder, so emissive values above 1 roll off into white instead of
 * clipping.
 *
 * Compared with BloomEffect the halo is much wider and cheaper to compute, which
 * suits glowing particles and light against dark backgrounds. The post processor
 * builds the mip chain because the shader declares `screenTextureLevels`.
 * Place it last in the effect chain.
 */
class MipBloomEffect : public Effect {
private:
    float m_threshold{0.6f}; ///< Brightness where the bloom starts.
    float m_intensity{0.8f}; ///< Amount of bloom added.
    float m_spread{0.75f};   ///< Weight falloff per blur level; higher gives wider halos [0, 1].
    float m_exposure{1.0f};  ///< Scale before tone mapping.

public:
    MipBloomEffect();

    static std::shared_ptr<MipBloomEffect> create();

    void setThreshold(float threshold);
    void setIntensity(float intensity);
    void setSpread(float spread);
    void setExposure(float exposure);

    [[nodiscard]] float threshold() const;
    [[nodiscard]] float intensity() const;
    [[nodiscard]] float spread() const;
    [[nodiscard]] float exposure() const;

protected:
    void doLoad() override;
    void doUpdateParams() override;
    void setupProperties() override;
};

using MipBloomEffectPtr = std::shared_ptr<MipBloomEffect>;

} // namespace ivf
