#include <ivf/composite_node.h>

using namespace std;
using namespace ivf;

CompositeNode::CompositeNode()
{
	this->setUseMaterial(false);
}

std::shared_ptr<CompositeNode> ivf::CompositeNode::create()
{
	return std::make_shared<CompositeNode>();
}

void CompositeNode::add(std::shared_ptr<Node> node)
{
	m_nodes.push_back(node);
}

std::vector<std::shared_ptr<Node>> ivf::CompositeNode::nodes()
{
	return m_nodes;
}

void ivf::CompositeNode::clear()
{
	 m_nodes.clear();
}

void ivf::CompositeNode::doDraw()
{
	for (auto node : m_nodes)
		node->draw();
}
