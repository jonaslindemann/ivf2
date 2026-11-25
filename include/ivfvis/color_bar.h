#pragma once

/**
 * @file color_bar.h
 * @brief A colour bar (legend of a colormap) drawn over the scene.
 */

#include <ivfvis/colormap.h>

#include <ivf/mesh_node.h>
#include <ivf/overlay_node.h>
#include <ivf/text_node.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ivfvis {

/** @brief Window edge a ColorBar is placed along. */
enum class ColorBarPlacement {
    Right,  ///< Vertical, along the right edge (default); labels to the left of the bar.
    Left,   ///< Vertical, along the left edge; labels to the right.
    Bottom, ///< Horizontal, along the bottom edge; labels below the bar.
    Top     ///< Horizontal, along the top edge; labels below the bar.
};

/**
 * @class ColorBar
 * @brief Shows a colormap with tick labels and a title at an edge of the window.
 *
 * The bar follows its colormap: changing the range, scale, steps or colours of the
 * colormap updates it on the next frame. Ticks are "nice numbers" (niceTicks(), logTicks()
 * for a log scale). Sizes are in pixels.
 *
 * @code
 * auto cmap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Viridis);
 * cmap->setRange(0.0f, 250.0f);
 * auto bar = ivfvis::ColorBar::create(cmap);
 * bar->setTitle("Temperature [K]");
 * scene->add(bar);   // drawn over the scene (an ivf::OverlayNode)
 * @endcode
 *
 * Needs a loaded font for the labels (GLFWSceneWindow loads one by default).
 */
class ColorBar : public ivf::OverlayNode {
public:
    explicit ColorBar(ColormapPtr colormap);

    [[nodiscard]] static std::shared_ptr<ColorBar> create(ColormapPtr colormap);

    void setColormap(ColormapPtr colormap);
    [[nodiscard]] ColormapPtr colormap() const;

    /** @brief Text above (vertical bars) or beside (horizontal bars) the bar; empty for none. */
    void setTitle(const std::string &title);
    [[nodiscard]] const std::string &title() const;

    void setPlacement(ColorBarPlacement placement);
    [[nodiscard]] ColorBarPlacement placement() const;

    /** @brief Length of the bar as a fraction of the window height (vertical) or width (default 0.5). */
    void setLength(float fraction);
    [[nodiscard]] float length() const;

    /** @brief Width of the bar across its length in pixels (default 20). */
    void setThickness(int pixels);
    [[nodiscard]] int thickness() const;

    /** @brief Distance of the bar and its labels from the window edge in pixels (default 24). */
    void setMargin(int pixels);
    [[nodiscard]] int margin() const;

    /** @brief Approximate number of ticks (default 5). */
    void setTickCount(int count);
    [[nodiscard]] int tickCount() const;

    /** @brief Use these tick values instead of automatic ones; empty to go back to automatic. */
    void setTicks(std::vector<double> ticks);

    /** @brief Height of the label text in pixels (default 14; the title is 2 pixels larger). */
    void setTextSize(int pixels);
    [[nodiscard]] int textSize() const;

    /** @brief Colour of text, ticks and frame (default white). */
    void setTextColor(const glm::vec3 &color);
    [[nodiscard]] glm::vec3 textColor() const;

    /** @brief Draw a frame around the bar (default true). */
    void setFrame(bool frame);
    [[nodiscard]] bool frame() const;

protected:
    void onLayout(int width, int height) override;

private:
    void rebuild(int width, int height);
    [[nodiscard]] ivf::TextNodePtr label(std::size_t index);

    ColormapPtr m_colormap;
    std::string m_title;
    ColorBarPlacement m_placement{ColorBarPlacement::Right};
    float m_length{0.5f};
    int m_thickness{20};
    int m_margin{24};
    int m_tickCount{5};
    std::vector<double> m_ticks;
    int m_textSize{14};
    glm::vec3 m_textColor{1.0f};
    bool m_frame{true};

    // Layout of the last rebuild
    int m_builtWidth{0};
    int m_builtHeight{0};
    std::uint64_t m_builtColormap{0};
    std::uint64_t m_settings{1};
    std::uint64_t m_builtSettings{0};
    const Colormap *m_builtFor{nullptr};

    ivf::MeshNodePtr m_bar;
    ivf::MeshNodePtr m_lines;
    std::vector<ivf::TextNodePtr> m_labels;
    ivf::TextNodePtr m_titleNode;
};

using ColorBarPtr = std::shared_ptr<ColorBar>;

} // namespace ivfvis
