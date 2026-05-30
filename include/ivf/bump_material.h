#pragma once

#include <ivf/material.h>
#include <ivf/texture.h>

#include <memory>

namespace ivf {

/**
 * @class BumpMaterial
 * @brief Phong material with normal-map (bump mapping) support.
 *
 * BumpMaterial inherits from Material and follows the same shader-switching
 * pattern as PBRMaterial.  When apply() is called it switches to the "bump"
 * shader, uploads the standard Phong uniforms via the base class, and binds
 * the optional normal-map texture.  unapply() restores the "basic" shader.
 *
 * The TBN matrix used for normal-map sampling is reconstructed in the
 * fragment shader from screen-space derivatives (dFdx / dFdy), so no tangent
 * vertex attribute is required — existing Mesh VAOs work unchanged.
 *
 * Usage:
 * @code
 * // During initialisation, load both shaders once:
 * ShaderManager::instance()->loadBasicShader();
 * ShaderManager::instance()->loadBumpShader();
 *
 * auto mat = BumpMaterial::create();
 * mat->setDiffuseColor({0.8f, 0.7f, 0.6f, 1.0f});
 *
 * auto normalTex = Texture::create();
 * normalTex->load("assets/rock_normal.png");
 * mat->setNormalMap(normalTex);
 *
 * auto sphere = Sphere::create(1.0f, 64, 64);
 * sphere->setMaterial(mat);
 * @endcode
 */
class BumpMaterial : public Material {
public:
    BumpMaterial() = default;
    ~BumpMaterial() override = default;

    static std::shared_ptr<BumpMaterial> create();

    // ---- Normal map --------------------------------------------------------
    void setNormalMap(std::shared_ptr<Texture> tex) { m_normalMap = tex; }
    std::shared_ptr<Texture> normalMap() const      { return m_normalMap; }

    // ---- Material interface ------------------------------------------------
    /**
     * @brief Switch to the "bump" shader, upload Phong uniforms, and bind the
     * normal map (if set) to texture unit 1.
     */
    void apply()   override;

    /**
     * @brief Unbind the normal map and restore the "basic" shader.
     * Called automatically by Node::doPostDraw().
     */
    void unapply() override;

private:
    std::shared_ptr<Texture> m_normalMap;

    void bindMaps();
    void unbindMaps();
    void refreshManagers();
};

using BumpMaterialPtr = std::shared_ptr<BumpMaterial>;

} // namespace ivf
