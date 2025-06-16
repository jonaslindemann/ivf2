#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <ivf/dir_light.h>
#include <ivf/point_light.h>
#include <ivf/spot_light.h>
#include <ivf/composite_node.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class LightManager
 * @brief Singleton class for managing scene lights and lighting state.
 *
 * The LightManager class provides a global interface for adding, configuring, and managing
 * all lights in a scene, including point, directional, and spot lights. It also manages
 * lighting state, shadow mapping, and shader uniform integration. This class is implemented
 * as a singleton.
 */
class LightManager {
private:
    bool m_useLighting;     ///< Whether lighting is enabled.
    bool m_useVertexColors; ///< Whether vertex colors are used.
    bool m_savedState;      ///< Saved state for lighting.

    GLint m_useLightingId;     ///< Shader uniform location for lighting enable.
    GLint m_useVertexColorsId; ///< Shader uniform location for vertex color enable.
    GLint m_useTextureId;      ///< Shader uniform location for texture enable.

    GLint m_pointLightCountId;       ///< Shader uniform location for point light count.
    GLint m_directionalLightCountId; ///< Shader uniform location for directional light count.
    GLint m_spotLightCountId;        ///< Shader uniform location for spot light count.

    GLint m_diffuseColorId;  ///< Shader uniform location for diffuse color.
    GLint m_specularColorId; ///< Shader uniform location for specular color.
    GLint m_ambientColorId;  ///< Shader uniform location for ambient color.
    GLint m_shininessId;     ///< Shader uniform location for shininess.
    GLint m_alphaId;         ///< Shader uniform location for alpha.

    std::vector<PointLightPtr> m_pointLights;     ///< List of point lights.
    std::vector<DirectionalLightPtr> m_dirLights; ///< List of directional lights.
    std::vector<SpotLightPtr> m_spotLights;       ///< List of spot lights.

    // Shadow mapping
    bool m_useShadows{false};   ///< Whether shadow mapping is enabled.
    GLint m_useShadowsId;       ///< Shader uniform location for shadow enable.
    GLint m_shadowMapId;        ///< Shader uniform location for shadow map.
    GLint m_lightSpaceMatrixId; ///< Shader uniform location for light space matrix.
    bool m_autoCalcBBox{true};  ///< Whether to auto-calculate scene bounding box.
    BoundingBox m_sceneBBox;    ///< Scene bounding box for shadow mapping.
    int m_debugShadow{0};       ///< Debug flag for shadow rendering.

    LightManager();                  ///< Private constructor for singleton pattern.
    static LightManager *m_instance; ///< Singleton instance pointer.

public:
    /**
     * @brief Get the singleton instance of the LightManager.
     * @return LightManager* Pointer to the singleton instance.
     */
    static LightManager *instance()
    {
        if (!m_instance)
            m_instance = new LightManager();

        return m_instance;
    }

    /**
     * @brief Create the singleton instance of the LightManager (if not already created).
     * @return LightManager* Pointer to the singleton instance.
     */
    static LightManager *create()
    {
        return instance();
    }

    /**
     * @brief Destroy the singleton instance and release all resources.
     */
    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    /**
     * @brief Add a new point light to the scene.
     * @return PointLightPtr Shared pointer to the new point light.
     */
    PointLightPtr addPointLight();

    /**
     * @brief Add a new directional light to the scene.
     * @return DirectionalLightPtr Shared pointer to the new directional light.
     */
    DirectionalLightPtr addDirectionalLight();

    /**
     * @brief Add a new spot light to the scene.
     * @return SpotLightPtr Shared pointer to the new spot light.
     */
    SpotLightPtr addSpotLight();

    /**
     * @brief Get a point light by index.
     * @param idx Index of the point light.
     * @return PointLightPtr Shared pointer to the point light.
     */
    PointLightPtr getPointLight(int idx);

    /**
     * @brief Get a directional light by index.
     * @param idx Index of the directional light.
     * @return DirectionalLightPtr Shared pointer to the directional light.
     */
    DirectionalLightPtr getDirectionalLight(int idx);

    /**
     * @brief Get a spot light by index.
     * @param idx Index of the spot light.
     * @return SpotLightPtr Shared pointer to the spot light.
     */
    SpotLightPtr getSpotLight(int idx);

    /**
     * @brief Get the number of point lights in the scene.
     * @return size_t Number of point lights.
     */
    size_t pointLightCount();

    /**
     * @brief Get the number of directional lights in the scene.
     * @return size_t Number of directional lights.
     */
    size_t dirLightCount();

    /**
     * @brief Get the number of spot lights in the scene.
     * @return size_t Number of spot lights.
     */
    size_t spotLightCount();

    /**
     * @brief Remove all point lights from the scene.
     */
    void clearPointLights();

    /**
     * @brief Remove all directional lights from the scene.
     */
    void clearDirectionalLights();

    /**
     * @brief Remove all spot lights from the scene.
     */
    void clearSpotLights();

    /**
     * @brief Remove all lights from the scene.
     */
    void clearLights();

    /**
     * @brief Enable or disable lighting.
     * @param flag True to enable, false to disable.
     */
    void setUseLighting(bool flag);

    /**
     * @brief Enable or disable vertex colors.
     * @param flag True to enable, false to disable.
     */
    void setUseVertexColors(bool flag);

    /**
     * @brief Check if lighting is enabled.
     * @return bool True if lighting is enabled.
     */
    bool useLighting();

    /**
     * @brief Enable lighting.
     */
    void enableLighting();

    /**
     * @brief Disable lighting.
     */
    void disableLighting();

    /**
     * @brief Set up default material colors.
     */
    void setupDefaultColors();

    /**
     * @brief Apply lighting and material parameters to the current shader.
     */
    void apply();

    /**
     * @brief Render shadow maps for all lights in the scene.
     * @param scene Shared pointer to the scene's composite node.
     */
    void renderShadowMaps(CompositeNodePtr scene);

    /**
     * @brief Set the diffuse color for materials.
     * @param color Diffuse color (glm::vec3).
     */
    void setDiffuseColor(glm::vec3 color);

    /**
     * @brief Set the diffuse color for materials.
     * @param color Diffuse color (glm::vec4).
     */
    void setDiffuseColor(glm::vec4 color);

    /**
     * @brief Set the diffuse color for materials.
     * @param red Red component.
     * @param green Green component.
     * @param blue Blue component.
     * @param alpha Alpha component (default 1.0f).
     */
    void setDiffuseColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

    /**
     * @brief Set the specular color for materials.
     * @param color Specular color (glm::vec3).
     */
    void setSpecularColor(glm::vec3 color);

    /**
     * @brief Set the specular color for materials.
     * @param color Specular color (glm::vec4).
     */
    void setSpecularColor(glm::vec4 color);

    /**
     * @brief Set the specular color for materials.
     * @param red Red component.
     * @param green Green component.
     * @param blue Blue component.
     * @param alpha Alpha component (default 1.0f).
     */
    void setSpecularColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

    /**
     * @brief Set the ambient color for materials.
     * @param color Ambient color (glm::vec3).
     */
    void setAmbientColor(glm::vec3 color);

    /**
     * @brief Set the ambient color for materials.
     * @param color Ambient color (glm::vec4).
     */
    void setAmbientColor(glm::vec4 color);

    /**
     * @brief Set the ambient color for materials.
     * @param red Red component.
     * @param green Green component.
     * @param blue Blue component.
     * @param alpha Alpha component (default 1.0f).
     */
    void setAmbientColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

    /**
     * @brief Set the shininess for materials.
     * @param shininess Shininess value.
     */
    void setShininess(float shininess);

    /**
     * @brief Set the alpha (opacity) for materials.
     * @param alpha Alpha value.
     */
    void setAlpha(float alpha);

    /**
     * @brief Enable or disable shadow mapping.
     * @param flag True to enable, false to disable.
     */
    void setUseShadows(bool flag);

    /**
     * @brief Check if shadow mapping is enabled.
     * @return bool True if shadow mapping is enabled.
     */
    bool useShadows() const;

    /**
     * @brief Enable or disable automatic scene bounding box calculation.
     * @param flag True to enable, false to disable.
     */
    void setAutoCalcBBox(bool flag);

    /**
     * @brief Check if automatic scene bounding box calculation is enabled.
     * @return bool True if enabled.
     */
    bool autoCalcBBox() const;

    /**
     * @brief Set the debug flag for shadow rendering.
     * @param flag Debug flag value.
     */
    void setDebugShadow(int flag);

    /**
     * @brief Get the current debug flag for shadow rendering.
     * @return int Debug flag value.
     */
    int debugShadow() const;

    /**
     * @brief Set the scene bounding box for shadow mapping.
     * @param bbox Reference to the bounding box.
     */
    void setSceneBoundingBox(BoundingBox &bbox);

    /**
     * @brief Set the scene bounding box for shadow mapping.
     * @param min Minimum corner of the bounding box.
     * @param max Maximum corner of the bounding box.
     */
    void setSceneBoundingBox(glm::vec3 min, glm::vec3 max);

    /**
     * @brief Get the current scene bounding box.
     * @return BoundingBox The scene bounding box.
     */
    BoundingBox sceneBoundingBox() const;

    /**
     * @brief Get a reference to the current scene bounding box.
     * @return BoundingBox& Reference to the scene bounding box.
     */
    BoundingBox &sceneBoundingBox();

    /**
     * @brief Save the current lighting state.
     */
    void saveState();

    /**
     * @brief Restore the previously saved lighting state.
     */
    void restoreState();
};

/**
 * @typedef LightManagerPtr
 * @brief Pointer type for LightManager singleton.
 */
typedef LightManager *LightManagerPtr;
}; // namespace ivf
