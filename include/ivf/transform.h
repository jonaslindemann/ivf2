#pragma once

#include <ivf/composite_node.h>

namespace ivf {

/**
 * @class Transform
 * @brief CompositeNode that groups and transforms multiple child nodes.
 *
 * The Transform class is a convenience wrapper around CompositeNode, allowing
 * grouping and transformation (translation, rotation, scaling) of multiple child nodes
 * as a single unit in the scene graph. Inherits all functionality from CompositeNode.
 */
class Transform : public CompositeNode {
private:
    // No additional members.
public:
    /**
     * @brief Constructor.
     */
    Transform();

    /**
     * @brief Factory method to create a shared pointer to a Transform instance.
     * @return std::shared_ptr<Transform> New Transform instance.
     */
    static std::shared_ptr<Transform> create();
};

/**
 * @typedef TransformPtr
 * @brief Shared pointer type for Transform.
 */
typedef std::shared_ptr<Transform> TransformPtr;

} // namespace ivf
