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

public:
    /**
     * @brief Default constructor.
     */
    DirectionalLight();

    /**
     * @brief Factory method to create a shared pointer to a DirectionalLight instance.
     * @return std::shared_ptr<DirectionalLight> New DirectionalLight instance.
     */
    static std::shared_ptr<DirectionalLight> create();

    /**
     * @brief Set the direction of the light.
     * @param direction Direction vector (should be normalized).
     */
    void setDirection(glm::vec3 direction);

    /**
     * @brief Get the current direction of the light.
     * @return glm::vec3 Direction vector.
     */
    glm::vec3 direction();

    /**
     * @brief Calculate the light space transformation matrix for shadow mapping.
     * @param sceneBBox Bounding box of the scene.
     * @return glm::mat4 Light space matrix.
     */
    virtual glm::mat4 calculateLightSpaceMatrix(BoundingBox &sceneBBox) override;

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
