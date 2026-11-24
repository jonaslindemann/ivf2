#pragma once
/**
 * @file renderer.h
 * @brief Draws the items of a RenderQueue.
 */

#include <ivf/draw_context.h>
#include <ivf/frustum.h>
#include <ivf/render_queue.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace ivf {

class RenderContext;

/**
 * @brief Draws collected scene items.
 *
 * A renderer draws into the RenderContext it is constructed with: everything it does
 * itself (model matrix, inherited and default materials, statistics) goes through that
 * context, never through the manager singletons. Make the context current before
 * execute(), as for any OpenGL call.
 *
 * For each item the renderer sets the model matrix of the node's parent, applies the
 * inherited material and textures if the node has none of its own, and calls the
 * node's doPreDraw(), doDrawPass() (whose default calls doDraw()) and doPostDraw() - the
 * same hooks Node::draw() always used, so nodes with their own drawing code keep
 * working. Nodes without any material (own or inherited) are drawn with a neutral
 * default material.
 *
 * With transparency blending enabled (colour passes), items of the Transparent layer are
 * drawn with alpha blending and without depth writes; sort the queue first
 * (RenderQueue::sortForView()) so they come after the opaque items, back to front.
 *
 * @code
 * ivf::RenderQueue queue;
 * scene->collect(queue, ivf::CollectState{});
 * queue.sortForView(ctx.transforms()->viewPos());
 *
 * ivf::Renderer renderer(ctx);              // RenderPass::Main
 * renderer.setTransparencyBlending(true);
 * renderer.execute(queue, ivf::Frustum(projection * view));
 * ivf::logInfo(ivf::logStr(renderer.stats().drawn, " of ", renderer.stats().items, " items drawn"));
 * @endcode
 */
class Renderer {
public:
    /// Returns true for items a pass should leave out.
    using SkipFn = std::function<bool(const DrawItem &)>;

    /// Counts of the last execute() call (or, added up with +=, of several).
    struct Stats {
        std::size_t items{0};             ///< Items in the queue.
        std::size_t drawn{0};             ///< Items drawn.
        std::size_t culled{0};            ///< Items outside the frustum.
        std::size_t skipped{0};           ///< Items left out by the skip filter.
        std::uint64_t drawCalls{0};       ///< OpenGL draw calls issued (RenderContext::Counters).
        std::uint64_t programChanges{0};  ///< Switches to another shader program.
        std::uint64_t materialApplies{0}; ///< Materials applied (own, inherited or default).

        /** @brief Add the counts of another execute() call (e.g. one per shadow light). */
        Stats &operator+=(const Stats &other)
        {
            items += other.items;
            drawn += other.drawn;
            culled += other.culled;
            skipped += other.skipped;
            drawCalls += other.drawCalls;
            programChanges += other.programChanges;
            materialApplies += other.materialApplies;
            return *this;
        }
    };

    /**
     * @brief Create a renderer that draws into @p context.
     * @param context Render context of the OpenGL context to draw into; must outlive the renderer.
     * @param pass Pass reported to the nodes in DrawContext::pass.
     */
    explicit Renderer(RenderContext &context, RenderPass pass = RenderPass::Main)
        : m_context(&context), m_pass(pass)
    {}

    /** @brief The render context this renderer draws into. */
    [[nodiscard]] RenderContext &context() const
    {
        return *m_context;
    }

    /** @brief Set the pass reported to the nodes (DrawContext::pass). */
    void setPass(RenderPass pass)
    {
        m_pass = pass;
    }

    /** @brief See setPass(). */
    [[nodiscard]] RenderPass pass() const
    {
        return m_pass;
    }

    /**
     * @brief Draw the items of @p queue in order, without culling.
     *
     * The model matrix is restored afterwards.
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

    /**
     * @brief Draw the Transparent layer with alpha blending and without depth writes.
     *
     * Enable it for colour passes; leave it off for depth-only (shadow) and id (selection)
     * passes. Default: off.
     */
    void setTransparencyBlending(bool enabled)
    {
        m_transparencyBlending = enabled;
    }

    /** @brief See setTransparencyBlending(). */
    [[nodiscard]] bool transparencyBlending() const
    {
        return m_transparencyBlending;
    }

    /** @brief Counts of the last execute() call. */
    [[nodiscard]] const Stats &stats() const
    {
        return m_stats;
    }

private:
    RenderContext *m_context; ///< Never null.
    RenderPass m_pass;
    Stats m_stats;
    bool m_transparencyBlending{false};
    std::shared_ptr<Material> m_defaultMaterial; ///< For nodes without any material.
};

/**
 * @brief Counts of one frame's passes, as GLFWSceneWindow::renderStats() reports them.
 *
 * The shadow and reflection counts add up all lights and reflections of the frame.
 * Selection counts are those of the last selection render, which only happens after
 * input or camera changes.
 */
struct RenderStats {
    Renderer::Stats shadow;     ///< All shadow maps (LightManager::shadowStats()).
    Renderer::Stats reflection; ///< All planar reflections (PlanarReflection::stats()).
    Renderer::Stats main;       ///< The camera's image.
    Renderer::Stats selection;  ///< The last selection buffer render.

    /** @brief Sum of the passes drawn every frame (shadow, reflection and main). */
    [[nodiscard]] Renderer::Stats frame() const
    {
        Renderer::Stats total = shadow;
        total += reflection;
        total += main;
        return total;
    }
};

} // namespace ivf
