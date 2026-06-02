#include <ivf/pbr_material.h>

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
    static GLuint fallbackTexture = 0;

    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

    if (fallbackTexture == 0)
    {
        const unsigned char pixel[] = {255, 255, 255, 255};

        glGenTextures(1, &fallbackTexture);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
    }

    glActiveTexture(activeTexture);
}

void bindFallbackCubemap(GLuint unit)
{
    static GLuint fallbackCubemap = 0;

    GLint activeTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

    if (fallbackCubemap == 0)
    {
        const unsigned char pixel[] = {0, 0, 0, 255};

        glGenTextures(1, &fallbackCubemap);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, fallbackCubemap);

        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++face)
            glTexImage2D(face, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, fallbackCubemap);
    }

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

    if (m_envCubemap) {
        m_envCubemap->bind(kEnvCubemapUnit);
        prog->uniformBool("useEnvCubemap", true);
    } else {
        bindFallbackCubemap(kEnvCubemapUnit);
        prog->uniformBool("useEnvCubemap", false);
    }
    prog->uniformInt("envCubemap", kEnvCubemapUnit);
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
