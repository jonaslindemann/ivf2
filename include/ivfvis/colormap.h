#pragma once

/**
 * @file colormap.h
 * @brief Colormaps: mapping values to colours, with the perceptually uniform maps of
 * matplotlib built in.
 */

#include <ivfvis/value_range.h>

#include <ivf/result.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ivfvis {

/** @brief The built-in colormaps. */
enum class BuiltinColormap {
    Viridis,  ///< Perceptually uniform, dark blue to yellow (default).
    Magma,    ///< Perceptually uniform, black to light yellow through purple.
    Inferno,  ///< Perceptually uniform, black to yellow through red.
    Plasma,   ///< Perceptually uniform, blue to yellow through magenta.
    Cividis,  ///< Perceptually uniform and readable with colour vision deficiencies.
    Turbo,    ///< Rainbow-like with smooth lightness (a better "jet").
    Coolwarm, ///< Diverging blue - white - red (for values around a centre).
    Gray      ///< Black to white.
};

/** @brief Number of built-in colormaps. */
inline constexpr int BuiltinColormapCount = 8;

/** @brief Lower-case name of a built-in colormap ("viridis", "coolwarm", ...). */
[[nodiscard]] const char *builtinColormapName(BuiltinColormap map);

/** @brief How values are spread over the colormap. */
enum class ColormapScale {
    Linear, ///< Evenly between the range limits.
    Log     ///< Evenly in log10 (the range must be positive; values <= 0 are below the range).
};

/** @brief A colour at a position (0 to 1) of a colormap. */
struct ColorStop {
    float position{0.0f};
    glm::vec4 color{0.0f, 0.0f, 0.0f, 1.0f};
};

class Colormap;
using ColormapPtr = std::shared_ptr<Colormap>;

/**
 * @class Colormap
 * @brief Maps values to colours: a value range, a scale and a list of colour stops.
 *
 * color() normalises a value to 0..1 with the range and scale, then reads the stops
 * (linearly interpolated). Values outside the range get the end colours, or the colours of
 * setOutOfRangeColors(); NaN gets nanColor(). setSteps() turns the map into bands and
 * setReversed() flips it.
 *
 * @code
 * auto cmap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Viridis);
 * cmap->setRange(0.0f, 100.0f);
 * glm::vec4 c = cmap->color(42.0f);
 *
 * auto custom = ivfvis::Colormap::create("ice", {{0.0f, {0, 0, 0.2f, 1}}, {1.0f, {0.8f, 1, 1, 1}}});
 * @endcode
 *
 * Colormaps are shared between the objects that use them (ColorBar, colorVertices());
 * version() changes with every setting so users can tell when to update.
 */
class Colormap {
public:
    Colormap(std::string name, std::vector<ColorStop> stops);

    /** @brief A built-in colormap (range 0 to 1). */
    [[nodiscard]] static ColormapPtr create(BuiltinColormap map = BuiltinColormap::Viridis);

    /** @brief A colormap from colour stops (sorted by position; positions are scaled to 0..1). */
    [[nodiscard]] static ColormapPtr create(std::string name, std::vector<ColorStop> stops);

    /** @brief A colormap from evenly spaced colours. */
    [[nodiscard]] static ColormapPtr create(std::string name, const std::vector<glm::vec3> &colors);

    /** @brief The built-in colormap called @p name (case-insensitive); an error for unknown names. */
    [[nodiscard]] static ivf::Result<ColormapPtr> named(std::string_view name);

    /** @brief Names of the built-in colormaps, in BuiltinColormap order. */
    [[nodiscard]] static std::vector<std::string> builtinNames();

    /** @brief A copy with the same stops and settings (version() starts over). */
    [[nodiscard]] ColormapPtr copy() const;

    [[nodiscard]] const std::string &name() const;
    [[nodiscard]] const std::vector<ColorStop> &stops() const;

    /** @brief Values mapped to the first and last colour. */
    void setRange(float min, float max);
    void setRange(const ValueRange &range);
    [[nodiscard]] ValueRange range() const;

    void setScale(ColormapScale scale);
    [[nodiscard]] ColormapScale scale() const;

    /** @brief Flip the colours (low values get the last colour). */
    void setReversed(bool reversed);
    [[nodiscard]] bool reversed() const;

    /** @brief Number of discrete colour bands; 0 (default) for a continuous map. */
    void setSteps(int steps);
    [[nodiscard]] int steps() const;

    /**
     * @brief Colours for values below and above the range; std::nullopt (default) uses the
     * colour at the nearest end.
     */
    void setOutOfRangeColors(std::optional<glm::vec4> below, std::optional<glm::vec4> above);
    [[nodiscard]] std::optional<glm::vec4> belowRangeColor() const;
    [[nodiscard]] std::optional<glm::vec4> aboveRangeColor() const;

    /** @brief Colour of NaN values (default mid grey). */
    void setNanColor(const glm::vec4 &color);
    [[nodiscard]] glm::vec4 nanColor() const;

    /**
     * @brief Position of @p value in the range: 0 at the minimum, 1 at the maximum (with the
     * scale applied); below 0 or above 1 outside the range. NaN for NaN.
     */
    [[nodiscard]] float normalize(float value) const;

    /** @brief Inverse of normalize(): the value at position @p t of the range. */
    [[nodiscard]] float value(float t) const;

    /** @brief The colour of @p value. */
    [[nodiscard]] glm::vec4 color(float value) const;

    /** @brief The colour at position @p t (0 to 1, clamped) with steps and reversal applied. */
    [[nodiscard]] glm::vec4 colorAt(float t) const;

    /** @brief @p n colours evenly from position 0 to 1. */
    [[nodiscard]] std::vector<glm::vec4> table(int n = 256) const;

    /** @brief table() as RGBA bytes (4 * n), e.g. for a texture. */
    [[nodiscard]] std::vector<std::uint8_t> rgba8(int n = 256) const;

    /** @brief Changes with every setting. */
    [[nodiscard]] std::uint64_t version() const;

private:
    [[nodiscard]] glm::vec4 stopColor(float t) const;

    std::string m_name;
    std::vector<ColorStop> m_stops;
    ValueRange m_range{0.0f, 1.0f};
    ColormapScale m_scale{ColormapScale::Linear};
    bool m_reversed{false};
    int m_steps{0};
    std::optional<glm::vec4> m_below;
    std::optional<glm::vec4> m_above;
    glm::vec4 m_nanColor{0.5f, 0.5f, 0.5f, 1.0f};
    std::uint64_t m_version{1};
};

} // namespace ivfvis
