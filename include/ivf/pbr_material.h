#pragma once

#include <ivf/material.h>
#include <ivf/texture.h>
#include <ivf/cubemap.h>

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
    PBRMaterial();
    ~PBRMaterial() override = default;

    static std::shared_ptr<PBRMaterial> create();

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
    void setEnvCubemap(std::shared_ptr<Cubemap> cm)    { m_envCubemap   = cm; }

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
    std::shared_ptr<Cubemap> m_envCubemap;

    void bindMaps();
    void unbindMaps();
    void refreshManagers();
};

using PBRMaterialPtr = std::shared_ptr<PBRMaterial>;

} // namespace ivf
