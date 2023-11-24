#pragma once

#include <ivf/mesh_node.h>
#include <ivf/utils.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

#include <generator/ParametricPath.hpp>

namespace ivf {

	class PointPath {
	private:

		int m_segments;

		using Impl = generator::ParametricPath;
		Impl parametricPath_;


	public:
		PointPath(int segments, std::vector<glm::vec3> points);

		using Edges = typename Impl::Edges;

		Edges edges() const noexcept { return parametricPath_.edges(); }

		using Vertices = typename Impl::Vertices;

		Vertices vertices() const noexcept { return parametricPath_.vertices(); }
	};

}


