#include <ivf/pointlightwindow.h>

#include <ivf/lightmanager.h>

using namespace ivf;

PointLightWindow::PointLightWindow(ivf::PointLightPtr pointLight, std::string caption)
	:UiWindow(caption),
	 m_pointLight(pointLight),
	 m_constAttenuation(pointLight->constAttenuation()),
	 m_linearAttenuation(pointLight->linearAttenutation()),
	 m_quadraticAttenuation(pointLight->quadraticAttenuation()),
	 m_diffuseColor(pointLight->diffuseColor()),
	 m_specularColor(pointLight->specularColor()),
	 m_ambientColor(pointLight->ambientColor()),
	 m_position(pointLight->position()),
	 m_enabled(pointLight->enabled()),
	 m_constAttenuationPrev(pointLight->constAttenuation()),
	 m_linearAttenuationPrev(pointLight->linearAttenutation()),
	 m_quadraticAttenuationPrev(pointLight->quadraticAttenuation()),
	 m_diffuseColorPrev(pointLight->diffuseColor()),
	 m_specularColorPrev(pointLight->specularColor()),
	 m_ambientColorPrev(pointLight->ambientColor()),
	 m_positionPrev(pointLight->position()),
	 m_enabledPrev(pointLight->enabled()),
	 m_isDirty(false)
{
}

std::shared_ptr<PointLightWindow> PointLightWindow::create(ivf::PointLightPtr pointLight, std::string caption)
{
	return std::make_shared<PointLightWindow>(pointLight, caption);
}

void PointLightWindow::doDraw()
{
	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::DragFloat3("Position", &m_position.x);
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
		(m_position != m_positionPrev);

	m_constAttenuationPrev = m_constAttenuation;
	m_linearAttenuationPrev = m_linearAttenuation;
	m_quadraticAttenuationPrev = m_quadraticAttenuation;
	m_diffuseColorPrev = m_diffuseColor;
	m_specularColorPrev = m_specularColor;
	m_ambientColorPrev = m_ambientColor;
	m_enabledPrev = m_enabled;
	m_positionPrev = m_position;

	m_pointLight->setPosition(m_position);
	m_pointLight->setAttenuation(m_constAttenuation, m_linearAttenuation, m_quadraticAttenuation);
	m_pointLight->setDiffuseColor(m_diffuseColor);
	m_pointLight->setSpecularColor(m_specularColor);
	m_pointLight->setAmbientColor(m_ambientColor);
	m_pointLight->setEnabled(m_enabled);
}
 
bool ivf::PointLightWindow::isDirty()
{
	return m_isDirty;
}
