#pragma once

#include <ivf/effect.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class ScreenSpaceReflectionEffect
 * @brief Reflections of the visible scene on floors, water and other surfaces facing one direction.
 *
 * For each pixel of a reflecting surface the reflected ray is marched through the depth
 * buffer; where it passes behind a visible surface, that pixel's colour is the reflection.
 * Rough surfaces take it from a blurred version of the image. Needs render to texture.
 *
 * There is no per-material reflectivity: the surfaces that reflect are chosen by their
 * normal, within setSurfaceAngle() of setUpDirection() (default: everything facing up,
 * such as floors). Reflections fade at the screen edges and with distance, and show only
 * what is on screen; for exact mirrors use PlanarReflection.
 *
 * @code
 * setRenderToTexture(true);
 * auto ssr = ScreenSpaceReflectionEffect::create();
 * ssr->setStrength(0.5f);
 * ssr->setRoughness(0.2f);
 * ssr->load();
 * addEffect(ssr);
 * @endcode
 */
class ScreenSpaceReflectionEffect : public Effect {
private:
    float m_strength{0.6f};       ///< Reflection amount on a fully reflecting surface.
    float m_maxDistance{20.0f};   ///< Longest reflected ray in world units.
    int m_steps{48};              ///< Ray march steps (4-128).
    float m_thickness{0.5f};      ///< Depth of surfaces a ray may pass behind and still hit.
    float m_roughness{0.0f};      ///< 0 sharp mirror .. 1 blurred.
    float m_fresnel{0.5f};        ///< 0 same everywhere .. 1 only at grazing angles.
    float m_edgeFade{0.1f};       ///< Fade towards the screen edges (fraction of the screen).
    float m_surfaceAngle{25.0f};  ///< Largest angle (degrees) between a reflecting surface's normal and the up direction.
    glm::vec3 m_upDirection{0.0f, 1.0f, 0.0f}; ///< Normal direction of reflecting surfaces.
    bool m_showReflections{false}; ///< Show only the reflections.

public:
    ScreenSpaceReflectionEffect();

    [[nodiscard]] static std::shared_ptr<ScreenSpaceReflectionEffect> create();

    /** @brief Reflection amount, 0-1 (default 0.6). */
    void setStrength(float strength);
    [[nodiscard]] float strength() const;

    /** @brief Longest reflected ray in world units (default 20). */
    void setMaxDistance(float distance);
    [[nodiscard]] float maxDistance() const;

    /** @brief Ray march steps, 4-128 (default 48). More finds thin objects, costs time. */
    void setSteps(int steps);
    [[nodiscard]] int steps() const;

    /** @brief How thick surfaces are assumed to be (world units, default 0.5). */
    void setThickness(float thickness);
    [[nodiscard]] float thickness() const;

    /** @brief Blur of the reflections: 0 mirror, 1 strongly blurred (default 0). */
    void setRoughness(float roughness);
    [[nodiscard]] float roughness() const;

    /** @brief Fresnel: 0 reflects equally at all angles, 1 mostly at grazing angles (default 0.5). */
    void setFresnel(float fresnel);
    [[nodiscard]] float fresnel() const;

    /** @brief Width of the fade towards the screen edges, as a fraction of the screen (default 0.1). */
    void setEdgeFade(float fraction);
    [[nodiscard]] float edgeFade() const;

    /** @brief Largest angle in degrees between a surface's normal and the up direction (default 25). */
    void setSurfaceAngle(float degrees);
    [[nodiscard]] float surfaceAngle() const;

    /** @brief Normal direction of the reflecting surfaces in world space (default +Y). */
    void setUpDirection(const glm::vec3 &direction);
    [[nodiscard]] glm::vec3 upDirection() const;

    /** @brief Show only the reflections (for tuning). */
    void setShowReflections(bool flag);
    [[nodiscard]] bool showReflections() const;

protected:
    void doLoad() override;
    void doUpdateParams() override;
    void setupProperties() override;
};

using ScreenSpaceReflectionEffectPtr = std::shared_ptr<ScreenSpaceReflectionEffect>;

} // namespace ivf
