#pragma once

#include <ivfui/ui_window.h>

#include <ivf/point_light.h>

namespace ivfui {

	class PointLightWindow : public ivfui::UiWindow {
	private:
		ivf::PointLightPtr m_pointLight;
		float m_constAttenuation;
		float m_linearAttenuation;
		float m_quadraticAttenuation;
		glm::vec3 m_position;
		glm::vec3 m_diffuseColor;
		glm::vec3 m_specularColor;
		glm::vec3 m_ambientColor;
		bool m_enabled;

		float m_constAttenuationPrev;
		float m_linearAttenuationPrev;
		float m_quadraticAttenuationPrev;
		glm::vec3 m_positionPrev;
		glm::vec3 m_diffuseColorPrev;
		glm::vec3 m_specularColorPrev;
		glm::vec3 m_ambientColorPrev;
		bool m_enabledPrev;

		bool m_isDirty;
	public:
		PointLightWindow(ivf::PointLightPtr pointLight, std::string caption);

		static std::shared_ptr<PointLightWindow> create(ivf::PointLightPtr pointLight, std::string caption);

		void doDraw() override;

		bool isDirty();

	};

	typedef std::shared_ptr<PointLightWindow> PointLightWindowPtr;
}



