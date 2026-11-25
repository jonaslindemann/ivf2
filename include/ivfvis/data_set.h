#pragma once

/**
 * @file data_set.h
 * @brief Base class of data sets: named scalar and vector arrays with one value per point.
 */

#include <ivfvis/value_range.h>

#include <glm/glm.hpp>

#include <cstddef>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ivfvis {

/**
 * @class DataSet
 * @brief Points with named arrays of values: scalars (one float per point) and vectors (one
 * glm::vec3 per point).
 *
 * Subclasses define where the points are (UniformGrid, PointSet). Arrays always have
 * pointCount() entries. Looking up an array that does not exist is a programming error and
 * throws std::out_of_range; use hasScalars()/hasVectors() for optional arrays.
 *
 * @code
 * auto grid = ivfvis::UniformGrid::create({64, 64, 64});
 * grid->fillScalars("density", [](const glm::vec3 &p) { return std::exp(-glm::dot(p, p)); });
 * ivfvis::ValueRange r = grid->range("density");
 * @endcode
 */
class DataSet {
public:
    virtual ~DataSet() = default;

    /** @brief Number of points (and of values in every array). */
    [[nodiscard]] virtual std::size_t pointCount() const = 0;

    /** @brief Position of point @p index. */
    [[nodiscard]] virtual glm::vec3 position(std::size_t index) const = 0;

    /**
     * @brief Add a scalar array (or reset an existing one) filled with @p value.
     * @return The new values, for filling them in.
     */
    std::span<float> addScalars(std::string_view name, float value = 0.0f);

    /** @brief Add a vector array (or reset an existing one) filled with @p value. */
    std::span<glm::vec3> addVectors(std::string_view name, const glm::vec3 &value = glm::vec3(0.0f));

    /** @brief Add (or replace) a scalar array with @p f evaluated at each point position. */
    std::span<float> fillScalars(std::string_view name, const std::function<float(const glm::vec3 &)> &f);

    /** @brief Add (or replace) a vector array with @p f evaluated at each point position. */
    std::span<glm::vec3> fillVectors(std::string_view name, const std::function<glm::vec3(const glm::vec3 &)> &f);

    /**
     * @brief Add a scalar array @p scalarName with the lengths of the vectors of @p vectorName.
     */
    std::span<float> addMagnitude(std::string_view vectorName, std::string_view scalarName);

    [[nodiscard]] bool hasScalars(std::string_view name) const;
    [[nodiscard]] bool hasVectors(std::string_view name) const;

    /** @brief Values of scalar array @p name (throws std::out_of_range if there is none). */
    [[nodiscard]] std::span<float> scalars(std::string_view name);
    [[nodiscard]] std::span<const float> scalars(std::string_view name) const;

    /** @brief Values of vector array @p name (throws std::out_of_range if there is none). */
    [[nodiscard]] std::span<glm::vec3> vectors(std::string_view name);
    [[nodiscard]] std::span<const glm::vec3> vectors(std::string_view name) const;

    /** @brief Remove the scalar or vector array @p name (nothing if there is none). */
    void removeArray(std::string_view name);

    /** @brief Names of the scalar arrays, sorted. */
    [[nodiscard]] std::vector<std::string> scalarNames() const;

    /** @brief Names of the vector arrays, sorted. */
    [[nodiscard]] std::vector<std::string> vectorNames() const;

    /**
     * @brief Smallest and largest value of scalar array @p name, or of the vector lengths of
     * vector array @p name. NaN values are ignored; invalid if there are no values.
     */
    [[nodiscard]] ValueRange range(std::string_view name) const;

protected:
    /** @brief Resize all arrays to pointCount() (new entries are 0); for subclasses whose point count changes. */
    void resizeArrays();

private:
    std::map<std::string, std::vector<float>, std::less<>> m_scalars;
    std::map<std::string, std::vector<glm::vec3>, std::less<>> m_vectors;
};

} // namespace ivfvis
