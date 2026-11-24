#include <ivf/renderer.h>

#include <ivf/material.h>
#include <ivf/mesh_node.h>
#include <ivf/node.h>
#include <ivf/render_context.h>
#include <ivf/transform_manager.h>

#include <glad/glad.h>

using namespace ivf;

void Renderer::execute(const RenderQueue &queue, const SkipFn &skip)
{
    execute(queue, Frustum(), skip);
}

void Renderer::execute(const RenderQueue &queue, const Frustum &frustum, const SkipFn &skip)
{
    RenderContext &ctx = *m_context;
    auto *xfm = ctx.transforms();
    xfm->syncCameraBlock(); // view/projection may have been changed through their references
    const glm::mat4 start = xfm->modelMatrix();
    const glm::mat4 view = xfm->viewMatrix();
    const glm::mat4 projection = xfm->projectionMatrix();

    // Statistics: differences of the context's running totals over this call
    GLint boundProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &boundProgram);
    ctx.resetProgramTracking(static_cast<GLuint>(boundProgram));
    const RenderContext::Counters before = ctx.counters();

    m_stats = Stats{};
    m_stats.items = queue.size();

    if (!m_defaultMaterial)
    {
        // Neutral, lit light grey (instead of whatever the previous node left applied)
        MaterialProps props;
        props.diffuseColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        m_defaultMaterial = Material::create(props);
    }

    // Blending state for the Transparent layer, restored at the end
    bool transparentState = false;
    GLboolean blendWas = GL_FALSE, depthMaskWas = GL_TRUE;
    GLint srcRgb = GL_ONE, dstRgb = GL_ZERO, srcAlpha = GL_ONE, dstAlpha = GL_ZERO;
    auto setTransparentState = [&](bool on) {
        if (on == transparentState)
            return;
        if (on)
        {
            blendWas = glIsEnabled(GL_BLEND);
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskWas);
            glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb);
            glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
            glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha);
            glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
        }
        else
        {
            if (blendWas)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);
            glBlendFuncSeparate(static_cast<GLenum>(srcRgb), static_cast<GLenum>(dstRgb),
                                static_cast<GLenum>(srcAlpha), static_cast<GLenum>(dstAlpha));
            glDepthMask(depthMaskWas);
        }
        transparentState = on;
    };

    for (const DrawItem &item : queue.items())
    {
        Node *node = item.node;
        if (node == nullptr)
            continue;
        if (skip && skip(item))
        {
            ++m_stats.skipped;
            continue;
        }
        if (!frustum.intersects(item.bounds))
        {
            ++m_stats.culled;
            continue;
        }
        ++m_stats.drawn;

        // The node applies its own transform on top of its parent's (TransformNode::doPreDraw)
        xfm->setModelMatrix(item.parentWorld);

        setTransparentState(m_transparencyBlending && item.layer == RenderLayer::Transparent);

        // Material and textures of the nearest ancestor, unless the node has its own;
        // the default material if there is none at all
        const bool ownMaterial = node->m_material && node->m_useMaterial;
        const std::shared_ptr<Material> &inherited = item.material ? item.material : m_defaultMaterial;
        const bool inheritMaterial = !ownMaterial;
        const bool inheritTextures = item.textureSource != nullptr && item.textureSource != node &&
                                     !node->m_useTexture;
        if (inheritMaterial)
        {
            inherited->apply();
            ctx.countMaterialApply();
        }
        if (inheritTextures)
            item.textureSource->bindTextures();

        const DrawContext drawContext{ctx, m_pass, item, view, projection};
        auto drawNode = [node, &drawContext] {
            node->doPreDraw();
            if (node->m_visible)
                node->doDrawPass(drawContext);
            node->doPostDraw();
        };

        if (transparentState && dynamic_cast<MeshNode *>(node) != nullptr)
        {
            // Closed transparent geometry: inside (back faces) first, then the outside, so
            // the triangle order within the mesh cannot put far faces over near ones
            const GLboolean cullWas = glIsEnabled(GL_CULL_FACE);
            GLint cullModeWas = GL_BACK;
            glGetIntegerv(GL_CULL_FACE_MODE, &cullModeWas);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            drawNode();
            glCullFace(GL_BACK);
            drawNode();
            glCullFace(static_cast<GLenum>(cullModeWas));
            if (!cullWas)
                glDisable(GL_CULL_FACE);
        }
        else
        {
            drawNode();
        }

        if (inheritTextures)
            item.textureSource->unbindTextures();
        if (inheritMaterial)
            inherited->unapply();
    }

    setTransparentState(false);
    xfm->setModelMatrix(start);

    const RenderContext::Counters &after = ctx.counters();
    m_stats.drawCalls = after.drawCalls - before.drawCalls;
    m_stats.programChanges = after.programChanges - before.programChanges;
    m_stats.materialApplies = after.materialApplies - before.materialApplies;
    m_stats.uniformLookups = after.uniformLookups - before.uniformLookups;
}
