#pragma once

/**
 * @file overlay_node.h
 * @brief A group of nodes drawn in window pixel coordinates on top of the scene (HUDs,
 * color bars, legends).
 */

#include <ivf/composite_node.h>

namespace ivf {

/**
 * @class OverlayNode
 * @brief Group whose children are drawn in pixel coordinates over the 3D scene.
 *
 * The children use an orthographic camera with the origin at the bottom left of the
 * viewport, x to the right and y up, one unit per pixel. They are drawn in the order they
 * were added, without depth testing and with alpha blending, after everything else in the
 * main pass (layer RenderLayer::Overlay). Overlays are not drawn into shadow maps,
 * reflections or the selection buffer, and are not picked.
 *
 * Children should use unlit materials (Material::setUseLighting(false)); text nodes look
 * sharpest with TextNode::setResolution() equal to their size and whole-pixel positions.
 *
 * @code
 * auto hud = ivf::OverlayNode::create();
 * auto label = ivf::TextNode::create();
 * label->setText("Frame 1");
 * label->setSize(16.0f);
 * label->setResolution(16);
 * label->setColor({1.0f, 1.0f, 1.0f});
 * label->setPos({20.0f, 20.0f, 0.0f});
 * hud->add(label);
 * scene->add(hud);
 * @endcode
 *
 * Subclasses lay out their children in onLayout(), which receives the viewport size
 * before each draw. Overlays are drawn into the scene image, so post-processing effects
 * and the window's tone mapping apply to them as well.
 */
class OverlayNode : public CompositeNode {
public:
    OverlayNode();

    [[nodiscard]] static std::shared_ptr<OverlayNode> create();

    /** @brief Adds one draw item for the whole overlay; the children are drawn by it. */
    void collect(RenderQueue &queue, const CollectState &state) override;

    /** @brief Width of the viewport at the last draw (0 before the first). */
    [[nodiscard]] int viewportWidth() const;

    /** @brief Height of the viewport at the last draw (0 before the first). */
    [[nodiscard]] int viewportHeight() const;

protected:
    void doDrawPass(const DrawContext &context) override;

    /**
     * @brief Called before the children are drawn, with the viewport size in pixels.
     *
     * Override to place children relative to the window edges.
     */
    virtual void onLayout(int width, int height);

private:
    int m_width{0};
    int m_height{0};
};

using OverlayNodePtr = std::shared_ptr<OverlayNode>;

} // namespace ivf
