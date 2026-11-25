#include <ivfvis/colormap.h>

#include "colormap_data.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

using namespace ivfvis;

namespace {

const char *const g_names[BuiltinColormapCount] = {"viridis", "magma",   "inferno",  "plasma",
                                                   "cividis", "turbo",   "coolwarm", "gray"};

std::vector<ColorStop> tableStops(const std::uint8_t (&data)[256][3])
{
    std::vector<ColorStop> stops(256);
    for (int i = 0; i < 256; ++i)
    {
        stops[static_cast<std::size_t>(i)].position = static_cast<float>(i) / 255.0f;
        stops[static_cast<std::size_t>(i)].color =
            glm::vec4(data[i][0] / 255.0f, data[i][1] / 255.0f, data[i][2] / 255.0f, 1.0f);
    }
    return stops;
}

std::vector<ColorStop> builtinStops(BuiltinColormap map)
{
    using namespace ivfvis::detail;
    switch (map)
    {
    case BuiltinColormap::Viridis:
        return tableStops(viridis_data);
    case BuiltinColormap::Magma:
        return tableStops(magma_data);
    case BuiltinColormap::Inferno:
        return tableStops(inferno_data);
    case BuiltinColormap::Plasma:
        return tableStops(plasma_data);
    case BuiltinColormap::Cividis:
        return tableStops(cividis_data);
    case BuiltinColormap::Turbo:
        return tableStops(turbo_data);
    case BuiltinColormap::Coolwarm:
        return tableStops(coolwarm_data);
    case BuiltinColormap::Gray:
        break;
    }
    return {{0.0f, {0.0f, 0.0f, 0.0f, 1.0f}}, {1.0f, {1.0f, 1.0f, 1.0f, 1.0f}}};
}

} // namespace

const char *ivfvis::builtinColormapName(BuiltinColormap map)
{
    const auto i = static_cast<int>(map);
    return i >= 0 && i < BuiltinColormapCount ? g_names[i] : "unknown";
}

Colormap::Colormap(std::string name, std::vector<ColorStop> stops) : m_name(std::move(name)), m_stops(std::move(stops))
{
    if (m_stops.empty())
        m_stops = builtinStops(BuiltinColormap::Gray);
    std::stable_sort(m_stops.begin(), m_stops.end(),
                     [](const ColorStop &a, const ColorStop &b) { return a.position < b.position; });
    // Positions from 0 to 1
    const float lo = m_stops.front().position, hi = m_stops.back().position;
    for (auto &stop : m_stops)
        stop.position = hi > lo ? (stop.position - lo) / (hi - lo) : 0.0f;
}

ColormapPtr Colormap::create(BuiltinColormap map)
{
    return std::make_shared<Colormap>(builtinColormapName(map), builtinStops(map));
}

ColormapPtr Colormap::create(std::string name, std::vector<ColorStop> stops)
{
    return std::make_shared<Colormap>(std::move(name), std::move(stops));
}

ColormapPtr Colormap::create(std::string name, const std::vector<glm::vec3> &colors)
{
    std::vector<ColorStop> stops;
    for (std::size_t i = 0; i < colors.size(); ++i)
    {
        const float t = colors.size() > 1 ? static_cast<float>(i) / static_cast<float>(colors.size() - 1) : 0.0f;
        stops.push_back({t, glm::vec4(colors[i], 1.0f)});
    }
    return std::make_shared<Colormap>(std::move(name), std::move(stops));
}

ivf::Result<ColormapPtr> Colormap::named(std::string_view name)
{
    std::string lower(name);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower == "grey")
        lower = "gray";
    for (int i = 0; i < BuiltinColormapCount; ++i)
        if (lower == g_names[i])
            return create(static_cast<BuiltinColormap>(i));
    return ivf::makeError(ivf::ErrorCode::InvalidArgument, "Unknown colormap '" + std::string(name) + "'", "Colormap");
}

std::vector<std::string> Colormap::builtinNames()
{
    return std::vector<std::string>(std::begin(g_names), std::end(g_names));
}

ColormapPtr Colormap::copy() const
{
    auto c = std::make_shared<Colormap>(*this);
    c->m_version = 1;
    return c;
}

const std::string &Colormap::name() const
{
    return m_name;
}

const std::vector<ColorStop> &Colormap::stops() const
{
    return m_stops;
}

void Colormap::setRange(float min, float max)
{
    m_range = {min, max};
    ++m_version;
}

void Colormap::setRange(const ValueRange &range)
{
    setRange(range.min, range.max);
}

ValueRange Colormap::range() const
{
    return m_range;
}

void Colormap::setScale(ColormapScale scale)
{
    m_scale = scale;
    ++m_version;
}

ColormapScale Colormap::scale() const
{
    return m_scale;
}

void Colormap::setReversed(bool reversed)
{
    m_reversed = reversed;
    ++m_version;
}

bool Colormap::reversed() const
{
    return m_reversed;
}

void Colormap::setSteps(int steps)
{
    m_steps = std::max(steps, 0);
    ++m_version;
}

int Colormap::steps() const
{
    return m_steps;
}

void Colormap::setOutOfRangeColors(std::optional<glm::vec4> below, std::optional<glm::vec4> above)
{
    m_below = below;
    m_above = above;
    ++m_version;
}

std::optional<glm::vec4> Colormap::belowRangeColor() const
{
    return m_below;
}

std::optional<glm::vec4> Colormap::aboveRangeColor() const
{
    return m_above;
}

void Colormap::setNanColor(const glm::vec4 &color)
{
    m_nanColor = color;
    ++m_version;
}

glm::vec4 Colormap::nanColor() const
{
    return m_nanColor;
}

float Colormap::normalize(float value) const
{
    if (value != value)
        return value;
    if (m_scale == ColormapScale::Log && m_range.min > 0.0f && m_range.max > m_range.min)
    {
        if (value <= 0.0f)
            return -std::numeric_limits<float>::infinity();
        const float lo = std::log10(m_range.min), hi = std::log10(m_range.max);
        return (std::log10(value) - lo) / (hi - lo);
    }
    const float length = m_range.max - m_range.min;
    if (length == 0.0f)
        return value < m_range.min ? -1.0f : (value > m_range.max ? 2.0f : 0.5f);
    return (value - m_range.min) / length;
}

float Colormap::value(float t) const
{
    if (m_scale == ColormapScale::Log && m_range.min > 0.0f && m_range.max > m_range.min)
    {
        const float lo = std::log10(m_range.min), hi = std::log10(m_range.max);
        return std::pow(10.0f, lo + t * (hi - lo));
    }
    return m_range.min + t * (m_range.max - m_range.min);
}

glm::vec4 Colormap::color(float value) const
{
    const float t = normalize(value);
    if (t != t)
        return m_nanColor;
    if (t < 0.0f && m_below)
        return *m_below;
    if (t > 1.0f && m_above)
        return *m_above;
    return colorAt(t);
}

glm::vec4 Colormap::colorAt(float t) const
{
    if (t != t)
        return m_nanColor;
    t = std::clamp(t, 0.0f, 1.0f);
    if (m_steps > 0)
    {
        // Band i of n takes the colour at i / (n - 1), so the first and last bands get the end colours
        const int band = std::min(static_cast<int>(t * static_cast<float>(m_steps)), m_steps - 1);
        t = m_steps > 1 ? static_cast<float>(band) / static_cast<float>(m_steps - 1) : 0.5f;
    }
    if (m_reversed)
        t = 1.0f - t;
    return stopColor(t);
}

glm::vec4 Colormap::stopColor(float t) const
{
    if (m_stops.size() == 1 || t <= m_stops.front().position)
        return m_stops.front().color;
    if (t >= m_stops.back().position)
        return m_stops.back().color;
    auto upper = std::upper_bound(m_stops.begin(), m_stops.end(), t,
                                  [](float v, const ColorStop &stop) { return v < stop.position; });
    const ColorStop &b = *upper;
    const ColorStop &a = *(upper - 1);
    const float span = b.position - a.position;
    const float f = span > 0.0f ? (t - a.position) / span : 0.0f;
    return glm::mix(a.color, b.color, f);
}

std::vector<glm::vec4> Colormap::table(int n) const
{
    std::vector<glm::vec4> colors;
    if (n <= 0)
        return colors;
    colors.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
        colors.push_back(colorAt(n > 1 ? static_cast<float>(i) / static_cast<float>(n - 1) : 0.5f));
    return colors;
}

std::vector<std::uint8_t> Colormap::rgba8(int n) const
{
    std::vector<std::uint8_t> bytes;
    for (const glm::vec4 &c : table(n))
        for (int k = 0; k < 4; ++k)
            bytes.push_back(static_cast<std::uint8_t>(std::lround(std::clamp(c[k], 0.0f, 1.0f) * 255.0f)));
    return bytes;
}

std::uint64_t Colormap::version() const
{
    return m_version;
}
