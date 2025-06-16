#pragma once

#include <ivf/glbase.h>
#include <ivf/shadow_map.h>
#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

#include <string>

namespace ivf {

/**
 * @class Light
 * @brief Base class for scene lights, supporting color, position, shadow, and OpenGL integration.
 *
 * The Light class provides a common interface for different types of lights in a scene.
 * It supports position, color (diffuse, specular, ambient), shadow mapping, and OpenGL resource management.
 * Derived classes can implement specific light types (directional, point, spot, etc.) and override
 * the light space matrix calculation and application logic.
 */
class Light : public GLBase {
private:
    glm::vec3 m_position;         ///< Position of the light in world space.
    glm::vec3 m_diffuseColor;     ///< Diffuse color of the light.
    glm::vec3 m_specularColor;    ///< Specular color of the light.
    glm::vec3 m_ambientColor;     ///< Ambient color of the light.
    bool m_enabled;               ///< Whether the light is enabled.
    int m_index;                  ///< Light index (for shader arrays).
    std::string m_lightArrayName; ///< Name of the light array in shaders.

    float m_shadowStrength{1.0f}; ///< Shadow strength (0.0 to 1.0).

    bool m_castsShadows{false}; ///< Whether the light casts shadows.
    ShadowMapPtr m_shadowMap;   ///< Shadow map resource.

    size_t m_shadowMapWidth{2048};  ///< Width of the shadow map.
    size_t m_shadowMapHeight{2048}; ///< Height of the shadow map.

public:
    /**
     * @brief Default constructor.
     */
    Light();

    /**
     * @brief Factory method to create a shared pointer to a Light instance.
     * @return std::shared_ptr<Light> New Light instance.
     */
    static std::shared_ptr<Light> create();

    /**
     * @brief Set the position of the light.
     * @param pos Position vector.
     */
    void setPosition(glm::vec3 pos);

    /**
     * @brief Set the diffuse color of the light.
     * @param color Diffuse color.
     */
    void setDiffuseColor(glm::vec3 color);

    /**
     * @brief Set the specular color of the light.
     * @param color Specular color.
     */
    void setSpecularColor(glm::vec3 color);

    /**
     * @brief Set the ambient color of the light.
     * @param color Ambient color.
     */
    void setAmbientColor(glm::vec3 color);

    /**
     * @brief Set the index of the light (for shader arrays).
     * @param idx Light index.
     */
    void setIndex(int idx);

    /**
     * @brief Set the name of the light array in shaders.
     * @param name Array name.
     */
    void setLightArrayName(std::string name);

    /**
     * @brief Get the name of the light array in shaders.
     * @return std::string Array name.
     */
    std::string lightArrayName();

    /**
     * @brief Get the position of the light.
     * @return glm::vec3 Position vector.
     */
    glm::vec3 position();

    /**
     * @brief Get the diffuse color of the light.
     * @return glm::vec3 Diffuse color.
     */
    glm::vec3 diffuseColor();

    /**
     * @brief Get the specular color of the light.
     * @return glm::vec3 Specular color.
     */
    glm::vec3 specularColor();

    /**
     * @brief Get the ambient color of the light.
     * @return glm::vec3 Ambient color.
     */
    glm::vec3 ambientColor();

    /**
     * @brief Enable or disable the light.
     * @param flag True to enable, false to disable.
     */
    void setEnabled(bool flag);

    /**
     * @brief Check if the light is enabled.
     * @return bool True if enabled.
     */
    bool enabled();

    /**
     * @brief Get the index of the light.
     * @return int Light index.
     */
    int index();

    /**
     * @brief Enable or disable shadow casting for the light.
     * @param flag True to enable, false to disable.
     */
    void setCastShadows(bool flag);

    /**
     * @brief Check if the light casts shadows.
     * @return bool True if the light casts shadows.
     */
    bool castsShadows() const;

    /**
     * @brief Set the size of the shadow map.
     * @param width Shadow map width.
     * @param height Shadow map height.
     */
    void setShadowMapSize(size_t width, size_t height);

    /**
     * @brief Set the shadow map resource.
     * @param shadowMap Shared pointer to the shadow map.
     */
    void setShadowMap(ShadowMapPtr shadowMap);

    /**
     * @brief Get the shadow map resource.
     * @return ShadowMapPtr Shared pointer to the shadow map.
     */
    ShadowMapPtr shadowMap();

    /**
     * @brief Clear the shadow map resource.
     */
    void clearShadowMap();

    /**
     * @brief Calculate the light space transformation matrix for shadow mapping.
     * @param sceneBBox Bounding box of the scene.
     * @return glm::mat4 Light space matrix.
     */
    virtual glm::mat4 calculateLightSpaceMatrix(BoundingBox &sceneBBox);

    /**
     * @brief Set the shadow strength (0.0 to 1.0).
     * @param strength Shadow strength.
     */
    void setShadowStrength(float strength);

    /**
     * @brief Get the shadow strength.
     * @return float Shadow strength.
     */
    float shadowStrength() const;

    /**
     * @brief Apply the light's parameters to the rendering context.
     */
    virtual void apply();
};

/**
 * @typedef LightPtr
 * @brief Shared pointer type for Light.
 */
typedef std::shared_ptr<Light> LightPtr;

} // namespace ivf
