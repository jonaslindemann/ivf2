#pragma once

#include <ivf/transform_node.h>

#include <vector>

namespace ivf {

/**
 * @class CompositeNode
 * @brief Node that can contain and manage multiple child nodes as a group.
 *
 * The CompositeNode class allows grouping of multiple Node objects, enabling hierarchical scene
 * management. It inherits transformation capabilities from TransformNode and provides methods
 * for adding, removing, and iterating over child nodes. CompositeNode supports visitor traversal
 * and can be used as a root or intermediate node in a scene graph.
 */
class CompositeNode : public TransformNode {
private:
    std::vector<std::shared_ptr<Node>> m_nodes; ///< List of child nodes.
    bool m_singleObjectId{false};               ///< If true, all children share a single object ID.

public:
    /**
     * @brief Default constructor.
     */
    CompositeNode();

    /**
     * @brief Destructor.
     */
    virtual ~CompositeNode() override;

    /**
     * @brief Factory method to create a shared pointer to a CompositeNode instance.
     * @return std::shared_ptr<CompositeNode> New CompositeNode instance.
     */
    static std::shared_ptr<CompositeNode> create();

    /**
     * @brief Add a child node to the composite node.
     * @param node Node to add.
     */
    void add(std::shared_ptr<Node> node);

    /**
     * @brief Get the list of child nodes.
     * @return std::vector<std::shared_ptr<Node>> Vector of child nodes.
     */
    std::vector<std::shared_ptr<Node>> nodes();

    /**
     * @brief Remove all child nodes from the composite node.
     */
    void clear();

    /**
     * @brief Remove a specific child node from the composite node.
     * @param node Node to remove.
     */
    void remove(std::shared_ptr<Node> node);

    /**
     * @brief Get the child node at the specified index.
     * @param index Index of the child node.
     * @return NodePtr Shared pointer to the child node.
     */
    NodePtr at(size_t index);

    /**
     * @brief Get the number of child nodes.
     * @return size_t Number of child nodes.
     */
    size_t count();

    /**
     * @brief Store the current positions of all child nodes.
     */
    void storeChildrenPos();

    /**
     * @brief Restore the previously stored positions of all child nodes.
     */
    void restoreChildrenPos();

    /**
     * @brief Compute the aggregate bounding box of all child nodes.
     * @param includeInvisible Whether to include invisible nodes in the computation.
     * @return BoundingBox The aggregate bounding box in this node's local space.
     */
    BoundingBox computeChildrenBoundingBox(bool includeInvisible = false) const;

    /**
     * @brief Get the local bounding box, including children.
     * @return BoundingBox The local bounding box including all children.
     */
    virtual BoundingBox localBoundingBox() const override;

    /**
     * @brief Get all TransformNode-derived children recursively.
     * @param results Vector to store the results.
     * @param includeInvisible Whether to include invisible nodes.
     */
    void getTransformNodes(std::vector<std::shared_ptr<TransformNode>>& results, bool includeInvisible = false) const;

    /**
     * @brief Get the world positions of all TransformNode-derived children.
     * @param includeInvisible Whether to include invisible nodes.
     * @return std::vector<glm::vec3> Vector of world positions.
     */
    std::vector<glm::vec3> getChildWorldPositions(bool includeInvisible = false) const;

    /**
     * @brief Get the world-space bounding box of this composite node, including all children.
     * @return BoundingBox The world-space bounding box including children.
     */
    virtual BoundingBox worldBoundingBox() const override;

    /**
     * @brief Accept a node visitor for traversal or processing.
     * @param visitor Pointer to the NodeVisitor.
     */
    virtual void accept(NodeVisitor *visitor) override;

    // Iterator type aliases
    using iterator = std::vector<NodePtr>::iterator;             ///< Iterator for child nodes.
    using const_iterator = std::vector<NodePtr>::const_iterator; ///< Const iterator for child nodes.

    // Iterator methods for range-based for loops
    /**
     * @brief Get iterator to the beginning of the child node list.
     * @return iterator Iterator to the first child node.
     */
    iterator begin()
    {
        return m_nodes.begin();
    }
    /**
     * @brief Get iterator to the end of the child node list.
     * @return iterator Iterator to one past the last child node.
     */
    iterator end()
    {
        return m_nodes.end();
    }

    // Const iterator methods
    /**
     * @brief Get const iterator to the beginning of the child node list.
     * @return const_iterator Const iterator to the first child node.
     */
    const_iterator begin() const
    {
        return m_nodes.begin();
    }
    /**
     * @brief Get const iterator to the end of the child node list.
     * @return const_iterator Const iterator to one past the last child node.
     */
    const_iterator end() const
    {
        return m_nodes.end();
    }

    // Optional: cbegin/cend for explicit const iteration
    /**
     * @brief Get const iterator to the beginning of the child node list.
     * @return const_iterator Const iterator to the first child node.
     */
    const_iterator cbegin() const
    {
        return m_nodes.cbegin();
    }
    /**
     * @brief Get const iterator to the end of the child node list.
     * @return const_iterator Const iterator to one past the last child node.
     */
    const_iterator cend() const
    {
        return m_nodes.cend();
    }

protected:
    /**
     * @brief Draw the composite node and its children.
     */
    virtual void doDraw();

    /**
     * @brief Enumerate object IDs for selection, starting from a given ID.
     * @param startId Starting object ID.
     * @return uint32_t Next available object ID after enumeration.
     */
    virtual uint32_t doEnumerateIds(uint32_t startId);
};

/**
 * @typedef CompositeNodePtr
 * @brief Shared pointer type for CompositeNode.
 */
typedef std::shared_ptr<CompositeNode> CompositeNodePtr;
} // namespace ivf
