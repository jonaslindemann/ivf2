#pragma once

/**
 * @file volume.h
 * @brief Volume rendering of a scalar field: rays through the grid gather colour and opacity
 * from a transfer function.
 */

#include <ivfvis/colormap.h>
#include <ivfvis/uniform_grid.h>

#include <ivf/texture3d.h>
#include <ivf/transform_node.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ivf {
class RenderContext;
}

namespace ivfvis {

/**
 * @brief Opacity at position @p t (0..1 of a colormap's range) of a piecewise linear curve
 * through @p points (x: position, y: opacity, sorted by x); constant beyond the ends.
 */
[[nodiscard]] float opacityAt(const std::vector<glm::vec2> &points, float t);

/**
 * @brief The transfer function as @p n RGBA colours: colours from @p colormap (positions 0..1,
 * with its steps and reversal) and opacity from @p opacity.
 */
[[nodiscard]] std::vector<glm::vec4> transferTable(const Colormap &colormap, const std::vector<glm::vec2> &opacity,
                                                   int n = 256);

/**
 * @class Volume
 * @brief Scene node showing a scalar array of a UniformGrid as a semi-transparent cloud.
 *
 * Each pixel's ray through the grid's box is sampled in small steps; the values are turned
 * into colour by a colormap and into opacity by an opacity curve over the same range (the
 * transfer function), and blended front to back. Rays stop at opaque geometry inside the
 * volume, so isosurfaces, slices and other objects mix correctly with the cloud.
 *
 * @code
 * auto cmap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Inferno);
 * cmap->setRange(0.0f, 1.0f);
 * auto volume = ivfvis::Volume::create(grid, "density", cmap);
 * volume->setOpacity({{0.0f, 0.0f}, {0.3f, 0.0f}, {1.0f, 0.8f}});   // hide low values
 * scene->add(volume);
 * @endcode
 *
 * The volume is drawn in the transparent layer, after opaque objects. It casts no shadows
 * and is picked at its bounding box.
 */
class Volume : public ivf::TransformNode {
public:
    Volume(UniformGridPtr grid, std::string scalars, ColormapPtr colormap);
    ~Volume() override;

    Volume(const Volume &) = delete;
    Volume &operator=(const Volume &) = delete;

    [[nodiscard]] static std::shared_ptr<Volume> create(UniformGridPtr grid, std::string scalars,
                                                        ColormapPtr colormap);

    void setGrid(UniformGridPtr grid);
    [[nodiscard]] UniformGridPtr grid() const;

    void setScalars(const std::string &scalars);
    [[nodiscard]] const std::string &scalars() const;

    /** @brief Colours and value range of the transfer function. */
    void setColormap(ColormapPtr colormap);
    [[nodiscard]] ColormapPtr colormap() const;

    /**
     * @brief Opacity curve over the colormap's range: points (position 0..1, opacity 0..1),
     * linear between them (default: 0 at the minimum to 1 at the maximum).
     */
    void setOpacity(std::vector<glm::vec2> points);
    [[nodiscard]] const std::vector<glm::vec2> &opacity() const;

    /**
     * @brief How quickly the volume becomes opaque (default 1: material of opacity 1 hides
     * what is behind it within about a quarter of the volume's size).
     */
    void setDensity(float density);
    [[nodiscard]] float density() const;

    /** @brief Samples along a ray through the whole volume (default 256; more: finer, slower). */
    void setSamples(int samples);
    [[nodiscard]] int samples() const;

    /** @brief Light the volume using the field's gradient as normals (default false). */
    void setShading(bool shading);
    [[nodiscard]] bool shading() const;

    /** @brief Upload the grid's values again (after changing them). */
    void update();

    /** @brief The 3D texture holding the values. */
    [[nodiscard]] ivf::Texture3DPtr texture() const;

protected:
    void doDrawPass(const ivf::DrawContext &context) override;

private:
    struct Gpu;
    Gpu &gpu(ivf::RenderContext &context);
    void release(Gpu &gpu);
    bool copySceneDepth(Gpu &gpu, int width, int height);

    UniformGridPtr m_grid;
    std::string m_scalars;
    ColormapPtr m_colormap;
    std::vector<glm::vec2> m_opacity{{0.0f, 0.0f}, {1.0f, 1.0f}};
    std::uint64_t m_opacityVersion{1};
    float m_density{1.0f};
    int m_samples{256};
    bool m_shading{false};

    ivf::Texture3DPtr m_texture;
    std::vector<std::unique_ptr<Gpu>> m_gpu;
};

using VolumePtr = std::shared_ptr<Volume>;

} // namespace ivfvis
