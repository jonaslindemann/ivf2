#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
* double radius = 1.0, int segments = 1, int rings = 1
*/

namespace ivf {

	class Dodecahedron : public MeshNode {
	private:
		double m_radius;
		int m_segments;
		int m_rings;
	public:

		Dodecahedron(double radius = 1.0, int segments = 1, int rings = 1);

		static std::shared_ptr<Dodecahedron> create(double radius = 1.0, int segments = 1, int rings = 1);

		void set(double radius = 1.0, int segments = 1, int rings = 1);

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<Dodecahedron> DodecahedronPtr;

}


