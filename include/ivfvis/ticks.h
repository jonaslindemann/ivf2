#pragma once

/**
 * @file ticks.h
 * @brief Tick positions and labels for axes and color bars ("nice numbers").
 */

#include <string>
#include <vector>

namespace ivfvis {

/**
 * @brief A "nice" number close to @p x: 1, 2, 5 or 10 times a power of ten.
 * @param round Round to the nearest nice number (true) or take the next larger one (false).
 */
[[nodiscard]] double niceNumber(double x, bool round);

/** @brief Distance between about @p count ticks over [min, max], a nice number. */
[[nodiscard]] double niceTickStep(double min, double max, int count = 5);

/**
 * @brief Tick values inside [min, max]: multiples of niceTickStep(), about @p count of
 * them. Empty if min > max; one tick if min == max.
 */
[[nodiscard]] std::vector<double> niceTicks(double min, double max, int count = 5);

/**
 * @brief Ticks for a logarithmic scale: the powers of ten inside [min, max] (min > 0); if
 * there are fewer than two, also 2x and 5x each power, then niceTicks().
 */
[[nodiscard]] std::vector<double> logTicks(double min, double max);

/**
 * @brief Label of tick @p value for ticks @p step apart: as many decimals as the step needs,
 * scientific notation for very large or small values, "0" for values rounding to zero.
 */
[[nodiscard]] std::string formatTick(double value, double step);

} // namespace ivfvis
