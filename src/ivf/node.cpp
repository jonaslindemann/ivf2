#include <ivf/node.h>

using namespace ivf;

ivf::Node::Node()
	:m_material(nullptr),
	 m_useMaterial(true)	
{
}

void Node::draw()
{
	doPreDraw();
	doDraw();
	doPostDraw();
}

void ivf::Node::setMaterial(std::shared_ptr<Material> material)
{
	m_material = material;
}

void ivf::Node::setUseMaterial(bool flag)
{
	m_useMaterial = flag;
}

bool ivf::Node::useMaterial()
{
	return m_useMaterial;
}

void Node::doPreDraw()
{
	if ((m_material != nullptr)&&(m_useMaterial))
		m_material->apply();
}

void ivf::Node::doDraw()
{
}

void ivf::Node::doPostDraw()
{
}
