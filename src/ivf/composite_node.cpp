#include <ivf/composite_node.h>

#include <algorithm>

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

void ivf::CompositeNode::remove(std::shared_ptr<Node> node)
{
    auto it = std::find(m_nodes.begin(), m_nodes.end(), node);
    if (it != m_nodes.end())
        m_nodes.erase(it);
}

NodePtr ivf::CompositeNode::at(size_t index)
{
    return nodes().at(index);
}

void ivf::CompositeNode::doDraw()
{
    for (auto node : m_nodes)
        node->draw();
}
