#pragma once

#include <ivf/line_trace.h>

namespace ivf {

class TraceNode : public LineTrace {
private:
    TransformNodePtr m_node;

public:
    TraceNode(int numVertices);
    virtual ~TraceNode();

    static std::shared_ptr<TraceNode> create(int numVertices);

    void setNode(TransformNodePtr node);
    TransformNodePtr node();

protected:
    virtual void doPreDraw();
    virtual void doPostDraw();
};

typedef std::shared_ptr<TraceNode> TraceNodePtr;

}; // namespace ivf
