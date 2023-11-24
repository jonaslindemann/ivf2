#include <ivf/dir_light.h>

#include <ivf/light_manager.h>
#include <ivf/shader_manager.h>

#include <sstream>
#include <iostream>
#include <string>

using namespace ivf;

DirectionalLight::DirectionalLight()
	:m_direction(glm::vec3(0.0, 0.0, 1.0))
{
	setLightArrayName("dirLights");
}

std::shared_ptr<DirectionalLight> ivf::DirectionalLight::create()
{
	return std::make_shared<DirectionalLight>();
}

void ivf::DirectionalLight::setDirection(glm::vec3 direction)
{
	m_direction = direction;
}

glm::vec3 ivf::DirectionalLight::direction()
{
	return m_direction;
}

void ivf::DirectionalLight::apply()
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
	ShaderManager::instance()->currentProgram()->uniformBool(prefix + "enabled", enabled());
	ShaderManager::instance()->currentProgram()->uniformVec3(prefix + "direction", direction());
}
