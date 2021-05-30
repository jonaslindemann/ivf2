#pragma once

#include <ivf/glbase.h>

#include <glm/glm.hpp>

namespace ivf {

	class Material : public GLBase {
	private:
		bool m_useVertexColor;
		bool m_useLighting;
		bool m_useTexture;
		glm::vec4 m_diffuseColor;
		glm::vec4 m_specularColor;
		glm::vec4 m_ambientColor;
	public:
		Material();

		static std::shared_ptr<Material> create();

		void setDiffuseColor(glm::vec4 color);
		void setSpecularColor(glm::vec4 color);
		void setAmbientColor(glm::vec4 color);
		void setUseTexture(bool flag);

		void apply();
	};

	typedef std::shared_ptr<Material> MaterialPtr;

}


