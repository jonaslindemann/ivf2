#pragma once

#include <ivf/glbase.h>

#include <glm/glm.hpp>

namespace ivf {

struct MaterialProps {
    glm::vec4 diffuseColor{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 specularColor{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 ambientColor{0.2f, 0.2f, 0.2f, 1.0f};
    float alpha{1.0f};
    float shininess{32.0f};
    bool useLighting{true};
    bool useTexture{false};
    bool useVertexColor{false};
};

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
    Material(MaterialProps props = MaterialProps{});

    /**
     * @brief Factory method to create a shared pointer to a Material instance.
     * @return std::shared_ptr<Material> New Material instance.
     */
    static std::shared_ptr<Material> create(MaterialProps props = MaterialProps{});

    /**
     * @brief Set the diffuse color.
     * @param color Diffuse color (RGBA).
     */
    inline void setDiffuseColor(glm::vec4 color) noexcept { m_diffuseColor = color; }

    /**
     * @brief Set the specular color.
     * @param color Specular color (RGBA).
     */
    inline void setSpecularColor(glm::vec4 color) noexcept { m_specularColor = color; }

    /**
     * @brief Set the ambient color.
     * @param color Ambient color (RGBA).
     */
    inline void setAmbientColor(glm::vec4 color) noexcept { m_ambientColor = color; }

    /**
     * @brief Enable or disable texture mapping.
     * @param flag True to enable, false to disable.
     */
    inline void setUseTexture(bool flag) noexcept { m_useTexture = flag; }

    /**
     * @brief Enable or disable per-vertex color.
     * @param flag True to enable, false to disable.
     */
    inline void setUseVertexColor(bool flag) noexcept { m_useVertexColor = flag; }

    /**
     * @brief Set the shininess (specular exponent).
     * @param shininess Shininess value.
     */
    inline void setShininess(float shininess) noexcept { m_shininess = shininess; }

    /**
     * @brief Enable or disable lighting.
     * @param flag True to enable, false to disable.
     */
    inline void setUseLighting(bool flag) noexcept { m_useLighting = flag; }

    /**
     * @brief Set the alpha (opacity) value.
     * @param alpha Alpha value.
     */
    inline void setAlpha(float alpha) noexcept { m_alpha = alpha; }

    /**
     * @brief Check if lighting is enabled.
     * @return bool True if lighting is enabled.
     */
    [[nodiscard]] inline bool useLighting() const noexcept { return m_useLighting; }

    /**
     * @brief Check if texture mapping is enabled.
     * @return bool True if texture mapping is enabled.
     */
    [[nodiscard]] inline bool useTexture() const noexcept { return m_useTexture; }

    /**
     * @brief Check if per-vertex color is enabled.
     * @return bool True if per-vertex color is enabled.
     */
    [[nodiscard]] inline bool useVertexColor() const noexcept { return m_useVertexColor; }

    /**
     * @brief Get the diffuse color.
     * @return glm::vec4 Diffuse color (RGBA).
     */
    [[nodiscard]] inline glm::vec4 diffuseColor() const noexcept { return m_diffuseColor; }

    /**
     * @brief Get the specular color.
     * @return glm::vec4 Specular color (RGBA).
     */
    [[nodiscard]] inline glm::vec4 specularColor() const noexcept { return m_specularColor; }

    /**
     * @brief Get the ambient color.
     * @return glm::vec4 Ambient color (RGBA).
     */
    [[nodiscard]] inline glm::vec4 ambientColor() const noexcept { return m_ambientColor; }

    /**
     * @brief Get the shininess value.
     * @return float Shininess.
     */
    [[nodiscard]] inline float shininess() const noexcept { return m_shininess; }

    /**
     * @brief Get the alpha (opacity) value.
     * @return float Alpha value.
     */
    [[nodiscard]] inline float alpha() const noexcept { return m_alpha; }

    /**
     * @brief Apply the material properties to the rendering context or shader.
     */
    void apply();
};

/**
 * @typedef MaterialPtr
 * @brief Shared pointer type for Material.
 */
using MaterialPtr = std::shared_ptr<Material>;

} // namespace ivf
