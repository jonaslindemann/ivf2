#include <ivf/overlay_node.h>

#include <ivf/draw_context.h>
#include <ivf/render_context.h>
#include <ivf/render_queue.h>
#include <ivf/renderer.h>
#include <ivf/transform_manager.h>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

using namespace ivf;

OverlayNode::OverlayNode()
{
    setRenderLayer(RenderLayer::Overlay);
    setCastShadows(false);
    setName("OverlayNode");
}

std::shared_ptr<OverlayNode> OverlayNode::create()
{
    return std::make_shared<OverlayNode>();
}

void OverlayNode::collect(RenderQueue &queue, const CollectState &state)
{
    if (!visible())
        return;
    // One item without bounds (never culled); the children are not part of the scene's queue
    DrawItem item = drawItem(state);
    item.layer = RenderLayer::Overlay;
    item.castShadows = false;
    item.bounds = BoundingBox();
    queue.add(item);
}

int OverlayNode::viewportWidth() const
{
    return m_width;
}

int OverlayNode::viewportHeight() const
{
    return m_height;
}

float OverlayNode::pixelRatio() const
{
    return m_pixelRatio;
}

void OverlayNode::onLayout(int /*width*/, int /*height*/)
{
}

const glm::mat4 &OverlayNode::sceneView() const
{
    return m_view;
}

const glm::mat4 &OverlayNode::sceneProjection() const
{
    return m_projection;
}

const glm::mat4 &OverlayNode::parentWorld() const
{
    return m_parentWorld;
}

std::optional<glm::vec3> OverlayNode::project(const glm::vec3 &p) const
{
    const glm::vec4 clip = m_projection * m_view * m_parentWorld * glm::vec4(p, 1.0f);
    if (clip.w <= 1e-6f)
        return std::nullopt;
    const glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return glm::vec3((ndc.x * 0.5f + 0.5f) * static_cast<float>(m_width),
                     (ndc.y * 0.5f + 0.5f) * static_cast<float>(m_height), ndc.z * 0.5f + 0.5f);
}

void OverlayNode::doDrawPass(const DrawContext &context)
{
    if (context.pass != RenderPass::Main)
        return;

    // Lay out in window units: on high-DPI displays the viewport has pixelRatio() times as
    // many pixels, and the pixel camera below spreads the units over all of them
    GLint viewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_pixelRatio = context.context.pixelRatio();
    m_width = static_cast<int>(std::lround(static_cast<float>(viewport[2]) / m_pixelRatio));
    m_height = static_cast<int>(std::lround(static_cast<float>(viewport[3]) / m_pixelRatio));
    if (m_width <= 0 || m_height <= 0)
        return;

    m_view = context.view;
    m_projection = context.projection;
    m_parentWorld = context.item.parentWorld;
    onLayout(m_width, m_height);

    // Pixel camera
    auto *xfm = context.context.transforms();
    const glm::mat4 model = xfm->modelMatrix();
    const glm::mat4 view = xfm->viewMatrix();
    const glm::mat4 projection = xfm->projectionMatrix();
    xfm->viewMatrix() = glm::mat4(1.0f);
    xfm->projectionMatrix() =
        glm::ortho(0.0f, static_cast<float>(m_width), 0.0f, static_cast<float>(m_height), -1000.0f, 1000.0f);

    const GLboolean depthTestWas = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blendWas = glIsEnabled(GL_BLEND);
    const GLboolean cullWas = glIsEnabled(GL_CULL_FACE);
    GLint srcRgb = GL_ONE, dstRgb = GL_ZERO, srcAlpha = GL_ONE, dstAlpha = GL_ZERO;
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Children in the order they were added (no sorting), with this node's transform
    RenderQueue queue;
    CollectState state;
    state.castShadows = false;
    const CollectState childStateValue = childState(state);
    for (auto &node : *this)
        node->collect(queue, childStateValue);

    Renderer renderer(context.context, RenderPass::Main);
    renderer.execute(queue);

    // Restore the scene's camera and state
    xfm->viewMatrix() = view;
    xfm->projectionMatrix() = projection;
    xfm->setModelMatrix(model);
    xfm->syncCameraBlock();

    if (depthTestWas)
        glEnable(GL_DEPTH_TEST);
    if (cullWas)
        glEnable(GL_CULL_FACE);
    if (blendWas)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    glBlendFuncSeparate(static_cast<GLenum>(srcRgb), static_cast<GLenum>(dstRgb), static_cast<GLenum>(srcAlpha),
                        static_cast<GLenum>(dstAlpha));
}
