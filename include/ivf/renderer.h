#pragma once
/**
 * @file renderer.h
 * @brief Draws the items of a RenderQueue.
 */

#include <ivf/render_queue.h>

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
    /**
     * @brief Draw all items of @p queue in order.
     *
     * Uses the current RenderContext's managers. The model matrix is restored afterwards.
     */
    void execute(const RenderQueue &queue);
};

} // namespace ivf
