#pragma once

/**
 * @file glow_material.h
 * @brief Glowing material: a bright core fading towards the silhouette.
 */

#include <ivf/blend_shader_material.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivf {

/**
 * @class GlowMaterial
 * @brief BlendShaderMaterial that makes a shape look like a ball of light.
 *
 * The surface is brightest where it faces the viewer and fades towards the
 * silhouette, so a Sphere reads as a glowing orb rather than a lit ball. Lights
 * are ignored. Additive blending (the default) lets overlapping glows add up;
 * combine it with MipBloomEffect for a soft halo.
 *
 * All glow materials share one shader program, so a material per color is cheap.
 * Add glowing nodes after the opaque ones.
 *
 * @code
 * auto glow = GlowMaterial::create({1.0f, 0.6f, 0.2f}, 1.5f);
 * auto orb = Sphere::create(0.3);
 * orb->setMaterial(glow);
 * @endcode
 */
class GlowMaterial : public BlendShaderMaterial {
public:
    /**
     * @brief Constructor. Use create() so the shader program is set up.
     */
    GlowMaterial(glm::vec3 color = glm::vec3(1.0f), float brightness = 1.0f, BlendMode mode = BlendMode::Additive);

    /**
     * @brief Create a glow material.
     * @param color      Glow color.
     * @param brightness Multiplier for the color; values above 1 feed bloom effects.
     * @param mode       Blend mode; Alpha gives a translucent glow with the core opaque.
     * @return The material, or nullptr if the glow shader failed to compile.
     */
    static std::shared_ptr<GlowMaterial> create(glm::vec3 color = glm::vec3(1.0f), float brightness = 1.0f,
                                                BlendMode mode = BlendMode::Additive);

    /** @brief Glow color. */
    void setColor(glm::vec3 color);
    [[nodiscard]] glm::vec3 color() const;

    /** @brief Multiplier for the color. */
    void setBrightness(float brightness);
    [[nodiscard]] float brightness() const;

    /**
     * @brief How quickly the glow fades towards the silhouette.
     *
     * 1 fades evenly, higher values give a smaller, sharper core (default 1.5).
     */
    void setFalloff(float falloff);
    [[nodiscard]] float falloff() const;

private:
    glm::vec3 m_color;
    float m_brightness;
    float m_falloff{1.5f};
};

using GlowMaterialPtr = std::shared_ptr<GlowMaterial>;

} // namespace ivf
