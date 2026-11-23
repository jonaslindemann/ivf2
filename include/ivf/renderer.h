#pragma once
/**
 * @file renderer.h
 * @brief Draws the items of a RenderQueue.
 */

#include <ivf/frustum.h>
#include <ivf/render_queue.h>

#include <cstddef>
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

    /// Counts of the last execute() call.
    struct Stats {
        std::size_t items{0};   ///< Items in the queue.
        std::size_t drawn{0};   ///< Items drawn.
        std::size_t culled{0};  ///< Items outside the frustum.
        std::size_t skipped{0}; ///< Items left out by the skip filter.
    };

    /**
     * @brief Draw the items of @p queue in order, without culling.
     *
     * Uses the current RenderContext's managers. The model matrix is restored afterwards.
     * @param queue Items to draw.
     * @param skip Optional filter: items for which it returns true are not drawn.
     */
    void execute(const RenderQueue &queue, const SkipFn &skip = {});

    /**
     * @brief Draw the items of @p queue whose bounds intersect @p frustum.
     *
     * Items without bounds are always drawn.
     * @param queue Items to draw.
     * @param frustum View volume of the pass (e.g. Frustum(projection * view)).
     * @param skip Optional filter: items for which it returns true are not drawn.
     */
    void execute(const RenderQueue &queue, const Frustum &frustum, const SkipFn &skip = {});

    /** @brief Counts of the last execute() call. */
    [[nodiscard]] const Stats &stats() const
    {
        return m_stats;
    }

private:
    Stats m_stats;
};

} // namespace ivf
