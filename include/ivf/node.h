#pragma once

#include <ivf/glbase.h>
#include <ivf/material.h>
#include <ivf/texture.h>

namespace ivf {

	class Node : public GLBase {
	private:
		std::shared_ptr<Material> m_material;
		std::shared_ptr<Texture> m_texture;
		bool m_useMaterial;
		bool m_useTexture;
	public:
		Node();
		void draw();

		void setMaterial(std::shared_ptr<Material> material);
		void setTexture(std::shared_ptr<Texture> texture);

		void setUseMaterial(bool flag);
		bool useMaterial();

		void setUseTexture(bool flag);
		bool useTexture();
	protected:
		virtual void doPreDraw();
		virtual void doDraw();
		virtual void doPostDraw();
	};

	typedef std::shared_ptr<Node> NodePtr;

}

