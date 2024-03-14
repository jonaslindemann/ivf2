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

	class CappedCone : public MeshNode {
	private:
        double m_radius{1.0};
        double m_size{1.0};
        int m_slices{32};
        int m_segments{8};
        int m_rings{4};
        double m_start{0.0};
        double m_sweep{2 * glm::pi<double>()};
	public:

		CappedCone(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4, double start = 0.0, double sweep = 2*glm::pi<double>());

		static std::shared_ptr<CappedCone> create(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4, double start = 0.0, double sweep = 2 * glm::pi<double>());

		void set(double radius = 1.0, double size = 1.0, int slices = 32, int segments = 8, int rings = 4, double start = 0.0, double sweep = 2 * glm::pi<double>());

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<CappedCone> CappedConePtr;

}


