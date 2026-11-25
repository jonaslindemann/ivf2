#pragma once

#include <ivf/material.h>
#include <ivf/texture.h>
#include <ivf/cubemap.h>
#include <ivf/environment_map.h>

#include <glm/glm.hpp>
#include <memory>

namespace ivf {

/**
 * @class PBRMaterial
 * @brief Physically-based rendering material using Cook-Torrance BRDF.
 *
 * PBRMaterial inherits from Material so it can be assigned to any Node via
 * setMaterial(). When apply() is called during the draw chain it switches to
 * the "pbr" shader, uploads PBR uniforms and optional texture maps, and
 * re-caches manager uniform IDs for the PBR program.  unapply() (called
 * automatically by Node::doPostDraw() after the node is drawn) restores the
 * "basic" shader and refreshes manager IDs back to the basic program.
 *
 * Usage:
 * @code
 * auto mat = PBRMaterial::create();
 * mat->setAlbedo({1.0f, 0.3f, 0.1f, 1.0f});
 * mat->setRoughness(0.4f);
 * mat->setMetallic(0.8f);
 * auto sphere = Sphere::create(1.0f, 32, 32);
 * sphere->setMaterial(mat);
 * @endcode
 */
class PBRMaterial : public Material {
public:
    /// How the albedo's alpha is used (as glTF's alphaMode).
    enum class AlphaMode {
        Opaque, ///< Alpha is ignored: fully opaque.
        Mask,   ///< Pixels with alpha below alphaCutoff() are discarded, the rest are opaque.
        Blend   ///< Alpha blending: drawn after opaque objects, back to front.
    };

    PBRMaterial();
    ~PBRMaterial() override = default;

    [[nodiscard]] static std::shared_ptr<PBRMaterial> create();

    // ---- Scalar properties -------------------------------------------------
    void setAlbedo(glm::vec4 color)   { m_albedo = color; }
    void setRoughness(float r)        { m_roughness = r; }
    void setMetallic(float m)         { m_metallic = m; }
    void setEmissive(glm::vec3 color) { m_emissive = color; }
    void setAO(float ao)              { m_ao = ao; }

    glm::vec4 albedo()    const { return m_albedo; }
    float     roughness() const { return m_roughness; }
    float     metallic()  const { return m_metallic; }
    glm::vec3 emissive()  const { return m_emissive; }
    float     ao()        const { return m_ao; }

    // ---- Texture maps ------------------------------------------------------
    void setAlbedoMap(std::shared_ptr<Texture> tex)    { m_albedoMap    = tex; }
    void setNormalMap(std::shared_ptr<Texture> tex)    { m_normalMap    = tex; }
    void setRoughnessMap(std::shared_ptr<Texture> tex) { m_roughnessMap = tex; }
    void setMetallicMap(std::shared_ptr<Texture> tex)  { m_metallicMap  = tex; }
    void setAOMap(std::shared_ptr<Texture> tex)        { m_aoMap        = tex; }

    /**
     * @brief Packed roughness (green) and metallic (blue) texture, as in glTF. Multiplies
     *        roughness() and metallic(); the separate maps take precedence if set.
     */
    void setMetallicRoughnessMap(std::shared_ptr<Texture> tex) { m_metallicRoughnessMap = tex; }

    /** @brief Emissive colour texture (sRGB), multiplied by emissive(). */
    void setEmissiveMap(std::shared_ptr<Texture> tex) { m_emissiveMap = tex; }

    [[nodiscard]] std::shared_ptr<Texture> albedoMap() const { return m_albedoMap; }
    [[nodiscard]] std::shared_ptr<Texture> normalMap() const { return m_normalMap; }
    [[nodiscard]] std::shared_ptr<Texture> metallicRoughnessMap() const { return m_metallicRoughnessMap; }
    [[nodiscard]] std::shared_ptr<Texture> emissiveMap() const { return m_emissiveMap; }
    [[nodiscard]] std::shared_ptr<Texture> aoMap() const { return m_aoMap; }

    // ---- Alpha and vertex colours ------------------------------------------
    /** @brief How the albedo's alpha is used (default AlphaMode::Opaque). */
    void setAlphaMode(AlphaMode mode) { m_alphaMode = mode; }
    [[nodiscard]] AlphaMode alphaMode() const { return m_alphaMode; }

    /** @brief Alpha below which AlphaMode::Mask discards a pixel (default 0.5). */
    void setAlphaCutoff(float cutoff) { m_alphaCutoff = cutoff; }
    [[nodiscard]] float alphaCutoff() const { return m_alphaCutoff; }

    /** @brief Multiply the albedo by the mesh's vertex colours (default off). */
    void setUseVertexColors(bool flag) { m_useVertexColors = flag; }
    [[nodiscard]] bool useVertexColors() const { return m_useVertexColors; }
    void setEnvCubemap(std::shared_ptr<Cubemap> cm)    { m_envCubemap   = cm; }

    // ---- Image-based lighting ----------------------------------------------
    /**
     * @brief Light the material with an HDR environment (diffuse irradiance and
     *        roughness-dependent reflections; see EnvironmentMap). Replaces the plain
     *        reflection of setEnvCubemap() while set.
     */
    void setEnvironment(EnvironmentMapPtr environment) { m_environment = std::move(environment); }
    [[nodiscard]] EnvironmentMapPtr environment() const { return m_environment; }

    /** @brief Scale of the environment's light (default 1). */
    void setEnvironmentIntensity(float intensity) { m_environmentIntensity = intensity; }
    [[nodiscard]] float environmentIntensity() const { return m_environmentIntensity; }

    /**
     * @brief Tone map (Reinhard) and gamma-encode in the shader (default on).
     *
     * Turn it off when the window tone maps (GLFWSceneWindow::setToneMapping() with
     * setGamma(2.2f)): the material then outputs linear HDR values.
     */
    void setToneMapping(bool enabled) { m_toneMapping = enabled; }
    [[nodiscard]] bool toneMapping() const { return m_toneMapping; }

    // ---- Material interface ------------------------------------------------
    /**
     * @brief Switch to the "pbr" shader, refresh manager cached IDs, and upload
     * all PBR uniforms and texture maps to the GPU.
     */
    void apply()   override;

    /**
     * @brief Restore the "basic" shader and refresh manager cached IDs back to
     * the basic program.  Called automatically by Node::doPostDraw().
     */
    void unapply() override;

    /** @brief Transparent for AlphaMode::Blend, else Opaque. */
    [[nodiscard]] RenderLayer renderLayer() const override;

private:
    glm::vec4 m_albedo{1.0f, 1.0f, 1.0f, 1.0f};
    float     m_roughness{0.5f};
    float     m_metallic{0.0f};
    glm::vec3 m_emissive{0.0f, 0.0f, 0.0f};
    float     m_ao{1.0f};

    std::shared_ptr<Texture> m_albedoMap;
    std::shared_ptr<Texture> m_normalMap;
    std::shared_ptr<Texture> m_roughnessMap;
    std::shared_ptr<Texture> m_metallicMap;
    std::shared_ptr<Texture> m_aoMap;
    std::shared_ptr<Texture> m_metallicRoughnessMap;
    std::shared_ptr<Texture> m_emissiveMap;
    AlphaMode m_alphaMode{AlphaMode::Opaque};
    float m_alphaCutoff{0.5f};
    bool m_useVertexColors{false};
std::shared_ptr<Cubemap> m_envCubemap;
    EnvironmentMapPtr m_environment;
    float m_environmentIntensity{1.0f};
    bool m_toneMapping{true};

    void bindMaps();
    void unbindMaps();
    void refreshManagers();
};

using PBRMaterialPtr = std::shared_ptr<PBRMaterial>;

} // namespace ivf
