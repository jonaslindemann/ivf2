#pragma once

#include <ivfui/ui_window.h>

#include <ivf/spot_light.h>

namespace ivfui {

	class SpotLightWindow : public ivfui::UiWindow {
	private:
		ivf::SpotLightPtr m_spotLight;
		float m_constAttenuation;
		float m_linearAttenuation;
		float m_quadraticAttenuation;
		float m_cutoff;
		float m_outerCutoff;
		glm::vec3 m_direction;
		glm::vec3 m_position;
		glm::vec3 m_diffuseColor;
		glm::vec3 m_specularColor;
		glm::vec3 m_ambientColor;
		bool m_enabled;

		float m_constAttenuationPrev;
		float m_linearAttenuationPrev;
		float m_quadraticAttenuationPrev;
		float m_cutoffPrev;
		float m_outerCutoffPrev;
		glm::vec3 m_directionPrev;
		glm::vec3 m_positionPrev;
		glm::vec3 m_diffuseColorPrev;
		glm::vec3 m_specularColorPrev;
		glm::vec3 m_ambientColorPrev;
		bool m_enabledPrev;

		bool m_isDirty;
	public:
		SpotLightWindow(ivf::SpotLightPtr spotLight, std::string caption);

		static std::shared_ptr<SpotLightWindow> create(ivf::SpotLightPtr spotLight, std::string caption);

		void doDraw() override;

		bool isDirty();

	};

	typedef std::shared_ptr<SpotLightWindow> SpotLightWindowPtr;
}



