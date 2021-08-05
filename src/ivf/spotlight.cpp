#include <ivf/spotlight.h>

#include <ivf/lightmanager.h>

using namespace ivf;

SpotLight::SpotLight()
	:m_direction(glm::vec3(0.0, 0.0, 1.0)),
	 m_constAttenuation(1.0),
	 m_linearAttenuation(0.0),
	 m_quadraticAttenuation(0.0),
	 m_cutoff(glm::radians(12.5f)),
	 m_outerCutoff(glm::radians(20.0f))
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
