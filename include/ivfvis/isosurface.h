#pragma once

/**
 * @file isosurface.h
 * @brief A surface of constant value through a scalar field (marching cubes).
 */

#include <ivfvis/colormap.h>
#include <ivfvis/surface_data.h>
#include <ivfvis/uniform_grid.h>

#include <ivf/mesh_node.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ivfvis {

/**
 * @class Isosurface
 * @brief Scene node showing where a scalar array of a UniformGrid equals a value.
 *
 * The surface is rebuilt when the grid, array or iso value change (setIsoValue() is fast
 * enough for sliders on grids up to about 128^3). It is a regular ivf::MeshNode: give it a
 * material (alpha below 1 for see-through shells), it casts shadows and is picked by its
 * triangles. With setColorBy() the surface is coloured by another array through a colormap,
 * following changes to the colormap.
 *
 * @code
 * auto shell = ivfvis::Isosurface::create(grid, "density", 0.3f);
 * shell->setMaterial(material);
 * scene->add(shell);
 *
 * auto core = ivfvis::Isosurface::create(grid, "density", 0.8f);
 * core->setColorBy("temperature", cmap);
 * scene->add(core);
 * @endcode
 */
class Isosurface : public ivf::MeshNode {
public:
    Isosurface(UniformGridPtr grid, std::string scalars, float isoValue);

    [[nodiscard]] static std::shared_ptr<Isosurface> create(UniformGridPtr grid, std::string scalars,
                                                            float isoValue);

    void setGrid(UniformGridPtr grid);
    [[nodiscard]] UniformGridPtr grid() const;

    /** @brief The scalar array whose isosurface is shown. */
    void setScalars(const std::string &scalars);
    [[nodiscard]] const std::string &scalars() const;

    void setIsoValue(float isoValue);
    [[nodiscard]] float isoValue() const;

    /** @brief Colour the surface by scalar array @p name of the grid through @p colormap. */
    void setColorBy(const std::string &name, ColormapPtr colormap);

    /** @brief Use the material's colour again. */
    void clearColorBy();
    [[nodiscard]] const std::string &colorBy() const;
    [[nodiscard]] ColormapPtr colormap() const;

    /** @brief Rebuild the surface (after changing the grid's values). */
    void update();

    /** @brief Number of triangles of the surface. */
    [[nodiscard]] std::size_t triangleCount() const;

    void collect(ivf::RenderQueue &queue, const ivf::CollectState &state) override;

private:
    void recolor();

    UniformGridPtr m_grid;
    std::string m_scalars;
    float m_isoValue;
    std::string m_colorBy;
    ColormapPtr m_colormap;
    std::vector<float> m_values;
    std::size_t m_triangles{0};
    const Colormap *m_coloredWith{nullptr};
    std::uint64_t m_coloredVersion{0};
};

using IsosurfacePtr = std::shared_ptr<Isosurface>;

} // namespace ivfvis
