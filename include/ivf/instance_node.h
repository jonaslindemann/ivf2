#pragma once

#include <ivf/transform_node.h>

namespace ivf {

/**
 * @class InstanceNode
 * @brief Node that instances another TransformNode, sharing its geometry and properties.
 *
 * The InstanceNode class allows instancing of a TransformNode, enabling multiple nodes to share
 * the same geometry and properties while having independent transformations. This is useful for
 * efficient rendering of repeated objects in a scene.
 */
class InstanceNode : public TransformNode {
private:
    std::shared_ptr<TransformNode> m_node; ///< The instanced node.

public:
    /**
     * @brief Default constructor.
     */
    InstanceNode();

    /**
     * @brief Factory method to create a shared pointer to an InstanceNode instance.
     * @return std::shared_ptr<InstanceNode> New InstanceNode instance.
     */
    static std::shared_ptr<InstanceNode> create();

    /**
     * @brief Set the node to be instanced.
     * @param node Shared pointer to the TransformNode to instance.
     */
    void setNode(std::shared_ptr<TransformNode> node);

    /**
     * @brief Get the currently instanced node.
     * @return std::shared_ptr<TransformNode> The instanced node.
     */
    std::shared_ptr<TransformNode> node();

protected:
    /**
     * @brief Draw the instanced node with this node's transformation.
     */
    virtual void doDraw();
};

/**
 * @typedef InstanceNodePtr
 * @brief Shared pointer type for InstanceNode.
 */
typedef std::shared_ptr<InstanceNode> InstanceNodePtr;
};
