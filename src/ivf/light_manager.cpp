#include <ivf/light_manager.h>

#include <ivf/shader_manager.h>

#include <strstream>

using namespace ivf;

LightManager *LightManager::m_instance = 0;

LightManager::LightManager()
    : m_useLighting(true), m_useVertexColors(false), m_useLightingId(-1), m_useVertexColorsId(-1), m_useTextureId(-1),
      m_pointLightCountId(-1), m_directionalLightCountId(-1), m_spotLightCountId(-1), m_diffuseColorId(-1),
      m_specularColorId(-1), m_ambientColorId(-1), m_shininessId(-1), m_alphaId(-1), m_savedState(false)
{
    m_useLightingId = ShaderManager::instance()->currentProgram()->uniformLoc("useLighting");
    m_useVertexColorsId = ShaderManager::instance()->currentProgram()->uniformLoc("useVertexColors");
    m_pointLightCountId = ShaderManager::instance()->currentProgram()->uniformLoc("pointLightCount");
    m_directionalLightCountId = ShaderManager::instance()->currentProgram()->uniformLoc("dirLightCount");
    m_spotLightCountId = ShaderManager::instance()->currentProgram()->uniformLoc("spotLightCount");

    m_diffuseColorId = ShaderManager::instance()->currentProgram()->uniformLoc("material.diffuseColor");
    m_specularColorId = ShaderManager::instance()->currentProgram()->uniformLoc("material.specularColor");
    m_ambientColorId = ShaderManager::instance()->currentProgram()->uniformLoc("material.ambientColor");
    m_shininessId = ShaderManager::instance()->currentProgram()->uniformLoc("material.shininess");
    m_alphaId = ShaderManager::instance()->currentProgram()->uniformLoc("material.alpha");

    this->setupDefaultColors();
}

PointLightPtr ivf::LightManager::addPointLight()
{
    auto pointLight = PointLight::create();
    m_pointLights.push_back(pointLight);
    return pointLight;
}

DirectionalLightPtr ivf::LightManager::addDirectionalLight()
{
    auto dirLight = DirectionalLight::create();
    m_dirLights.push_back(dirLight);
    return dirLight;
}

SpotLightPtr ivf::LightManager::addSpotLight()
{
    auto spotLight = SpotLight::create();
    m_spotLights.push_back(spotLight);
    return spotLight;
}

PointLightPtr ivf::LightManager::getPointLight(int idx)
{
    if ((idx >= 0) && (idx < m_pointLights.size()))
        return m_pointLights[idx];
    else
        return nullptr;
}

DirectionalLightPtr ivf::LightManager::getDirectionalLight(int idx)
{
    if ((idx >= 0) && (idx < m_dirLights.size()))
        return m_dirLights[idx];
    else
        return nullptr;
}

SpotLightPtr ivf::LightManager::getSpotLight(int idx)
{
    if ((idx >= 0) && (idx < m_spotLights.size()))
        return m_spotLights[idx];
    else
        return nullptr;
}

size_t ivf::LightManager::pointLightCount()
{
    return m_pointLights.size();
}

size_t ivf::LightManager::dirLightCount()
{
    return m_dirLights.size();
}

size_t ivf::LightManager::spotLightCount()
{
    return m_spotLights.size();
}

void ivf::LightManager::clearPointLights()
{
    for (auto i = 0; i < m_pointLights.size(); i++)
    {
        m_pointLights[i]->setEnabled(false);
        m_pointLights[i]->apply();
    }
    m_pointLights.clear();
}

void ivf::LightManager::clearDirectionalLights()
{
    for (auto i = 0; i < m_dirLights.size(); i++)
    {
        m_dirLights[i]->setEnabled(i);
        m_dirLights[i]->apply();
    }
    m_dirLights.clear();
}

void ivf::LightManager::clearSpotLights()
{
    for (auto i = 0; i < m_spotLights.size(); i++)
    {
        m_spotLights[i]->setEnabled(i);
        m_spotLights[i]->apply();
    }
    m_spotLights.clear();
}

void ivf::LightManager::clearLights()
{
    this->clearPointLights();
    this->clearDirectionalLights();
    this->clearSpotLights();
}

void LightManager::setUseLighting(bool flag)
{
    m_useLighting = flag;

    ShaderManager::instance()->currentProgram()->uniformBool(m_useLightingId, m_useLighting);
}

void ivf::LightManager::setUseVertexColors(bool flag)
{
    m_useVertexColors = flag;

    ShaderManager::instance()->currentProgram()->uniformBool(m_useVertexColorsId, m_useVertexColors);
}

bool LightManager::useLighting()
{
    return m_useLighting;
}

void LightManager::enableLighting()
{
    this->setUseLighting(true);
    this->setUseVertexColors(false);
}

void LightManager::disableLighting()
{
    this->setUseLighting(false);
    this->setUseVertexColors(true);
}

void ivf::LightManager::setupDefaultColors()
{
    this->setUseLighting(true);
    this->setUseVertexColors(false);
    this->setDiffuseColor(glm::vec3(1.0, 1.0, 0.0));
    this->setAmbientColor(glm::vec3(0.2, 0.2, 0.2));
    this->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
    this->setAlpha(1.0);
}

void ivf::LightManager::apply()
{
    ShaderManager::instance()->currentProgram()->uniformInt(m_pointLightCountId, m_pointLights.size());
    ShaderManager::instance()->currentProgram()->uniformInt(m_directionalLightCountId, m_dirLights.size());
    ShaderManager::instance()->currentProgram()->uniformInt(m_spotLightCountId, m_spotLights.size());

    for (auto i = 0; i < m_pointLights.size(); i++)
    {
        m_pointLights[i]->setIndex(i);
        m_pointLights[i]->apply();
    }

    for (auto i = 0; i < m_dirLights.size(); i++)
    {
        m_dirLights[i]->setIndex(i);
        m_dirLights[i]->apply();
    }

    for (auto i = 0; i < m_spotLights.size(); i++)
    {
        m_spotLights[i]->setIndex(i);
        m_spotLights[i]->apply();
    }
}

void ivf::LightManager::setDiffuseColor(glm::vec3 color)
{
    glm::vec4 colorAlpha(1.0);
    colorAlpha.r = color.r;
    colorAlpha.g = color.g;
    colorAlpha.b = color.b;
    ShaderManager::instance()->currentProgram()->uniformVec3(m_diffuseColorId, colorAlpha);
}

void ivf::LightManager::setDiffuseColor(glm::vec4 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_diffuseColorId, glm::vec3(color.r, color.g, color.b));
}

void ivf::LightManager::setDiffuseColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_diffuseColorId, glm::vec3(red, green, blue));
}

void ivf::LightManager::setSpecularColor(glm::vec3 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_specularColorId, color);
}

void ivf::LightManager::setSpecularColor(glm::vec4 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_specularColorId, glm::vec3(color.r, color.g, color.b));
}

void ivf::LightManager::setSpecularColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_specularColorId, glm::vec3(red, green, blue));
}

void ivf::LightManager::setAmbientColor(glm::vec3 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_ambientColorId, color);
}

void ivf::LightManager::setAmbientColor(glm::vec4 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_ambientColorId, glm::vec3(color.r, color.g, color.b));
}

void ivf::LightManager::setAmbientColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_ambientColorId, glm::vec3(red, green, blue));
}

void ivf::LightManager::setShininess(float shininess)
{
    ShaderManager::instance()->currentProgram()->uniformFloat(m_shininessId, shininess);
}

void ivf::LightManager::setAlpha(float alpha)
{
    ShaderManager::instance()->currentProgram()->uniformFloat(m_alphaId, alpha);
}

void LightManager::saveState()
{
    m_savedState = m_useLighting;
}

void LightManager::restoreState()
{
    m_useLighting = m_savedState;
    this->setUseLighting(m_useLighting);
}
