#pragma once

/**
 * @file value_range.h
 * @brief A closed interval of values (data ranges, colormap ranges).
 */

namespace ivfvis {

/** @brief Values from @c min to @c max. Invalid (min > max) for empty data. */
struct ValueRange {
    float min{1.0f};
    float max{0.0f};

    /** @brief True if the range holds at least one value. */
    [[nodiscard]] bool valid() const
    {
        return min <= max;
    }

    /** @brief max - min (0 for an invalid range). */
    [[nodiscard]] float length() const
    {
        return valid() ? max - min : 0.0f;
    }

    /** @brief True if @p value lies in the range. */
    [[nodiscard]] bool contains(float value) const
    {
        return value >= min && value <= max;
    }

    /** @brief Grow the range to include @p value (NaN is ignored). */
    void add(float value)
    {
        if (value != value)
            return;
        if (!valid())
        {
            min = max = value;
            return;
        }
        if (value < min)
            min = value;
        if (value > max)
            max = value;
    }
};

} // namespace ivfvis
