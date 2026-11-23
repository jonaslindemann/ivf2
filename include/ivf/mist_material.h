#pragma once

/**
 * @file mist_material.h
 * @brief Translucent mist that fades out towards its edge.
 */

#include <ivf/blend_shader_material.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivf {

/**
 * @class MistMaterial
 * @brief Alpha-blended BlendShaderMaterial for a soft layer of mist or haze.
 *
 * Meant for flat shapes such as a Plane or Disk. The mist is densest at the
 * node's origin and fades out with the distance from it, measured in the
 * node's own coordinates, so the layer can be moved, rotated and scaled freely.
 * Beyond the radius it is fully transparent, so the shape's edges never show.
 *
 * Mist nodes are drawn after the opaque ones (Transparent render layer).
 *
 * @code
 * auto mist = MistMaterial::create({0.5f, 0.45f, 0.7f}, 0.35f, 12.0f);
 * auto layer = Plane::create(24.0, 24.0);
 * layer->setMaterial(mist);
 * @endcode
 */
class MistMaterial : public BlendShaderMaterial {
public:
    /**
     * @brief Constructor. Use create() so the shader program is set up.
     */
    MistMaterial(glm::vec3 color = glm::vec3(1.0f), float opacity = 0.5f, float radius = 1.0f);

    /**
     * @brief Create a mist material.
     * @param color   Mist color.
     * @param opacity Opacity at the center [0, 1].
     * @param radius  Distance from the node's origin where the mist has faded out.
     * @return The material, or nullptr if the mist shader failed to compile.
     */
    [[nodiscard]] static std::shared_ptr<MistMaterial> create(glm::vec3 color = glm::vec3(1.0f), float opacity = 0.5f,
                                                float radius = 1.0f);

    /** @brief Mist color. */
    void setColor(glm::vec3 color);
    [[nodiscard]] glm::vec3 color() const;

    /** @brief Opacity at the center [0, 1]. */
    void setOpacity(float opacity);
    [[nodiscard]] float opacity() const;

    /** @brief Distance from the node's origin where the mist has faded out (local units). */
    void setRadius(float radius);
    [[nodiscard]] float radius() const;

    /**
     * @brief Part of the radius over which the mist fades [0, 1].
     *
     * 1 fades all the way from the center, small values give a sharp edge (default 0.8).
     */
    void setSoftness(float softness);
    [[nodiscard]] float softness() const;

private:
    glm::vec3 m_color;
    float m_opacity;
    float m_radius;
    float m_softness{0.8f};
};

using MistMaterialPtr = std::shared_ptr<MistMaterial>;

} // namespace ivf
