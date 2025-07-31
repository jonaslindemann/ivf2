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

// Forward declarations
class CompositeNode;

/**
 * @class ExtentVisitor
 * @brief Node visitor for computing the axis-aligned bounding box (extent) of a scene graph.
 *
 * The ExtentVisitor traverses a scene graph and accumulates the transformed bounding box
 * of all nodes, taking into account their transformations. This is useful for determining
 * the spatial extent of a scene or subgraph for view fitting, culling, or collision detection.
 * Enhanced to work with the improved bounding box system.
 */
class ExtentVisitor : public NodeVisitor {
    MatrixStack m_matrixStack; ///< Stack for managing transformation matrices during traversal.
    BoundingBox m_bbox;        ///< Accumulated bounding box of all visited nodes.
    bool m_includeInvisible;   ///< Whether to include invisible nodes.

public:
    /**
     * @brief Default constructor.
     * @param includeInvisible Whether to include invisible nodes in computation.
     */
    ExtentVisitor(bool includeInvisible = false);

    /**
     * @brief Visit a node and update the bounding box.
     * @param node Pointer to the node to visit.
     */
    virtual void visit(Node *node) override;

    /**
     * @brief Get the computed bounding box after traversal.
     * @return BoundingBox The accumulated bounding box.
     */
    BoundingBox bbox() const;

    /**
     * @brief Reset the visitor for a new traversal.
     */
    void reset();

    /**
     * @brief Set whether to include invisible nodes.
     * @param include True to include invisible nodes.
     */
    void setIncludeInvisible(bool include);

    /**
     * @brief Check if invisible nodes are included.
     * @return bool True if invisible nodes are included.
     */
    bool includeInvisible() const;

private:
    /**
     * @brief Handle transformation nodes during traversal.
     * @param node Pointer to the TransformNode.
     */
    void handleTransformNode(TransformNode *node);

    /**
     * @brief Handle composite nodes during traversal.
     * @param node Pointer to the CompositeNode.
     */
    void handleCompositeNode(CompositeNode *node);
};

/**
 * @class PositionVisitor
 * @brief Node visitor for collecting world positions of TransformNode-derived nodes.
 *
 * This visitor traverses a scene graph and collects the world positions of all
 * TransformNode-derived nodes, useful for spatial analysis and positioning operations.
 */
class PositionVisitor : public NodeVisitor {
    MatrixStack m_matrixStack;           ///< Stack for managing transformation matrices.
    std::vector<glm::vec3> m_positions;  ///< Collected world positions.
    bool m_includeInvisible;             ///< Whether to include invisible nodes.

public:
    /**
     * @brief Constructor.
     * @param includeInvisible Whether to include invisible nodes.
     */
    PositionVisitor(bool includeInvisible = false);

    /**
     * @brief Visit a node and collect its position if it's a TransformNode.
     * @param node Pointer to the node to visit.
     */
    virtual void visit(Node *node) override;

    /**
     * @brief Get the collected world positions.
     * @return const std::vector<glm::vec3>& Vector of world positions.
     */
    const std::vector<glm::vec3>& positions() const;

    /**
     * @brief Reset the visitor for a new traversal.
     */
    void reset();

    /**
     * @brief Set whether to include invisible nodes.
     * @param include True to include invisible nodes.
     */
    void setIncludeInvisible(bool include);

private:
    /**
     * @brief Handle transformation nodes during traversal.
     * @param node Pointer to the TransformNode.
     */
    void handleTransformNode(TransformNode *node);

    /**
     * @brief Handle composite nodes during traversal.
     * @param node Pointer to the CompositeNode.
     */
    void handleCompositeNode(CompositeNode *node);
};

} // namespace ivf
