#include <ivf/node_visitor.h>

#include <iostream>

#include <ivf/node.h>

using namespace ivf;

void PrintVisitor::visit(Node *node)
{
    std::cout << typeid(*node).name() << std::endl;
    std::cout << "ObjectID = " << node->objectId() << std::endl;
}

void MapVisitor::visit(Node *node)
{
    m_nodeMap[node->objectId()] = node;
}

NodeMap MapVisitor::takeMap()
{
    return std::move(m_nodeMap);
}

const NodeMap &MapVisitor::getMap() const
{
    return m_nodeMap;
}

// Return copy if caller needs their own version
NodeMap MapVisitor::getMapCopy() const
{
    return m_nodeMap;
}
