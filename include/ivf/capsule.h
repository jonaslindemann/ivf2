#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

/*
		double radius = 1.0,
		double size = 0.5,
		int slices = 32,
		int segments = 4,
		int rings = 8,
		double start = 0.0,
		double sweep = gml::radians(360.0)

*/

namespace ivf {

	class Capsule : public MeshNode {
	private:
		double m_radius;
		double m_size;
		int m_slices;
		int m_segments;
		int m_rings;
		double m_start;
		double m_sweep;
	public:

		Capsule(double radius = 1.0, double size = 0.5, int slices = 32, int segments = 4, int rings = 8, double start = 0.0, double sweep = gml::radians(360.0));

		static std::shared_ptr<Capsule> create(double radius = 1.0, double size = 0.5, int slices = 32, int segments = 4, int rings = 8, double start = 0.0, double sweep = gml::radians(360.0));

		void set(double radius = 1.0, double size = 0.5, int slices = 32, int segments = 4, int rings = 8, double start = 0.0, double sweep = gml::radians(360.0));

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<Capsule> CapsulePtr;

}


