#pragma once

/**
 * @file uniform_grid.h
 * @brief Regular 3D grid of points with named scalar and vector arrays (image data).
 */

#include <ivfvis/data_set.h>

#include <ivf/bounding_box.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivfvis {

/**
 * @class UniformGrid
 * @brief Points on a regular grid: dimensions (points per axis), origin and spacing.
 *
 * Point (i, j, k) is at origin + (i, j, k) * spacing and has index i + nx * (j + ny * k),
 * so x varies fastest (the layout of VTK image data and of C-ordered NumPy arrays indexed
 * [k][j][i]). A dimension of 1 makes a 2D (or 1D) grid.
 *
 * sample() and gradient() interpolate the arrays trilinearly between the points and clamp
 * positions outside the grid to its boundary.
 *
 * @code
 * // 64^3 points covering [-1, 1]^3
 * auto grid = ivfvis::UniformGrid::create({64, 64, 64}, ivf::BoundingBox({-1, -1, -1}, {1, 1, 1}));
 * grid->fillScalars("r", [](const glm::vec3 &p) { return glm::length(p); });
 * float v = grid->sample("r", {0.5f, 0.0f, 0.0f});
 * @endcode
 */
class UniformGrid : public DataSet {
public:
    /** @param dimensions Points per axis (each at least 1). */
    UniformGrid(const glm::ivec3 &dimensions, const glm::vec3 &origin, const glm::vec3 &spacing);

    [[nodiscard]] static std::shared_ptr<UniformGrid> create(const glm::ivec3 &dimensions,
                                                             const glm::vec3 &origin = glm::vec3(0.0f),
                                                             const glm::vec3 &spacing = glm::vec3(1.0f));

    /** @brief A grid whose points span @p bounds (corner to corner). */
    [[nodiscard]] static std::shared_ptr<UniformGrid> create(const glm::ivec3 &dimensions,
                                                             const ivf::BoundingBox &bounds);

    [[nodiscard]] glm::ivec3 dimensions() const;
    [[nodiscard]] glm::vec3 origin() const;
    [[nodiscard]] glm::vec3 spacing() const;
    void setOrigin(const glm::vec3 &origin);
    void setSpacing(const glm::vec3 &spacing);

    [[nodiscard]] std::size_t pointCount() const override;
    [[nodiscard]] glm::vec3 position(std::size_t index) const override;

    /** @brief Position of point (i, j, k). */
    [[nodiscard]] glm::vec3 position(int i, int j, int k) const;

    /** @brief Index of point (i, j, k) in the arrays. */
    [[nodiscard]] std::size_t index(int i, int j, int k) const;

    /** @brief Box spanned by the points. */
    [[nodiscard]] ivf::BoundingBox bounds() const;

    /** @brief True if @p position lies inside bounds() (boundary included). */
    [[nodiscard]] bool contains(const glm::vec3 &position) const;

    /** @brief Scalar array @p name interpolated at @p position (clamped to the grid). */
    [[nodiscard]] float sample(std::string_view name, const glm::vec3 &position) const;

    /** @brief Vector array @p name interpolated at @p position (clamped to the grid). */
    [[nodiscard]] glm::vec3 sampleVector(std::string_view name, const glm::vec3 &position) const;

    /**
     * @brief Gradient of scalar array @p name at @p position, by central differences of
     * sample() one grid spacing apart (zero along axes with one point).
     */
    [[nodiscard]] glm::vec3 gradient(std::string_view name, const glm::vec3 &position) const;

private:
    struct Cell {
        std::size_t i0, j0, k0, i1, j1, k1;
        glm::vec3 t;
    };
    [[nodiscard]] Cell cell(const glm::vec3 &position) const;

    glm::ivec3 m_dimensions;
    glm::vec3 m_origin;
    glm::vec3 m_spacing;
};

using UniformGridPtr = std::shared_ptr<UniformGrid>;

} // namespace ivfvis
