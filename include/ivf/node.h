#pragma once

#include <ivf/glbase.h>
#include <ivf/material.h>

namespace ivf {

	class Node : public GLBase {
	private:
		std::shared_ptr<Material> m_material;
		bool m_useMaterial;
	public:
		Node();
		void draw();

		void setMaterial(std::shared_ptr<Material> material);

		void setUseMaterial(bool flag);
		bool useMaterial();
	protected:
		virtual void doPreDraw();
		virtual void doDraw();
		virtual void doPostDraw();
	};

	typedef std::shared_ptr<Node> NodePtr;

}

