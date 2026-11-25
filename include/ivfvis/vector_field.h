#pragma once

/**
 * @file vector_field.h
 * @brief A vector field that can be sampled anywhere: from grid data, a function or an
 * ivf::FlowField. Used by glyphs and streamlines.
 */

#include <ivfvis/uniform_grid.h>

#include <ivf/bounding_box.h>
#include <ivf/flow_field.h>

#include <glm/glm.hpp>

#include <functional>
#include <memory>
#include <string>

namespace ivfvis {

class VectorField;
using VectorFieldPtr = std::shared_ptr<VectorField>;

/**
 * @class VectorField
 * @brief Interface of a 3D vector field.
 *
 * @code
 * auto dipole = ivfvis::VectorField::fromFunction(
 *     [](const glm::vec3 &p) { return ...; },
 *     ivf::BoundingBox({-2, -2, -2}, {2, 2, 2}));
 * auto wind = ivfvis::VectorField::fromGrid(grid, "velocity");
 * @endcode
 */
class VectorField {
public:
    virtual ~VectorField() = default;

    /** @brief The vector at @p position. */
    [[nodiscard]] virtual glm::vec3 sample(const glm::vec3 &position) const = 0;

    /** @brief Region where the field is defined; invalid if it is unbounded (the default). */
    [[nodiscard]] virtual ivf::BoundingBox bounds() const;

    /** @brief True if @p position lies where the field is defined (inside bounds(), if valid). */
    [[nodiscard]] virtual bool contains(const glm::vec3 &position) const;

    /** @brief Vector array @p name of @p grid, interpolated trilinearly; bounded by the grid. */
    [[nodiscard]] static VectorFieldPtr fromGrid(UniformGridPtr grid, std::string name);

    /** @brief Field given by @p function, optionally limited to @p bounds. */
    [[nodiscard]] static VectorFieldPtr fromFunction(std::function<glm::vec3(const glm::vec3 &)> function,
                                                     const ivf::BoundingBox &bounds = ivf::BoundingBox());

    /** @brief Velocity of @p field (FlowField::sampleVelocity()), optionally limited to @p bounds. */
    [[nodiscard]] static VectorFieldPtr fromFlowField(ivf::FlowFieldPtr field,
                                                      const ivf::BoundingBox &bounds = ivf::BoundingBox());
};

} // namespace ivfvis
