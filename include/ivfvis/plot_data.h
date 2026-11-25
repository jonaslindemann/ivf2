#pragma once

/**
 * @file plot_data.h
 * @brief Data for 2D plots: values along a line through a grid, histograms.
 *
 * GL- and UI-free; draw the results with ImPlot in an ImGui window (IVF2_WITH_IMPLOT):
 *
 * @code
 * auto profile = ivfvis::sampleLine(*grid, "temperature", a, b, 200);
 * if (ImPlot::BeginPlot("Along the probe"))
 * {
 *     ImPlot::PlotLine("temperature", profile.distance.data(), profile.values.data(),
 *                      static_cast<int>(profile.values.size()));
 *     ImPlot::EndPlot();
 * }
 * @endcode
 */

#include <ivfvis/uniform_grid.h>
#include <ivfvis/value_range.h>

#include <glm/glm.hpp>

#include <span>
#include <string_view>
#include <vector>

namespace ivfvis {

/** @brief Values sampled along a line. */
struct LineSamples {
    std::vector<float> distance;      ///< Distance of each sample from the start.
    std::vector<float> values;        ///< The values.
    std::vector<glm::vec3> positions; ///< Where the samples are.
};

/**
 * @brief Scalar array @p scalars of @p grid sampled at @p count evenly spaced points from
 * @p a to @p b (trilinear interpolation, clamped to the grid).
 */
[[nodiscard]] LineSamples sampleLine(const UniformGrid &grid, std::string_view scalars, const glm::vec3 &a,
                                     const glm::vec3 &b, int count);

/** @brief Counts of values in equal bins. */
struct Histogram {
    std::vector<float> edges;   ///< bins + 1 bin edges, from range.min to range.max.
    std::vector<float> centers; ///< Bin centres (for bar plots).
    std::vector<float> counts;  ///< Values per bin.
    ValueRange range;           ///< Range covered by the bins.

    /** @brief Width of a bin. */
    [[nodiscard]] float binWidth() const;

    /** @brief Largest count. */
    [[nodiscard]] float peak() const;
};

/**
 * @brief Histogram of @p values in @p bins equal bins over @p range (an invalid range, the
 * default: the values' own range). NaN and values outside the range are not counted.
 */
[[nodiscard]] Histogram histogram(std::span<const float> values, int bins, ValueRange range = {});

} // namespace ivfvis
