#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class SsaoEffect
 * @brief Screen-space ambient occlusion: darkens creases, corners and contact areas.
 *
 * Works from the scene's depth buffer: for every pixel it tests points in the hemisphere
 * above the surface and counts how many lie behind other surfaces. A depth-aware blur
 * removes the sampling noise without smearing across edges. Needs render to texture;
 * add it before effects that change colours or blur (bloom, tone mapping in the
 * window's output step comes after all effects anyway).
 *
 * @code
 * setRenderToTexture(true);
 * auto ssao = SsaoEffect::create();
 * ssao->setRadius(0.5f);     // world units: size of the creases that get darker
 * ssao->setIntensity(1.0f);
 * ssao->load();
 * addEffect(ssao);
 * @endcode
 *
 * Like all screen-space effects, it only knows what is visible: occluders outside the
 * screen or hidden behind other objects do not count.
 */
class SsaoEffect : public Effect {
private:
    float m_radius{0.5f};    ///< Sample radius in world units.
    float m_bias{0.025f};    ///< Depth bias in world units (against self-occlusion).
    float m_intensity{1.0f}; ///< 0 no effect .. 1 full occlusion.
    float m_power{1.5f};     ///< Contrast of the occlusion.
    int m_samples{16};       ///< Samples per pixel (1-64).
    int m_blurRadius{2};     ///< Blur radius in pixels (0-4).
    bool m_showOcclusion{false}; ///< Show the occlusion instead of the image.

public:
    SsaoEffect();

    [[nodiscard]] static std::shared_ptr<SsaoEffect> create();

    /** @brief Sample radius in world units: how far away surfaces still occlude (default 0.5). */
    void setRadius(float radius);
    [[nodiscard]] float radius() const;

    /** @brief Depth bias in world units against self-occlusion of flat surfaces (default 0.025). */
    void setBias(float bias);
    [[nodiscard]] float bias() const;

    /** @brief Strength: 0 leaves the image unchanged, 1 applies the full occlusion (default 1). */
    void setIntensity(float intensity);
    [[nodiscard]] float intensity() const;

    /** @brief Contrast: the occlusion is raised to this power (default 1.5). */
    void setPower(float power);
    [[nodiscard]] float power() const;

    /** @brief Samples per pixel, 1-64 (default 16). More is smoother and slower. */
    void setSamples(int samples);
    [[nodiscard]] int samples() const;

    /** @brief Radius of the noise-removing blur in pixels, 0-4 (default 2). */
    void setBlurRadius(int radius);
    [[nodiscard]] int blurRadius() const;

    /** @brief Show the occlusion as a grey image instead of the scene (for tuning). */
    void setShowOcclusion(bool flag);
    [[nodiscard]] bool showOcclusion() const;

protected:
    void doLoad() override;
    void doUpdateParams() override;
    void setupProperties() override;
};

using SsaoEffectPtr = std::shared_ptr<SsaoEffect>;

} // namespace ivf
