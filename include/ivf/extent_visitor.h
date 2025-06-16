#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ivf/node_visitor.h>
#include <ivf/transform_node.h>
#include <ivf/matrix_stack.h>
#include <ivf/bounding_box.h>

namespace ivf {

/**
 * @class ExtentVisitor
 * @brief Node visitor for computing the axis-aligned bounding box (extent) of a scene graph.
 *
 * The ExtentVisitor traverses a scene graph and accumulates the transformed bounding box
 * of all nodes, taking into account their transformations. This is useful for determining
 * the spatial extent of a scene or subgraph for view fitting, culling, or collision detection.
 */
class ExtentVisitor : public NodeVisitor {
    MatrixStack m_matrixStack; ///< Stack for managing transformation matrices during traversal.
    BoundingBox m_bbox;        ///< Accumulated bounding box of all visited nodes.

public:
    /**
     * @brief Default constructor.
     */
    ExtentVisitor();

    /**
     * @brief Visit a node and update the bounding box.
     * @param node Pointer to the node to visit.
     */
    void visit(Node *node);

private:
    /**
     * @brief Handle transformation nodes during traversal.
     * @param node Pointer to the TransformNode.
     */
    void handleTransformNode(TransformNode *node);

public:
    /**
     * @brief Get the computed bounding box after traversal.
     * @return BoundingBox The accumulated bounding box.
     */
    BoundingBox bbox() const;
};
} // namespace ivf
