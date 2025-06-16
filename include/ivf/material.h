#pragma once

#include <ivf/glbase.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class Material
 * @brief Represents material properties for rendering, including color, lighting, and texture options.
 *
 * The Material class encapsulates the properties used for rendering surfaces, such as diffuse,
 * specular, and ambient colors, shininess, alpha (opacity), and flags for lighting, texture, and
 * vertex color usage. It provides methods to configure these properties and apply them to the
 * rendering context or shader.
 */
class Material : public GLBase {
private:
    bool m_useVertexColor;     ///< Whether to use per-vertex color.
    bool m_useLighting;        ///< Whether to use lighting.
    bool m_useTexture;         ///< Whether to use texture mapping.
    glm::vec4 m_diffuseColor;  ///< Diffuse color (RGBA).
    glm::vec4 m_specularColor; ///< Specular color (RGBA).
    glm::vec4 m_ambientColor;  ///< Ambient color (RGBA).
    float m_alpha;             ///< Alpha (opacity) value.
    float m_shininess;         ///< Shininess (specular exponent).

public:
    /**
     * @brief Default constructor.
     */
    Material();

    /**
     * @brief Factory method to create a shared pointer to a Material instance.
     * @return std::shared_ptr<Material> New Material instance.
     */
    static std::shared_ptr<Material> create();

    /**
     * @brief Set the diffuse color.
     * @param color Diffuse color (RGBA).
     */
    void setDiffuseColor(glm::vec4 color);

    /**
     * @brief Set the specular color.
     * @param color Specular color (RGBA).
     */
    void setSpecularColor(glm::vec4 color);

    /**
     * @brief Set the ambient color.
     * @param color Ambient color (RGBA).
     */
    void setAmbientColor(glm::vec4 color);

    /**
     * @brief Enable or disable texture mapping.
     * @param flag True to enable, false to disable.
     */
    void setUseTexture(bool flag);

    /**
     * @brief Enable or disable per-vertex color.
     * @param flag True to enable, false to disable.
     */
    void setUseVertexColor(bool flag);

    /**
     * @brief Set the shininess (specular exponent).
     * @param shininess Shininess value.
     */
    void setShininess(float shininess);

    /**
     * @brief Enable or disable lighting.
     * @param flag True to enable, false to disable.
     */
    void setUseLighting(bool flag);

    /**
     * @brief Set the alpha (opacity) value.
     * @param alpha Alpha value.
     */
    void setAlpha(float alpha);

    /**
     * @brief Check if lighting is enabled.
     * @return bool True if lighting is enabled.
     */
    bool useLighting() const;

    /**
     * @brief Check if texture mapping is enabled.
     * @return bool True if texture mapping is enabled.
     */
    bool useTexture() const;

    /**
     * @brief Check if per-vertex color is enabled.
     * @return bool True if per-vertex color is enabled.
     */
    bool useVertexColor() const;

    /**
     * @brief Get the diffuse color.
     * @return glm::vec4 Diffuse color (RGBA).
     */
    glm::vec4 diffuseColor() const;

    /**
     * @brief Get the specular color.
     * @return glm::vec4 Specular color (RGBA).
     */
    glm::vec4 specularColor() const;

    /**
     * @brief Get the ambient color.
     * @return glm::vec4 Ambient color (RGBA).
     */
    glm::vec4 ambientColor() const;

    /**
     * @brief Get the shininess value.
     * @return float Shininess.
     */
    float shininess() const;

    /**
     * @brief Get the alpha (opacity) value.
     * @return float Alpha value.
     */
    float alpha() const;

    /**
     * @brief Apply the material properties to the rendering context or shader.
     */
    void apply();
};

/**
 * @typedef MaterialPtr
 * @brief Shared pointer type for Material.
 */
typedef std::shared_ptr<Material> MaterialPtr;

} // namespace ivf
