#pragma once

#include <ivf/meshnode.h>

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

	class Disk : public MeshNode {
	private:
		double m_radius;
		double m_innerRadius;
		int m_slices;
		int m_rings;
		double m_start;
		double m_sweep;
	public:

		Disk(double radius = 1.0, double innerRadius = 0.0, int slices = 32, int rings = 4, double start = 0.0, double sweep = 2*glm::pi<double>());

		static std::shared_ptr<Disk> create(double radius = 1.0, double innerRadius = 0.0, int slices = 32, int rings = 4, double start = 0.0, double sweep = 2 * glm::pi<double>());

		void set(double radius = 1.0, double innerRadius = 0.0, int slices = 32, int rings = 4, double start = 0.0, double sweep = 2 * glm::pi<double>());

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<Disk> DiskPtr;

}


