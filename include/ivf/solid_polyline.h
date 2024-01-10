#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <gml/gml.hpp>

#include <generator/PolyLinePath.hpp>

namespace ivf {

	class SolidPolyLine : public MeshNode {
	private:
		double m_radius;
		generator::PolyLinePath m_polyPath;
	public:

		SolidPolyLine(double radius = 0.1);

		static std::shared_ptr<SolidPolyLine> create(double radius = 0.1);

		void setRadius(double radius);

	protected:
		virtual void doSetup();
	};

	typedef std::shared_ptr<SolidPolyLine> SolidPolyLinePtr;

}


