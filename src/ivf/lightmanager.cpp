#include <ivf/lightmanager.h>

#include <ivf/shadermanager.h>

using namespace ivf;

LightManager* LightManager::m_instance = 0;

LightManager::LightManager()
{
	m_useLighting = true;
	m_savedState = true;

	this->setupDefaultColors();

	ShaderManager::instance()->currentProgram()->uniformVec4f("lightColor", 1.0f, 1.0f, 1.0f, 1.0f);
	ShaderManager::instance()->currentProgram()->uniformVec3f("lightPos", 4.0, 6.0, 2.0);
}

void LightManager::setUseLighting(bool flag)
{
	m_useLighting = flag;

	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformBool("useLighting", m_useLighting);
}

void ivf::LightManager::setUseVertexColors(bool flag)
{
	m_useVertexColors = flag;

	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformBool("useVertexColors", m_useVertexColors);
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

void ivf::LightManager::setUseTexture(bool flag)
{
	m_useTexture = flag;

	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformBool("useTexture", m_useTexture);
}

void ivf::LightManager::setupDefaultColors()
{
	this->setUseLighting(true);
	this->setUseVertexColors(false);
	this->setDiffuseColor(glm::vec3(1.0, 1.0, 0.0));
	this->setAmbientColor(glm::vec3(0.2, 0.2, 0.2));
	this->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
}

void ivf::LightManager::setDiffuseColor(glm::vec3 color)
{
	glm::vec4 colorAlpha(1.0);
	colorAlpha.r = color.r;
	colorAlpha.g = color.g;
	colorAlpha.b = color.b;
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("diffuseColor", colorAlpha);
}

void ivf::LightManager::setDiffuseColor(glm::vec4 color)
{
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("diffuseColor", color);
}

void ivf::LightManager::setDiffuseColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glm::vec4 colorAlpha(red, green, blue, alpha);
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("diffuseColor", colorAlpha);
}

void ivf::LightManager::setSpecularColor(glm::vec3 color)
{
	glm::vec4 colorAlpha(1.0);
	colorAlpha.r = color.r;
	colorAlpha.g = color.g;
	colorAlpha.b = color.b;
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("specularColor", colorAlpha);
}

void ivf::LightManager::setSpecularColor(glm::vec4 color)
{
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("specularColor", color);
}

void ivf::LightManager::setSpecularColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glm::vec4 colorAlpha(red, green, blue, alpha);
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("specularColor", colorAlpha);
}

void ivf::LightManager::setAmbientColor(glm::vec3 color)
{
	glm::vec4 colorAlpha(1.0);
	colorAlpha.r = color.r;
	colorAlpha.g = color.g;
	colorAlpha.b = color.b;
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("ambientColor", colorAlpha);
}

void ivf::LightManager::setAmbientColor(glm::vec4 color)
{
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("ambientColor", color);
}

void ivf::LightManager::setAmbientColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glm::vec4 colorAlpha(red, green, blue, alpha);
	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec4("ambientColor", colorAlpha);
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


