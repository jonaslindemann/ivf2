#pragma once
/**
 * @file draw_context.h
 * @brief What a node is being drawn for: render context, pass and camera (ivf::DrawContext).
 */

#include <glm/glm.hpp>

namespace ivf {

class RenderContext;
struct DrawItem;

/**
 * @brief The pass a Renderer draws for.
 *
 * Nodes whose drawing depends on the pass (for example, skipping an expensive effect in
 * shadow maps) read it from DrawContext::pass in Node::doDrawPass().
 */
enum class RenderPass {
    Main,       ///< The camera's colour image.
    Shadow,     ///< Depth of shadow casters as seen by a light (uniform `lightSpaceMatrix`).
    Reflection, ///< The scene mirrored in a PlanarReflection's plane.
    Selection   ///< Object ids for picking (SelectionManager).
};

/** @brief Name of a pass ("main", "shadow", "reflection", "selection"). */
[[nodiscard]] constexpr const char *renderPassName(RenderPass pass)
{
    switch (pass)
    {
    case RenderPass::Main:
        return "main";
    case RenderPass::Shadow:
        return "shadow";
    case RenderPass::Reflection:
        return "reflection";
    case RenderPass::Selection:
        return "selection";
    }
    return "unknown";
}

/**
 * @brief Everything the Renderer knows about the drawing of one item.
 *
 * Passed to Node::doDrawPass(). Use @c context instead of the manager singletons
 * (`context.shaders()`, `context.lights()`, ...) so the node draws into the context the
 * renderer was given.
 */
struct DrawContext {
    RenderContext &context; ///< Render context (OpenGL context) being drawn into.
    RenderPass pass;        ///< Pass being drawn.
    const DrawItem &item;   ///< The item being drawn (world matrix, bounds, layer, ...).
    glm::mat4 view;         ///< View matrix of the pass (the mirrored camera in Reflection).
    glm::mat4 projection;   ///< Projection matrix of the pass. In the Shadow pass, view and
                            ///< projection are the camera's; geometry is transformed by the
                            ///< light's `lightSpaceMatrix`.
};

} // namespace ivf
