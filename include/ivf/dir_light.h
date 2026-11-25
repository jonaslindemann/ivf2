#pragma once

#include <ivf/light.h>
#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class DirectionalLight
 * @brief Represents a directional light source for scene illumination and shadow mapping.
 *
 * The DirectionalLight class models a light source with parallel rays, such as sunlight.
 * It provides methods to set the light direction, compute the light space matrix for shadow mapping,
 * and apply the light's parameters to the rendering context. Inherits from Light.
 */
class DirectionalLight : public Light {
private:
    glm::vec3 m_direction; ///< Direction vector of the light.
    int m_shadowCascades{1};          ///< Number of shadow cascades (1 = one map fitted to the scene).
    float m_shadowDistance{0.0f};     ///< View distance covered by the cascades (0 = automatic).
    float m_cascadeSplitLambda{0.75f}; ///< Logarithmic (1) or uniform (0) cascade splits.

public:
    /**
     * @brief Default constructor.
     */
    DirectionalLight();

    /**
     * @brief Factory method to create a shared pointer to a DirectionalLight instance.
     * @return std::shared_ptr<DirectionalLight> New DirectionalLight instance.
     */
    [[nodiscard]] static std::shared_ptr<DirectionalLight> create();

    /**
     * @brief Set the direction of the light.
     * @param direction Direction vector (should be normalized).
     */
    void setDirection(glm::vec3 direction);

    /**
     * @brief Get the current direction of the light.
     * @return glm::vec3 Direction vector.
     */
    glm::vec3 direction() const;

    /**
     * @brief Calculate the light space transformation matrix for shadow mapping.
     * @param sceneBBox Bounding box of the scene.
     * @return glm::mat4 Light space matrix.
     */
    virtual glm::mat4 calculateLightSpaceMatrix(BoundingBox &sceneBBox) override;

    /**
     * @brief Number of shadow cascades, 1-4 (default 1).
     *
     * With one, the shadow map covers the whole scene (LightManager's scene bounding box).
     * With more, cascaded shadow maps split the camera's view range: nearby shadows get
     * a small, sharp map, distant ones larger, coarser ones. The cascades share the map
     * (setShadowMapSize()) as tiles. Shadows end at setShadowDistance().
     */
    void setShadowCascades(int count);

    /** @brief Number of shadow cascades. */
    [[nodiscard]] int shadowCascades() const;

    /**
     * @brief View distance covered by the cascades (0, the default: up to the farthest
     * point of the scene, at most the camera's far plane). Beyond it there are no shadows.
     */
    void setShadowDistance(float distance);

    /** @brief View distance covered by the cascades (0 = automatic). */
    [[nodiscard]] float shadowDistance() const;

    /**
     * @brief Placement of the cascade splits: 1 logarithmic (more resolution near the
     * camera), 0 uniform; default 0.75.
     */
    void setCascadeSplitLambda(float lambda);

    /** @brief Placement of the cascade splits. */
    [[nodiscard]] float cascadeSplitLambda() const;

    /**
     * @brief Apply the light's parameters to the rendering context.
     */
    void apply() override;
};

/**
 * @typedef DirectionalLightPtr
 * @brief Shared pointer type for DirectionalLight.
 */
typedef std::shared_ptr<DirectionalLight> DirectionalLightPtr;

} // namespace ivf
