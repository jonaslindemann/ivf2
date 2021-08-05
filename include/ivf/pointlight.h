#pragma once

#include <ivf/light.h>

namespace ivf {

	class PointLight : public Light {
	private:
		float m_constAttenuation;
		float m_linearAttenuation;
		float m_quadraticAttenuation;
	public:
		PointLight();

		static std::shared_ptr<PointLight> create();

		void setAttenuation(float cnst, float linear, float quadratic);

		float constAttenuation();
		float linearAttenutation();
		float quadraticAttenuation();

		void apply() override;
	};

	typedef std::shared_ptr<PointLight> PointLightPtr;

}


