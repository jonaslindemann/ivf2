#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

	class SolidLine : public MeshNode {
	private:
		glm::vec3 m_p0;
		glm::vec3 m_p1;
		double m_radius;
	public:

		SolidLine(glm::vec3 p0, glm::vec3 p1, double radius = 0.1);

		static std::shared_ptr<SolidLine> create(glm::vec3 p0, glm::vec3 p1, double radius = 0.1);

		void set(glm::vec3 p0, glm::vec3 p1, double radius = 0.1);

		void setRadius(double radius);
		void setP0(glm::vec3 p);
		void setP1(glm::vec3 p);
		void setPoints(glm::vec3 p0, glm::vec3 p1);

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<SolidLine> SolidLinePtr;

}


