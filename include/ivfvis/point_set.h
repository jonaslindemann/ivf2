#pragma once

/**
 * @file point_set.h
 * @brief Scattered points with named scalar and vector arrays (point clouds, particles,
 * measurement locations).
 */

#include <ivfvis/data_set.h>

#include <ivf/bounding_box.h>

#include <memory>
#include <vector>

namespace ivfvis {

/**
 * @class PointSet
 * @brief Points at arbitrary positions with named arrays (see DataSet).
 *
 * @code
 * auto points = ivfvis::PointSet::create(positions);
 * auto speed = points->addScalars("speed");
 * for (std::size_t i = 0; i < speed.size(); ++i)
 *     speed[i] = ...;
 * @endcode
 */
class PointSet : public DataSet {
public:
    PointSet() = default;
    explicit PointSet(std::vector<glm::vec3> positions);

    [[nodiscard]] static std::shared_ptr<PointSet> create();
    [[nodiscard]] static std::shared_ptr<PointSet> create(std::vector<glm::vec3> positions);

    [[nodiscard]] std::size_t pointCount() const override;
    [[nodiscard]] glm::vec3 position(std::size_t index) const override;

    /** @brief The positions (for changing them in place). */
    [[nodiscard]] std::span<glm::vec3> positions();
    [[nodiscard]] std::span<const glm::vec3> positions() const;

    /** @brief Replace the points; arrays are resized (new values are 0). */
    void setPositions(std::vector<glm::vec3> positions);

    /** @brief Append a point (its values in all arrays are 0); returns its index. */
    std::size_t addPoint(const glm::vec3 &position);

    /** @brief Box around the points (invalid if there are none). */
    [[nodiscard]] ivf::BoundingBox bounds() const;

private:
    std::vector<glm::vec3> m_positions;
};

using PointSetPtr = std::shared_ptr<PointSet>;

} // namespace ivfvis
