#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
double radius = 1.0,
double size = 1.0,
int slices = 32,
int segments = 8,
int rings = 4,
double start = 0.0,
double sweep = gml::radians(360.0)
*/

namespace ivf {

	class CappedCylinder : public MeshNode {
	private:
		double m_radius;
		double m_size;
		int m_slices;
		int m_segments;
		int m_rings;
		double m_start;
		double m_sweep;
	public:

		CappedCylinder(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4, double start = 0.0, double sweep = 2*glm::pi<double>());

		static std::shared_ptr<CappedCylinder> create(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4, double start = 0.0, double sweep = 2 * glm::pi<double>());

		void set(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4, double start = 0.0, double sweep = 2 * glm::pi<double>());

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<CappedCylinder> CappedCylinderPtr;

}


