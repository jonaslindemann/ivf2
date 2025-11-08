#include <ivf/trace_node.h>

#include <ivf/light_manager.h>

using namespace ivf;

TraceNode::TraceNode(int numVertices, bool useColor) : LineTrace(numVertices, useColor)
{
    this->doSetup();
    this->setName("TraceNode");
}

TraceNode::~TraceNode()
{}

std::shared_ptr<TraceNode> ivf::TraceNode::create(int numVertices, bool useColor)
{
    return std::make_shared<TraceNode>(numVertices, useColor);
}

void ivf::TraceNode::setNode(TransformNodePtr node)
{
    m_node = node;
}

TransformNodePtr ivf::TraceNode::node()
{
    return m_node;
}

void ivf::TraceNode::doPreDraw()
{
    if (m_node == nullptr)
        return;

    auto nodePos = m_node->pos();

    this->add(nodePos);

    LineTrace::doPreDraw();
}

void ivf::TraceNode::doPostDraw()
{
    LineTrace::doPostDraw();
}
