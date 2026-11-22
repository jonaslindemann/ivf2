#include <ivf/material.h>

#include <ivf/light_manager.h>

using namespace ivf;

Material::Material(MaterialProps props)
    : m_useVertexColor(props.useVertexColor), m_useLighting(props.useLighting), m_useTexture(props.useTexture),
      m_diffuseColor(props.diffuseColor), m_specularColor(props.specularColor), m_ambientColor(props.ambientColor),
      m_alpha(props.alpha), m_shininess(props.shininess)
{}

std::shared_ptr<Material> ivf::Material::create(MaterialProps props)
{
    return std::make_shared<Material>(props);
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
