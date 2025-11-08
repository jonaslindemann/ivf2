#pragma once

#include <ivf/line_trace.h>

namespace ivf {

/**
 * @class TraceNode
 * @brief LineTrace node that traces the path of a TransformNode.
 *
 * The TraceNode class extends LineTrace to visualize the path or trajectory of a
 * TransformNode in the scene. It maintains a reference to the node being traced
 * and can update the trace as the node moves. Inherits from LineTrace, allowing
 * color configuration and efficient refresh for real-time visualization.
 */
class TraceNode : public LineTrace {
private:
    TransformNodePtr m_node; ///< Node whose path is being traced.

public:
    /**
     * @brief Constructor.
     * @param numVertices Number of vertices in the trace.
     */
    TraceNode(int numVertices, bool useColor);

    /**
     * @brief Destructor.
     */
    virtual ~TraceNode();

    /**
     * @brief Factory method to create a shared pointer to a TraceNode instance.
     * @param numVertices Number of vertices in the trace.
     * @return std::shared_ptr<TraceNode> New TraceNode instance.
     */
    static std::shared_ptr<TraceNode> create(int numVertices, bool useColor);

    /**
     * @brief Set the node to trace.
     * @param node TransformNode pointer.
     */
    void setNode(TransformNodePtr node);

    /**
     * @brief Get the currently traced node.
     * @return TransformNodePtr Traced node pointer.
     */
    TransformNodePtr node();

protected:
    /**
     * @brief Called before drawing the trace node.
     */
    virtual void doPreDraw();

    /**
     * @brief Called after drawing the trace node.
     */
    virtual void doPostDraw();
};

/**
 * @typedef TraceNodePtr
 * @brief Shared pointer type for TraceNode.
 */
typedef std::shared_ptr<TraceNode> TraceNodePtr;

}; // namespace ivf
