#pragma once

#include <ivf/meshnode.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

	class RoundedBox : public MeshNode {
	private:
		glm::vec3 m_size;
		glm::uvec3 m_segments;
		double m_radius;
		int m_slices;
	public:

		RoundedBox(glm::vec3 size = { 0.75f, 0.75f, 0.75f }, glm::vec3 segments = { 8, 8, 8 }, double radius = 0.25, int slices = 4);

		static std::shared_ptr<RoundedBox> create(glm::vec3 size = { 0.75f, 0.75f, 0.75f }, glm::vec3 segments = { 8, 8, 8 }, double radius = 0.25, int slices = 4);

		void set(glm::vec3 size = { 0.75f, 0.75f, 0.75f }, glm::vec3 segments = { 8, 8, 8 }, double radius = 0.25, int slices = 4);

		void setSize(double w, double h, double d);
		void setSize(glm::vec3 size);
		glm::vec3 size();

		void setSegments(int i, int j, int k);
		void setSegments(glm::uvec3 segments);
		glm::uvec3 segments();

		void setRadius(double radius);
		double radius();

		void setSlices(int slices);
		int slices();


	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<RoundedBox> RoundedBoxPtr;

}


