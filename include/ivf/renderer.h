#pragma once
/**
 * @file renderer.h
 * @brief Draws the items of a RenderQueue.
 */

#include <ivf/render_queue.h>

#include <functional>

namespace ivf {

/**
 * @brief Draws collected scene items.
 *
 * For each item the renderer sets the model matrix of the node's parent, applies the
 * inherited material and textures if the node has none of its own, and calls the
 * node's doPreDraw()/doDraw()/doPostDraw() - the same hooks Node::draw() always used,
 * so nodes with their own drawing code keep working.
 */
class Renderer {
public:
    /// Returns true for items a pass should leave out.
    using SkipFn = std::function<bool(const DrawItem &)>;

    /**
     * @brief Draw the items of @p queue in order.
     *
     * Uses the current RenderContext's managers. The model matrix is restored afterwards.
     * @param queue Items to draw.
     * @param skip Optional filter: items for which it returns true are not drawn.
     */
    void execute(const RenderQueue &queue, const SkipFn &skip = {});
};

} // namespace ivf
