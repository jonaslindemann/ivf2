#pragma once

#include <ivfui/ui_window.h>

#include <ivf/dir_light.h>

namespace ivfui {

	class DirectionalLightWindow : public ivfui::UiWindow {
	private:
		ivf::DirectionalLightPtr m_dirLight;
		glm::vec3 m_direction;
		glm::vec3 m_diffuseColor;
		glm::vec3 m_specularColor;
		glm::vec3 m_ambientColor;
		bool m_enabled;

		glm::vec3 m_directionPrev;
		glm::vec3 m_diffuseColorPrev;
		glm::vec3 m_specularColorPrev;
		glm::vec3 m_ambientColorPrev;
		bool m_enabledPrev;

		bool m_isDirty;
	public:
		DirectionalLightWindow(ivf::DirectionalLightPtr dirLight, std::string caption);

		static std::shared_ptr<DirectionalLightWindow> create(ivf::DirectionalLightPtr dirLight, std::string caption);

		void doDraw() override;

		bool isDirty();

	};

	typedef std::shared_ptr<DirectionalLightWindow> DirectionalLightWindowPtr;
}



