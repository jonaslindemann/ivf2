#pragma once

/**
 * @file point_cloud.h
 * @brief Draw a PointSet as round point sprites, coloured and sized by its arrays.
 */

#include <ivfvis/colormap.h>
#include <ivfvis/point_set.h>

#include <ivf/transform_node.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ivf {
class RenderContext;
}

namespace ivfvis {

/** @brief Shape of the points of a PointCloud. */
enum class PointShape {
    Disc,   ///< Flat round discs (default).
    Sphere, ///< Discs shaded like small spheres (lit from the viewer's upper right).
    Square  ///< Squares (cheapest; no fragment discarding).
};

/** @brief Unit of PointCloud::setPointSize(). */
enum class PointSizeMode {
    World, ///< Diameter in the node's units: points shrink with distance (default).
    Pixels ///< Diameter in pixels: points keep their size on screen.
};

/**
 * @class PointCloud
 * @brief Scene node drawing the points of a PointSet, coloured by a scalar array through a
 * colormap (or in one colour), optionally sized by another array.
 *
 * Points are drawn as sprites with one draw call; a million points render in real time.
 * Colours are looked up on the GPU, so changing the colormap's range, scale, steps or colours
 * costs nothing per point. After changing the positions or arrays of the PointSet, call
 * update().
 *
 * @code
 * auto points = ivfvis::PointSet::create(positions);
 * points->addScalars("speed") ...;
 *
 * auto cmap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Inferno);
 * cmap->setRange(points->range("speed"));
 *
 * auto cloud = ivfvis::PointCloud::create(points);
 * cloud->setColorBy("speed", cmap);
 * cloud->setPointSize(0.02f);           // world units
 * scene->add(cloud);
 * @endcode
 *
 * Point clouds are picked point by point (RayHit::point), take part in hover and click
 * selection, and are culled with their bounding box. They do not cast shadows. With
 * setOpacity() below 1 they are drawn in the transparent layer, blended and without depth
 * writes.
 */
class PointCloud : public ivf::TransformNode {
public:
    explicit PointCloud(PointSetPtr points = nullptr);
    ~PointCloud() override;

    PointCloud(const PointCloud &) = delete;
    PointCloud &operator=(const PointCloud &) = delete;

    [[nodiscard]] static std::shared_ptr<PointCloud> create(PointSetPtr points = nullptr);

    /** @brief The points to draw (shared; call update() after changing them). */
    void setPoints(PointSetPtr points);
    [[nodiscard]] PointSetPtr points() const;

    /** @brief Upload changed positions and arrays of the PointSet before the next draw. */
    void update();

    /** @brief Colour the points by scalar array @p name through @p colormap. */
    void setColorBy(const std::string &name, ColormapPtr colormap);

    /** @brief Draw all points in the solid colour (setColor()). */
    void clearColorBy();

    /** @brief Name of the scalar array the points are coloured by (empty: solid colour). */
    [[nodiscard]] const std::string &colorBy() const;

    /** @brief Colormap used with setColorBy() (changes to it show on the next frame). */
    void setColormap(ColormapPtr colormap);
    [[nodiscard]] ColormapPtr colormap() const;

    /** @brief Colour of all points without setColorBy() (default light grey). */
    void setColor(const glm::vec3 &color);
    [[nodiscard]] glm::vec3 color() const;

    /**
     * @brief Scale each point by scalar array @p name: its data range maps linearly to
     * @p minScale .. @p maxScale times the point size.
     */
    void setSizeBy(const std::string &name, float minScale = 0.25f, float maxScale = 1.0f);

    /** @brief All points the same size. */
    void clearSizeBy();
    [[nodiscard]] const std::string &sizeBy() const;

    /** @brief Point diameter in world units or pixels (see setSizeMode(); default 0.05). */
    void setPointSize(float size);
    [[nodiscard]] float pointSize() const;

    void setSizeMode(PointSizeMode mode);
    [[nodiscard]] PointSizeMode sizeMode() const;

    void setShape(PointShape shape);
    [[nodiscard]] PointShape shape() const;

    /** @brief Opacity of all points (default 1); below 1 they are blended. */
    void setOpacity(float opacity);
    [[nodiscard]] float opacity() const;

    /** @brief Number of points drawn. */
    [[nodiscard]] std::size_t pointCount() const;

    [[nodiscard]] std::optional<bool> intersectRay(const ivf::Ray &ray, const glm::mat4 &world, float maxDistance,
                                                   ivf::RayHit &hit) const override;

protected:
    void doDrawPass(const ivf::DrawContext &context) override;

private:
    struct Gpu;
    Gpu *gpu(ivf::RenderContext &context);
    void upload(Gpu &gpu);
    void uploadColormap(Gpu &gpu);
    void release(Gpu &gpu);
    void rebuildSizes();

    PointSetPtr m_points;
    ColormapPtr m_colormap;
    std::string m_colorBy;
    std::string m_sizeBy;
    float m_minScale{0.25f};
    float m_maxScale{1.0f};
    glm::vec3 m_color{0.8f, 0.8f, 0.8f};
    float m_pointSize{0.05f};
    PointSizeMode m_sizeMode{PointSizeMode::World};
    PointShape m_shape{PointShape::Disc};
    float m_opacity{1.0f};

    std::uint64_t m_dataVersion{1};
    std::vector<float> m_sizes; ///< Size factor per point (empty: all 1).
    float m_maxSizeFactor{1.0f};

    // Camera of the last main-pass draw, for picking points sized in pixels
    float m_lastProjectionScale{0.0f}; ///< projection[1][1] * viewport height (window units) / 2
    bool m_lastOrthographic{false};

    std::vector<std::unique_ptr<Gpu>> m_gpu;
};

using PointCloudPtr = std::shared_ptr<PointCloud>;

} // namespace ivfvis
