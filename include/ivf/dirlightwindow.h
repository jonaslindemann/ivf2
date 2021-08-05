#pragma once

#include <ivf/imguiwindow.h>
#include <ivf/dirlight.h>

namespace ivf {

	class DirectionalLightWindow : public ivf::UiWindow {
	private:
		DirectionalLightPtr m_dirLight;
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
		DirectionalLightWindow(DirectionalLightPtr dirLight, std::string caption);

		static std::shared_ptr<DirectionalLightWindow> create(DirectionalLightPtr dirLight, std::string caption);

		void doDraw() override;

		bool isDirty();

	};

	typedef std::shared_ptr<DirectionalLightWindow> DirectionalLightWindowPtr;
}



