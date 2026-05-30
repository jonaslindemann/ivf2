#include <ivf/pbr_material.h>

#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>
#include <ivf/light_manager.h>
#include <ivf/selection_manager.h>

using namespace ivf;

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

void PBRMaterial::bindMaps()
{
    auto prog = ShaderManager::instance()->currentProgram();

    if (m_albedoMap) {
        m_albedoMap->setTexUnit(0);
        m_albedoMap->bind();
        prog->uniformInt("albedoMap", 0);
        prog->uniformBool("useAlbedoMap", true);
    } else {
        prog->uniformBool("useAlbedoMap", false);
    }

    if (m_normalMap) {
        m_normalMap->setTexUnit(1);
        m_normalMap->bind();
        prog->uniformInt("normalMap", 1);
        prog->uniformBool("useNormalMap", true);
    } else {
        prog->uniformBool("useNormalMap", false);
    }

    if (m_roughnessMap) {
        m_roughnessMap->setTexUnit(2);
        m_roughnessMap->bind();
        prog->uniformInt("roughnessMap", 2);
        prog->uniformBool("useRoughnessMap", true);
    } else {
        prog->uniformBool("useRoughnessMap", false);
    }

    if (m_metallicMap) {
        m_metallicMap->setTexUnit(3);
        m_metallicMap->bind();
        prog->uniformInt("metallicMap", 3);
        prog->uniformBool("useMetallicMap", true);
    } else {
        prog->uniformBool("useMetallicMap", false);
    }

    if (m_aoMap) {
        m_aoMap->setTexUnit(4);
        m_aoMap->bind();
        prog->uniformInt("aoMap", 4);
        prog->uniformBool("useAOMap", true);
    } else {
        prog->uniformBool("useAOMap", false);
    }

    if (m_envCubemap) {
        m_envCubemap->bind(5);
        prog->uniformInt("envCubemap", 5);
        prog->uniformBool("useEnvCubemap", true);
    } else {
        prog->uniformBool("useEnvCubemap", false);
    }
}

void PBRMaterial::unbindMaps()
{
    if (m_albedoMap)    m_albedoMap->unbind();
    if (m_normalMap)    m_normalMap->unbind();
    if (m_roughnessMap) m_roughnessMap->unbind();
    if (m_metallicMap)  m_metallicMap->unbind();
    if (m_aoMap)        m_aoMap->unbind();
    if (m_envCubemap)   m_envCubemap->unbind();
}
