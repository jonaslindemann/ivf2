#include <ivf/renderer.h>

#include <ivf/material.h>
#include <ivf/node.h>
#include <ivf/transform_manager.h>

using namespace ivf;

void Renderer::execute(const RenderQueue &queue, const SkipFn &skip)
{
    execute(queue, Frustum(), skip);
}

void Renderer::execute(const RenderQueue &queue, const Frustum &frustum, const SkipFn &skip)
{
    auto *xfm = TransformManager::instance();
    const glm::mat4 start = xfm->modelMatrix();

    m_stats = Stats{};
    m_stats.items = queue.size();

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

        // Material and textures of the nearest ancestor, unless the node has its own
        const bool inheritMaterial = item.material && !(node->m_material && node->m_useMaterial);
        const bool inheritTextures = item.textureSource != nullptr && item.textureSource != node &&
                                     !node->m_useTexture;
        if (inheritMaterial)
            item.material->apply();
        if (inheritTextures)
            item.textureSource->bindTextures();

        node->doPreDraw();
        if (node->m_visible)
            node->doDraw();
        node->doPostDraw();

        if (inheritTextures)
            item.textureSource->unbindTextures();
        if (inheritMaterial)
            item.material->unapply();
    }

    xfm->setModelMatrix(start);
}
