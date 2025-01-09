#include <ivf/material.h>

#include <ivf/light_manager.h>

using namespace ivf;

Material::Material()
    : m_diffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0)), m_ambientColor(glm::vec4(0.2, 0.2, 0.2, 1.0)),
      m_specularColor(glm::vec4(1.0, 1.0, 1.0, 1.0)), m_useLighting(true), m_useVertexColor(false), m_useTexture(false),
      m_shininess(70.0)
{}

std::shared_ptr<Material> ivf::Material::create()
{
    return std::make_shared<Material>();
}

void ivf::Material::setDiffuseColor(glm::vec4 color)
{
    m_diffuseColor = color;
}

void ivf::Material::setSpecularColor(glm::vec4 color)
{
    m_specularColor = color;
}

void ivf::Material::setAmbientColor(glm::vec4 color)
{
    m_ambientColor = color;
}

void ivf::Material::setUseTexture(bool flag)
{
    m_useTexture = flag;
}

void ivf::Material::setShininess(float shininess)
{
    m_shininess = shininess;
}

void ivf::Material::setUseLighting(bool flag)
{
    m_useLighting = flag;
}

bool ivf::Material::useLighting() const
{
    return m_useLighting;
}

bool ivf::Material::useTexture() const
{
    return m_useTexture;
}

bool ivf::Material::useVertexColor() const
{
    return m_useVertexColor;
}

glm::vec4 ivf::Material::diffuseColor() const
{
    return m_diffuseColor;
}

glm::vec4 ivf::Material::specularColor() const
{
    return m_specularColor;
}

glm::vec4 ivf::Material::ambientColor() const
{
    return m_ambientColor;
}

float ivf::Material::shininess() const
{
    return m_shininess;
}

void ivf::Material::setUseVertexColor(bool flag)
{
    m_useVertexColor = flag;
}

void ivf::Material::apply()
{
    LightManager::instance()->setUseLighting(m_useLighting);
    LightManager::instance()->setUseVertexColors(m_useVertexColor);
    LightManager::instance()->setDiffuseColor(m_diffuseColor);
    LightManager::instance()->setAmbientColor(m_ambientColor);
    LightManager::instance()->setSpecularColor(m_specularColor);
    LightManager::instance()->setShininess(m_shininess);
}
