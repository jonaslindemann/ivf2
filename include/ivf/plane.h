#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
		double radius = 1.0,
		double innerRadius = 0.0,
		int slices = 32,
		int rings = 4,
		double start = 0.0,
		double sweep = gml::radians(360.0)
*/

namespace ivf {

	class Plane : public MeshNode {
	private:
		double m_width;
		double m_depth;
		int m_rows;
		int m_cols;
	public:

		Plane(double width = 1.0, double depth = 1.0, int rows = 8, int cols = 8);

		static std::shared_ptr<Plane> create(double width = 1.0, double depth = 1.0, int rows = 8, int cols = 8);

		void set(double width = 1.0, double depth = 1.0, int rows = 8, int cols = 8);

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<Plane> PlanePtr;

}


