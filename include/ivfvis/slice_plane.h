#pragma once

/**
 * @file slice_plane.h
 * @brief A plane cut through a scalar field, coloured by its values, with contour lines.
 */

#include <ivfvis/colormap.h>
#include <ivfvis/surface_data.h>
#include <ivfvis/uniform_grid.h>

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ivfvis {

/** @brief Grid axis for SlicePlane::setAxis(). */
enum class SliceAxis {
    X, ///< Plane x = position.
    Y, ///< Plane y = position.
    Z  ///< Plane z = position.
};

/**
 * @class SlicePlane
 * @brief Scene node showing the values of a scalar array of a UniformGrid on a plane.
 *
 * The plane is any plane through the grid (setPlane()) or one perpendicular to an axis
 * (setAxis()); the part inside the grid's box is coloured by the values through a colormap,
 * unlit, so the colours read the same as on the colour bar. Optional contour lines mark
 * chosen values.
 *
 * @code
 * auto slice = ivfvis::SlicePlane::create(grid, "pressure", cmap);
 * slice->setAxis(ivfvis::SliceAxis::Z, 0.0f);
 * slice->setContours(8);
 * scene->add(slice);
 * @endcode
 */
class SlicePlane : public ivf::MeshNode {
public:
    SlicePlane(UniformGridPtr grid, std::string scalars, ColormapPtr colormap);

    [[nodiscard]] static std::shared_ptr<SlicePlane> create(UniformGridPtr grid, std::string scalars,
                                                            ColormapPtr colormap);

    void setGrid(UniformGridPtr grid);
    [[nodiscard]] UniformGridPtr grid() const;

    void setScalars(const std::string &scalars);
    [[nodiscard]] const std::string &scalars() const;

    /** @brief Colormap of the values (changes to it show on the next frame). */
    void setColormap(ColormapPtr colormap);
    [[nodiscard]] ColormapPtr colormap() const;

    /** @brief The plane through @p origin with normal @p normal (default: z through the grid's centre). */
    void setPlane(const glm::vec3 &origin, const glm::vec3 &normal);

    /** @brief The plane perpendicular to @p axis at @p position along it. */
    void setAxis(SliceAxis axis, float position);

    [[nodiscard]] glm::vec3 planeOrigin() const;
    [[nodiscard]] glm::vec3 planeNormal() const;

    /** @brief Squares along the slice's longer side (default 128). */
    void setResolution(int resolution);
    [[nodiscard]] int resolution() const;

    /** @brief @p count contour lines evenly spaced inside the colormap's range (0: none). */
    void setContours(int count);

    /** @brief Contour lines at these values. */
    void setContourValues(std::vector<float> values);

    /** @brief The contour values in use. */
    [[nodiscard]] std::vector<float> contourValues() const;

    /** @brief Colour of the contour lines (default black). */
    void setContourColor(const glm::vec3 &color);

    /** @brief Rebuild the slice (after changing the grid's values). */
    void update();

    void collect(ivf::RenderQueue &queue, const ivf::CollectState &state) override;

private:
    void rebuild();
    void recolor();

    UniformGridPtr m_grid;
    std::string m_scalars;
    ColormapPtr m_colormap;
    glm::vec3 m_origin{0.0f};
    glm::vec3 m_normal{0.0f, 0.0f, 1.0f};
    int m_resolution{128};
    int m_contourCount{0};
    std::vector<float> m_contourValues;
    glm::vec3 m_contourColor{0.0f};

    SurfaceData m_surface;
    std::shared_ptr<ivf::Material> m_lineMaterial;
    const Colormap *m_builtWith{nullptr};
    std::uint64_t m_builtVersion{0};
};

using SlicePlanePtr = std::shared_ptr<SlicePlane>;

} // namespace ivfvis
