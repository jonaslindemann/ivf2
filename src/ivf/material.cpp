#include <ivf/material.h>

#include <ivf/light_manager.h>

using namespace ivf;

Material::Material()
    : m_diffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0)), m_ambientColor(glm::vec4(0.2, 0.2, 0.2, 1.0)),
      m_specularColor(glm::vec4(1.0, 1.0, 1.0, 1.0)), m_useLighting(true), m_useVertexColor(false), m_useTexture(false),
      m_shininess(70.0), m_alpha(1.0)
{}

std::shared_ptr<Material> ivf::Material::create()
{
    return std::make_shared<Material>();
}

void ivf::Material::apply()
{
    LightManager::instance()->setUseLighting(m_useLighting);
    LightManager::instance()->setUseVertexColors(m_useVertexColor);
    LightManager::instance()->setDiffuseColor(m_diffuseColor);
    LightManager::instance()->setAmbientColor(m_ambientColor);
    LightManager::instance()->setSpecularColor(m_specularColor);
    LightManager::instance()->setShininess(m_shininess);
    LightManager::instance()->setAlpha(m_alpha);
}
