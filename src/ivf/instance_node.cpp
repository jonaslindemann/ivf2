#include <ivf/instance_node.h>

using namespace std;
using namespace ivf;

InstanceNode::InstanceNode()
{
	this->setUseMaterial(true);
	this->setUseTexture(false);
}

std::shared_ptr<InstanceNode> ivf::InstanceNode::create()
{
	return std::make_shared<InstanceNode>();
}

void InstanceNode::setNode(std::shared_ptr<TransformNode> node)
{
	m_node = node;
	m_node->setUseTransform(false);
}

std::shared_ptr<TransformNode> ivf::InstanceNode::node()
{
	return m_node;
}

void ivf::InstanceNode::doDraw()
{
	if (m_node!=nullptr)
		m_node->draw();
}
