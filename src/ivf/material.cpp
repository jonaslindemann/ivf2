#include <ivf/material.h>

#include <ivf/lightmanager.h>

using namespace ivf;

Material::Material()
	:m_diffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0)),
	 m_ambientColor(glm::vec4(0.2, 0.2, 0.2, 1.0)),
	 m_specularColor(glm::vec4(1.0, 1.0, 1.0, 1.0)),
	 m_useLighting(true),
	 m_useVertexColor(false), 
	 m_useTexture(false)
{
}

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

void ivf::Material::apply()
{
	LightManager::instance()->setUseLighting(m_useLighting);
	LightManager::instance()->setUseVertexColors(m_useVertexColor);
	LightManager::instance()->setUseTexture(m_useTexture);
	LightManager::instance()->setDiffuseColor(m_diffuseColor);
	LightManager::instance()->setAmbientColor(m_ambientColor);
	LightManager::instance()->setSpecularColor(m_specularColor);
	LightManager::instance()->setUseTexture(m_useTexture);
}
