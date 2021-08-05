#pragma once

#include <ivf/light.h>

namespace ivf {

	class DirectionalLight : public Light {
	private:
		glm::vec3 m_direction;
	public:
		DirectionalLight();

		static std::shared_ptr<DirectionalLight> create();

		void setDirection(glm::vec3 direction);
		glm::vec3 direction();

		void apply() override;
	};

	typedef std::shared_ptr<DirectionalLight> DirectionalLightPtr;

}


