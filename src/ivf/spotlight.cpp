#include <ivf/spotlight.h>

#include <ivf/lightmanager.h>
#include <ivf/shadermanager.h>

#include <sstream>
#include <iostream>
#include <string>

using namespace ivf;

SpotLight::SpotLight()
	:m_direction(glm::vec3(0.0, 0.0, 1.0)),
	 m_constAttenuation(1.0),
	 m_linearAttenuation(0.0),
	 m_quadraticAttenuation(0.0),
	 m_cutoff(25.5f),
	 m_outerCutoff(30.0f)
{
	setLightArrayName("spotLights");
}

std::shared_ptr<SpotLight> ivf::SpotLight::create()
{
	return std::make_shared<SpotLight>();
}

void ivf::SpotLight::setDirection(glm::vec3 direction)
{
	m_direction = direction;
}

void ivf::SpotLight::setAttenuation(float cnst, float linear, float quadratic)
{
	m_constAttenuation = cnst;
	m_linearAttenuation = linear;
	m_quadraticAttenuation = quadratic;
}

void ivf::SpotLight::setCutoff(float inner, float outer)
{
	m_cutoff = inner;
	m_outerCutoff = outer;
}

glm::vec3 ivf::SpotLight::direction()
{
	return m_direction;
}

float ivf::SpotLight::constAttenuation()
{
	return m_constAttenuation;
}

float ivf::SpotLight::linearAttenutation()
{
	return m_linearAttenuation;
}

float ivf::SpotLight::quadraticAttenuation()
{
	return m_quadraticAttenuation;
}

float ivf::SpotLight::innerCutoff()
{
	return m_cutoff;
}

float ivf::SpotLight::outerCutoff()
{
	return m_outerCutoff;
}

void ivf::SpotLight::apply()
{
	std::string prefix;
	std::stringstream ss;

	ss << lightArrayName() << "[" << index() << "].";
	prefix = ss.str();

	ShaderManager::instance()->currentProgram()->use();
	ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "diffuseColor", diffuseColor());
	ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "specularColor", specularColor());
	ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "ambientColor", ambientColor());
	ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "position", position());
	ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "direction", direction());
	ShaderManager::instance()->currentProgram()->uniformBool(prefix + "enabled", enabled());
	ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "constant", constAttenuation());
	ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "linear", linearAttenutation());
	ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "quadratic", quadraticAttenuation());
	ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "cutOff", glm::cos(glm::radians(innerCutoff())));
	ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "outerCutOff", glm::cos(glm::radians(outerCutoff())));
}

