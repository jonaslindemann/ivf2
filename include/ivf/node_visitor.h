#pragma once
#pragma once

/**
 * @file node_visitor.h
 * @brief Defines the NodeVisitor interface and concrete visitors for scene graph traversal in the ivf library.
 */

#include <memory>
#include <map>

namespace ivf {

class Node;
class CompositeNode;

/**
 * @class NodeVisitor
 * @brief Abstract base class for implementing the Visitor pattern on Node objects.
 *
 * The NodeVisitor interface allows external operations to be performed on Node objects
 * without modifying their classes. Concrete visitors should implement the visit() method.
 */
class NodeVisitor {
public:
    /**
     * @brief Visit a node.
     * @param node Pointer to the node being visited.
     */
    virtual void visit(Node *node) = 0;
};

/**
 * @class PrintVisitor
 * @brief Concrete visitor that prints information about nodes.
 *
 * Implements the NodeVisitor interface to perform printing or logging operations on nodes.
 */
class PrintVisitor : public NodeVisitor {
private:
public:
    /**
     * @brief Visit a node and print its information.
     * @param node Pointer to the node being visited.
     */
    virtual void visit(Node *node) override;
};

/**
 * @typedef NodeMap
 * @brief Type alias for a map from node object IDs to Node pointers.
 */
typedef std::map<unsigned int, Node *> NodeMap;

/**
 * @class MapVisitor
 * @brief Concrete visitor that builds a map of node object IDs to Node pointers.
 *
 * Implements the NodeVisitor interface to collect nodes into a map for quick lookup by object ID.
 */
class MapVisitor : public NodeVisitor {
private:
    NodeMap m_nodeMap; ///< Internal map of object IDs to Node pointers.

public:
    /**
     * @brief Visit a node and add it to the map.
     * @param node Pointer to the node being visited.
     */
    virtual void visit(Node *node) override;

    /**
     * @brief Take ownership of the internal map and clear it.
     * @return The internal NodeMap, moved out of the visitor.
     */
    NodeMap takeMap();

    /**
     * @brief Get a const reference to the internal map.
     * @return Const reference to the NodeMap.
     */
    const NodeMap &getMap() const;

    /**
     * @brief Get a copy of the internal map.
     * @return A copy of the NodeMap.
     */
    NodeMap getMapCopy() const;
};

} // namespace ivf
