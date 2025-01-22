#include <ivf/point_light.h>

#include <ivf/light_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>

#include <sstream>
#include <iostream>
#include <string>

using namespace ivf;

PointLight::PointLight() : m_constAttenuation(1.0), m_linearAttenuation(0.0), m_quadraticAttenuation(0.0)
{
    setLightArrayName("pointLights");
}

std::shared_ptr<PointLight> ivf::PointLight::create()
{
    return std::make_shared<PointLight>();
}

void ivf::PointLight::setAttenuation(float cnst, float linear, float quadratic)
{
    m_constAttenuation = cnst;
    m_linearAttenuation = linear;
    m_quadraticAttenuation = quadratic;
}

float ivf::PointLight::constAttenuation()
{
    return m_constAttenuation;
}

float ivf::PointLight::linearAttenutation()
{
    return m_linearAttenuation;
}

float ivf::PointLight::quadraticAttenuation()
{
    return m_quadraticAttenuation;
}

void ivf::PointLight::apply()
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
    ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "constant", constAttenuation());
    ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "linear", linearAttenutation());
    ShaderManager::instance()->currentProgram()->uniformFloat(prefix + "quadratic", quadraticAttenuation());
}
