#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

	class Sphere : public MeshNode {
	private:
		double m_radius;
		int m_slices;
		int m_segments;
		double m_sliceStart;
		double m_sliceSweep;
		double m_segmentStart;
		double m_segmentSweep;
	public:

		Sphere(double radius = 1.0, int slices = 32, int segments = 16, double sliceStart = 0.0, double sliceSweep = 2 * glm::pi<double>(), double segmentStart = 0.0, double segmentSweep = glm::pi<double>());

		static std::shared_ptr<Sphere> create(double radius = 1.0, int slices = 32, int segments = 16, double sliceStart = 0.0, double sliceSweep = 2 * glm::pi<double>(), double segmentStart = 0.0, double segmentSweep = glm::pi<double>());

		void set(double radius = 1.0, int slices = 32, int segments = 16, double sliceStart = 0.0, double sliceSweep = 2 * glm::pi<double>(), double segmentStart = 0.0, double segmentSweep = glm::pi<double>());

		void setRadius(double radius);
		void setSlices(int slices);
		void setSegments(int segments);
		void setSliceStart(double sliceStart);
		void setSliceSweep(double sliceSweep);
		void setSegmentStart(double segmentStart);
		void setSegmentSweep(double segmentSweep);
	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<Sphere> SpherePtr;

}


