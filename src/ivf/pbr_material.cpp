#include <ivf/pbr_material.h>
#include <ivf/render_context.h>

#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>
#include <ivf/light_manager.h>
#include <ivf/selection_manager.h>

#include <glad/glad.h>

using namespace ivf;

namespace {

constexpr GLuint kEnvCubemapUnit = 15;

void bindFallbackTexture2D(GLuint unit)
{
    // The fallback texture belongs to the current RenderContext (one per GL context).
    const GLuint texture = RenderContext::currentOrFallback()->fallbackTexture2D();
    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(activeTexture);
}

void bindFallbackCubemap(GLuint unit)
{
    // The fallback texture belongs to the current RenderContext (one per GL context).
    const GLuint texture = RenderContext::currentOrFallback()->fallbackCubemap();
    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glActiveTexture(activeTexture);
}

}

PBRMaterial::PBRMaterial()
{
    setUseLighting(true);
}

std::shared_ptr<PBRMaterial> PBRMaterial::create()
{
    return std::make_shared<PBRMaterial>();
}

void PBRMaterial::apply()
{
    // Switch to the PBR shader program
    ShaderManager::instance()->setCurrentProgram("pbr");

    // Re-cache all manager uniform IDs for the PBR program and upload current matrices
    refreshManagers();

    // Upload PBR scalar uniforms
    auto prog = ShaderManager::instance()->currentProgram();
    prog->uniformVec4("pbr.albedo",    m_albedo);
    prog->uniformFloat("pbr.roughness", m_roughness);
    prog->uniformFloat("pbr.metallic",  m_metallic);
    prog->uniformVec3("pbr.emissive",  m_emissive);
    prog->uniformFloat("pbr.ao",        m_ao);

    // Bind optional texture maps
    bindMaps();

    // Apply lights through the now-refreshed LightManager
    LightManager::instance()->apply();
}

void PBRMaterial::unapply()
{
    unbindMaps();

    // Restore the "basic" shader and re-cache manager IDs for the basic program
    ShaderManager::instance()->setCurrentProgram("basic");
    refreshManagers();
}

void PBRMaterial::refreshManagers()
{
    TransformManager::instance()->refreshForProgram();
    LightManager::instance()->refreshForProgram();
    SelectionManager::instance()->refreshForProgram();
}

RenderLayer PBRMaterial::renderLayer() const
{
    return m_alphaMode == AlphaMode::Blend ? RenderLayer::Transparent : RenderLayer::Opaque;
}

void PBRMaterial::bindMaps()
{
    auto prog = ShaderManager::instance()->currentProgram();

    if (m_albedoMap) {
        m_albedoMap->setTexUnit(0);
        m_albedoMap->bind();
        prog->uniformBool("useAlbedoMap", true);
    } else {
        bindFallbackTexture2D(0);
        prog->uniformBool("useAlbedoMap", false);
    }
    prog->uniformInt("albedoMap", 0);

    if (m_normalMap) {
        m_normalMap->setTexUnit(1);
        m_normalMap->bind();
        prog->uniformBool("useNormalMap", true);
    } else {
        bindFallbackTexture2D(1);
        prog->uniformBool("useNormalMap", false);
    }
    prog->uniformInt("normalMap", 1);

    if (m_roughnessMap) {
        m_roughnessMap->setTexUnit(2);
        m_roughnessMap->bind();
        prog->uniformBool("useRoughnessMap", true);
    } else {
        bindFallbackTexture2D(2);
        prog->uniformBool("useRoughnessMap", false);
    }
    prog->uniformInt("roughnessMap", 2);

    if (m_metallicMap) {
        m_metallicMap->setTexUnit(3);
        m_metallicMap->bind();
        prog->uniformBool("useMetallicMap", true);
    } else {
        bindFallbackTexture2D(3);
        prog->uniformBool("useMetallicMap", false);
    }
    prog->uniformInt("metallicMap", 3);

    if (m_aoMap) {
        m_aoMap->setTexUnit(4);
        m_aoMap->bind();
        prog->uniformBool("useAOMap", true);
    } else {
        bindFallbackTexture2D(4);
        prog->uniformBool("useAOMap", false);
    }
    prog->uniformInt("aoMap", 4);

    if (m_metallicRoughnessMap) {
        m_metallicRoughnessMap->setTexUnit(5);
        m_metallicRoughnessMap->bind();
        prog->uniformBool("useMetallicRoughnessMap", true);
    } else {
        bindFallbackTexture2D(5);
        prog->uniformBool("useMetallicRoughnessMap", false);
    }
    prog->uniformInt("metallicRoughnessMap", 5);

    if (m_emissiveMap) {
        m_emissiveMap->setTexUnit(6);
        m_emissiveMap->bind();
        prog->uniformBool("useEmissiveMap", true);
    } else {
        bindFallbackTexture2D(6);
        prog->uniformBool("useEmissiveMap", false);
    }
    prog->uniformInt("emissiveMap", 6);

    prog->uniformInt("pbrAlphaMode", static_cast<int>(m_alphaMode));
    prog->uniformFloat("pbrAlphaCutoff", m_alphaCutoff);
    prog->uniformBool("pbrVertexColors", m_useVertexColors);

    if (m_envCubemap) {
        m_envCubemap->bind(kEnvCubemapUnit);
        prog->uniformBool("useEnvCubemap", true);
    } else {
        bindFallbackCubemap(kEnvCubemapUnit);
        prog->uniformBool("useEnvCubemap", false);
    }
    prog->uniformInt("envCubemap", kEnvCubemapUnit);

    // Image-based lighting
    const bool ibl = m_environment && m_environment->isValid();
    if (ibl)
    {
        m_environment->bind();
        prog->uniformFloat("prefilterMaxLod", float(m_environment->prefiltered()->levels() - 1));
    }
    else
    {
        bindFallbackCubemap(IrradianceMapUnit);
        bindFallbackCubemap(PrefilterMapUnit);
        bindFallbackTexture2D(BrdfLutUnit);
    }
    prog->uniformBool("useIBL", ibl);
    prog->uniformFloat("envIntensity", m_environmentIntensity);
    prog->uniformInt("irradianceMap", static_cast<int>(IrradianceMapUnit));
    prog->uniformInt("prefilterMap", static_cast<int>(PrefilterMapUnit));
    prog->uniformInt("brdfLUT", static_cast<int>(BrdfLutUnit));
    prog->uniformBool("pbrToneMap", m_toneMapping);
}

void PBRMaterial::unbindMaps()
{
    if (m_albedoMap)    m_albedoMap->unbind();
    if (m_normalMap)    m_normalMap->unbind();
    if (m_roughnessMap) m_roughnessMap->unbind();
    if (m_metallicMap)  m_metallicMap->unbind();
    if (m_aoMap)        m_aoMap->unbind();
    if (m_metallicRoughnessMap) m_metallicRoughnessMap->unbind();
    if (m_emissiveMap)  m_emissiveMap->unbind();
    if (m_envCubemap)   m_envCubemap->unbind();
}
