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

std::shared_ptr<TransformNode> ivf::InstanceNode::node() const
{
	return m_node;
}

void ivf::InstanceNode::collect(RenderQueue &queue, const CollectState &state)
{
	if (!visible() || !m_node)
		return;
	m_node->collect(queue, childState(state));
}

void ivf::InstanceNode::doDraw()
{
	if (m_node!=nullptr)
		m_node->draw();
}
