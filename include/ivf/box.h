#pragma once

#include <ivf/meshnode.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

namespace ivf {

	class Box : public MeshNode {
	private:
		glm::vec3 m_size;
		glm::uvec3 m_segments;
	public:

		Box(glm::vec3 size = { 1.0f, 1.0f, 1.0f }, glm::vec3 segments = { 8, 8, 8 });

		static std::shared_ptr<Box> create(glm::vec3 size = { 1.0f, 1.0f, 1.0f }, glm::vec3 segments = { 8, 8, 8 });

		void set(glm::vec3 size = { 1.0f, 1.0f, 1.0f }, glm::vec3 segments = { 8, 8, 8 });

		void setSize(double w, double h, double d);
		void setSize(glm::vec3 size);
		glm::vec3 size();

		void setSegments(int i, int j, int k);
		void setSegments(glm::uvec3 segments);
		glm::uvec3 segments();


	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<Box> BoxPtr;

}


