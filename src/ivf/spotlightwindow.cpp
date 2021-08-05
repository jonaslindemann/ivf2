#include <ivf/spotlightwindow.h>

#include <ivf/lightmanager.h>

using namespace ivf;

SpotLightWindow::SpotLightWindow(ivf::SpotLightPtr spotLight, std::string caption)
	:UiWindow(caption),
	 m_spotLight(spotLight),
	 m_constAttenuation(spotLight->constAttenuation()),
	 m_linearAttenuation(spotLight->linearAttenutation()),
	 m_quadraticAttenuation(spotLight->quadraticAttenuation()),
	 m_diffuseColor(spotLight->diffuseColor()),
	 m_specularColor(spotLight->specularColor()),
	 m_ambientColor(spotLight->ambientColor()),
	 m_position(spotLight->position()),
	 m_direction(spotLight->direction()),
	 m_enabled(spotLight->enabled()),
	 m_constAttenuationPrev(spotLight->constAttenuation()),
	 m_linearAttenuationPrev(spotLight->linearAttenutation()),
	 m_quadraticAttenuationPrev(spotLight->quadraticAttenuation()),
	 m_diffuseColorPrev(spotLight->diffuseColor()),
	 m_specularColorPrev(spotLight->specularColor()),
	 m_ambientColorPrev(spotLight->ambientColor()),
	 m_positionPrev(spotLight->position()),
	 m_directionPrev(spotLight->direction()),
	 m_enabledPrev(spotLight->enabled()),
	 m_isDirty(false)
{
}

std::shared_ptr<SpotLightWindow> SpotLightWindow::create(ivf::SpotLightPtr spotLight, std::string caption)
{
	return std::make_shared<SpotLightWindow>(spotLight, caption);
}

void SpotLightWindow::doDraw()
{
	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::DragFloat3("Position", &m_position.x);
	ImGui::DragFloat3("Direction", &m_direction.x);
	ImGui::ColorEdit3("Diffuse Color", &m_diffuseColor.x);
	ImGui::ColorEdit3("Specular Color", &m_specularColor.x);
	ImGui::ColorEdit3("Ambient Color", &m_ambientColor.x);
	ImGui::SliderFloat("Kc", &m_constAttenuation, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::SliderFloat("Kl", &m_linearAttenuation, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::SliderFloat("Kq", &m_quadraticAttenuation, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

	m_isDirty = (m_constAttenuation != m_constAttenuationPrev) ||
		(m_linearAttenuation != m_linearAttenuationPrev) ||
		(m_quadraticAttenuation != m_quadraticAttenuationPrev) ||
		(m_enabled != m_enabledPrev) ||
		(m_diffuseColor != m_diffuseColorPrev) ||
		(m_specularColor != m_specularColorPrev) ||
		(m_ambientColor != m_ambientColorPrev) ||
		(m_direction != m_directionPrev) ||
		(m_position != m_positionPrev);

	m_constAttenuationPrev = m_constAttenuation;
	m_linearAttenuationPrev = m_linearAttenuation;
	m_quadraticAttenuationPrev = m_quadraticAttenuation;
	m_diffuseColorPrev = m_diffuseColor;
	m_specularColorPrev = m_specularColor;
	m_ambientColorPrev = m_ambientColor;
	m_enabledPrev = m_enabled;
	m_positionPrev = m_position;
	m_directionPrev = m_direction;

	m_spotLight->setPosition(m_position);
	m_spotLight->setDirection(m_direction);
	m_spotLight->setAttenuation(m_constAttenuation, m_linearAttenuation, m_quadraticAttenuation);
	m_spotLight->setDiffuseColor(m_diffuseColor);
	m_spotLight->setSpecularColor(m_specularColor);
	m_spotLight->setAmbientColor(m_ambientColor);
	m_spotLight->setEnabled(m_enabled);
}
 
bool ivf::SpotLightWindow::isDirty()
{
	return m_isDirty;
}
