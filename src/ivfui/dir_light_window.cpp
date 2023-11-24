#include <ivfui/dir_light_window.h>

#include <ivf/light_manager.h>

using namespace ivfui;
using namespace ivf;

DirectionalLightWindow::DirectionalLightWindow(ivf::DirectionalLightPtr dirLight, std::string caption)
	:UiWindow(caption),
	 m_dirLight(dirLight),
	 m_direction(dirLight->direction()),
	 m_diffuseColor(dirLight->diffuseColor()),
	 m_specularColor(dirLight->specularColor()),
	 m_ambientColor(dirLight->ambientColor()),
	 m_enabled(dirLight->enabled()),
	 m_diffuseColorPrev(dirLight->diffuseColor()),
	 m_specularColorPrev(dirLight->specularColor()),
	 m_ambientColorPrev(dirLight->ambientColor()),
	 m_directionPrev(dirLight->direction()),
	 m_enabledPrev(dirLight->enabled()),
	 m_isDirty(false)
{
}

std::shared_ptr<DirectionalLightWindow> DirectionalLightWindow::create(ivf::DirectionalLightPtr dirLight, std::string caption)
{
	return std::make_shared<DirectionalLightWindow>(dirLight, caption);
}

void DirectionalLightWindow::doDraw()
{
	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::DragFloat3("Direction", &m_direction.x);
	ImGui::ColorEdit3("Diffuse Color", &m_diffuseColor.x);
	ImGui::ColorEdit3("Specular Color", &m_specularColor.x);
	ImGui::ColorEdit3("Ambient Color", &m_ambientColor.x);

	m_isDirty = 
		(m_enabled != m_enabledPrev) ||
		(m_diffuseColor != m_diffuseColorPrev) ||
		(m_specularColor != m_specularColorPrev) ||
		(m_ambientColor != m_ambientColorPrev) ||
		(m_direction != m_directionPrev);

	m_diffuseColorPrev = m_diffuseColor;
	m_specularColorPrev = m_specularColor;
	m_ambientColorPrev = m_ambientColor;
	m_enabledPrev = m_enabled;
	m_directionPrev = m_direction;

	m_dirLight->setDirection(m_direction);
	m_dirLight->setDiffuseColor(m_diffuseColor);
	m_dirLight->setSpecularColor(m_specularColor);
	m_dirLight->setAmbientColor(m_ambientColor);
	m_dirLight->setEnabled(m_enabled);
}
 
bool ivfui::DirectionalLightWindow::isDirty()
{
	return m_isDirty;
}
