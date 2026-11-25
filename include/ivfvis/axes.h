#pragma once

/**
 * @file axes.h
 * @brief A box of 3D axes around data: back walls with grid lines, tick marks on the outer
 * edges and labels that stay readable from any direction.
 */

#include <ivf/bounding_box.h>
#include <ivf/composite_node.h>
#include <ivf/transform_node.h>

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace ivfvis {

/**
 * @class Axes3D
 * @brief Axes around a box: grid lines on the walls facing away from the camera, ticks and
 * tick labels on one outer edge per axis, and axis titles.
 *
 * Which walls and edges are used follows the camera, so the axes never cover the data: the
 * grid is always behind it, and each axis is labelled along its lowest (x, z) or leftmost
 * (y) outline edge. Tick values are "nice numbers" (niceTicks()). Labels are drawn in screen
 * space, sharp and facing the viewer.
 *
 * @code
 * auto axes = ivfvis::Axes3D::create(grid->bounds());
 * axes->setTitles("x [m]", "height [m]", "z [m]");
 * scene->add(axes);
 *
 * auto fitted = ivfvis::Axes3D::create(surface->worldBoundingBox());
 * @endcode
 *
 * The axes are in the coordinates of their parent (world coordinates when added to the
 * scene directly). They cast no shadows and are not picked.
 */
class Axes3D : public ivf::CompositeNode {
public:
    explicit Axes3D(const ivf::BoundingBox &bounds);

    [[nodiscard]] static std::shared_ptr<Axes3D> create(const ivf::BoundingBox &bounds);

    void setBounds(const ivf::BoundingBox &bounds);
    [[nodiscard]] ivf::BoundingBox bounds() const;

    /** @brief Axis titles (default "x", "y", "z"; empty for none). */
    void setTitles(const std::string &x, const std::string &y, const std::string &z);
    [[nodiscard]] const std::string &title(int axis) const;

    /** @brief Approximate number of ticks per axis (default 5). */
    void setTickCount(int count);
    [[nodiscard]] int tickCount() const;

    /** @brief The tick values of axis @p axis (0 x, 1 y, 2 z). */
    [[nodiscard]] std::vector<double> ticks(int axis) const;

    /** @brief Grid lines on the back walls (default true; the walls' outlines stay). */
    void setGrid(bool grid);
    [[nodiscard]] bool grid() const;

    /** @brief Length of the tick marks as a fraction of the box's largest side (default 0.025). */
    void setTickLength(float fraction);

    /** @brief Colour of the edges and tick marks (default light grey). */
    void setColor(const glm::vec3 &color);

    /** @brief Colour of the grid lines (default dark grey). */
    void setGridColor(const glm::vec3 &color);

    /** @brief Colour of the labels and titles (default white). */
    void setTextColor(const glm::vec3 &color);

    /** @brief Label height in pixels (default 13; titles are 2 pixels larger). */
    void setTextSize(int pixels);

    /** @brief Size of labels in pixels. */
    [[nodiscard]] int textSize() const;
    [[nodiscard]] glm::vec3 textColor() const;
    [[nodiscard]] float tickLength() const;

    void collect(ivf::RenderQueue &queue, const ivf::CollectState &state) override;

private:
    void rebuild();

    bool m_built{false};

    ivf::BoundingBox m_bounds;
    std::array<std::string, 3> m_titles{"x", "y", "z"};
    int m_tickCount{5};
    bool m_grid{true};
    float m_tickLength{0.025f};
    glm::vec3 m_color{0.75f};
    glm::vec3 m_gridColor{0.32f};
    glm::vec3 m_textColor{1.0f};
    int m_textSize{13};
};

using Axes3DPtr = std::shared_ptr<Axes3D>;

/**
 * @brief The outline edge parallel to @p axis that carries ticks for a camera at @p eye (in
 * the box's coordinates), seen through @p viewProjection: the lowest on screen for x and z,
 * the leftmost for y. Edges are numbered by which sides of the other two axes they lie on
 * (bit 0: high side of axis + 1, bit 1: high side of axis + 2). -1 if no edge is on the
 * outline (looking along the axis, or from inside the box).
 */
[[nodiscard]] int axisEdge(const ivf::BoundingBox &box, int axis, const glm::vec3 &eye,
                           const glm::mat4 &viewProjection);

} // namespace ivfvis
