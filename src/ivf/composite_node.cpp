#include <ivf/composite_node.h>

#include <algorithm>

using namespace std;
using namespace ivf;

CompositeNode::CompositeNode() : TransformNode(), m_singleObjectId(false)
{
    this->setUseMaterial(false);
}

ivf::CompositeNode::~CompositeNode()
{
    clear();
}

std::shared_ptr<CompositeNode> ivf::CompositeNode::create()
{
    auto compositeNode = std::make_shared<CompositeNode>();
    compositeNode->doSetup();
    return compositeNode;
}

void CompositeNode::add(std::shared_ptr<Node> node)
{
    m_nodes.push_back(node);
    node->setParent(shared_from_this());
}

std::vector<std::shared_ptr<Node>> ivf::CompositeNode::nodes()
{
    return m_nodes;
}

void ivf::CompositeNode::clear()
{
    for (auto node : m_nodes)
    {
        // If the node has a parent, remove this composite node from its parent
        if (node->parent())
            node->setParent(nullptr);
    }
    m_nodes.clear();
}

void ivf::CompositeNode::remove(std::shared_ptr<Node> node)
{
    auto it = std::find(m_nodes.begin(), m_nodes.end(), node);
    if (it != m_nodes.end())
    {
        // If the node has a parent, remove this composite node from its parent
        if (node->parent())
            node->setParent(nullptr);

        m_nodes.erase(it);
    }
}

NodePtr ivf::CompositeNode::at(size_t index)
{
    return nodes().at(index);
}

size_t ivf::CompositeNode::count()
{
    return m_nodes.size();
}

void ivf::CompositeNode::storeChildrenPos()
{
    for (auto node : m_nodes)
    {
        auto transformNode = std::dynamic_pointer_cast<TransformNode>(node);
        if (transformNode)
            transformNode->storePos();
    }
}

void ivf::CompositeNode::restoreChildrenPos()
{
    for (auto node : m_nodes)
    {
        auto transformNode = std::dynamic_pointer_cast<TransformNode>(node);
        if (transformNode)
            transformNode->restorePos();
    }
}

void ivf::CompositeNode::accept(NodeVisitor *visitor)
{
    visitor->visit(this);

    for (auto node : m_nodes)
        node->accept(visitor);
}

void ivf::CompositeNode::doDraw()
{
    for (auto node : m_nodes)
        node->draw();
}

uint32_t ivf::CompositeNode::doEnumerateIds(uint32_t startId)
{
    if (m_singleObjectId)
    {
        for (auto node : m_nodes)
            node->setObjectId(startId);
        return startId + 1;
    }
    else
    {
        uint32_t nextId = startId;
        for (auto node : m_nodes)
            nextId = node->enumerateIds(nextId);
        return nextId;
    }
}
