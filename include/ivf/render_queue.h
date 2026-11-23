#pragma once
/**
 * @file render_queue.h
 * @brief Draw items collected from the scene graph, drawn by ivf::Renderer.
 *
 * Drawing a scene has two phases: Node::collect() walks the scene graph once and adds
 * a DrawItem per drawable node to a RenderQueue; the Renderer then draws the items.
 * Containers (CompositeNode, InstanceNode) add no item of their own; they pass their
 * transform, material and textures down to their children through CollectState.
 *
 * Node::draw() does both phases, so existing code that calls scene->draw() keeps working.
 */

#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace ivf {

class Node;
class Material;

/**
 * @brief State passed down the scene graph while collecting.
 */
struct CollectState {
    glm::mat4 world{1.0f};              ///< Model matrix accumulated from the ancestors.
    std::shared_ptr<Material> material; ///< Material of the nearest ancestor that has one.
    Node *textureSource{nullptr};       ///< Nearest ancestor that has textures.
};

/**
 * @brief One drawable node with everything needed to draw it.
 *
 * Items refer to the nodes of the scene; a queue is only valid while the scene is
 * unchanged (it is rebuilt every time the scene is drawn).
 */
struct DrawItem {
    Node *node{nullptr};                ///< The node to draw.
    glm::mat4 parentWorld{1.0f};        ///< Model matrix before the node's own transform.
    glm::mat4 world{1.0f};              ///< Model matrix including the node's own transform.
    std::shared_ptr<Material> material; ///< Inherited material, used if the node has none of its own.
    Node *textureSource{nullptr};       ///< Inherited textures, used if the node has none of its own.
    BoundingBox bounds;                 ///< World-space bounds; invalid if the node has none.
};

/**
 * @brief The list of draw items for one drawing of a scene.
 */
class RenderQueue {
public:
    /** @brief Add an item (called from Node::collect()). */
    void add(const DrawItem &item)
    {
        m_items.push_back(item);
    }

    /** @brief Remove all items. */
    void clear()
    {
        m_items.clear();
    }

    /** @brief The items in the order they were collected. */
    [[nodiscard]] std::span<const DrawItem> items() const
    {
        return m_items;
    }

    /** @brief Union of the valid item bounds (invalid if no item has bounds). */
    [[nodiscard]] BoundingBox bounds() const
    {
        BoundingBox box;
        for (const auto &item : m_items)
            if (item.bounds.isValid())
                box.add(item.bounds);
        return box;
    }

    /** @brief Number of items. */
    [[nodiscard]] std::size_t size() const
    {
        return m_items.size();
    }

private:
    std::vector<DrawItem> m_items;
};

} // namespace ivf
