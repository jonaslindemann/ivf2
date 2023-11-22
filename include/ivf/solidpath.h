#pragma once

#include <ivf/meshnode.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

#include <vector>

namespace ivf {

	class SolidPath : public MeshNode {
	private:
		std::vector<glm::vec3> m_points;
		double m_radius;
		int m_segments;
	public:

		SolidPath(double radius = 0.1, int segments = 64);

		static std::shared_ptr<SolidPath> create(double radius = 0.1, int segments = 64);

		void addPoint(glm::vec3 p);
		void clear();

		void setRadius(double radius);

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<SolidPath> SolidPathPtr;

}


