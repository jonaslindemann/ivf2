#include <ivfvis/plot_data.h>

#include <algorithm>
#include <cmath>

using namespace ivfvis;

LineSamples ivfvis::sampleLine(const UniformGrid &grid, std::string_view scalars, const glm::vec3 &a,
                               const glm::vec3 &b, int count)
{
    LineSamples samples;
    count = std::max(count, 2);
    const float length = glm::length(b - a);
    for (int i = 0; i < count; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(count - 1);
        const glm::vec3 p = glm::mix(a, b, t);
        samples.positions.push_back(p);
        samples.distance.push_back(t * length);
        samples.values.push_back(grid.sample(scalars, p));
    }
    return samples;
}

float Histogram::binWidth() const
{
    return edges.size() > 1 ? edges[1] - edges[0] : 0.0f;
}

float Histogram::peak() const
{
    return counts.empty() ? 0.0f : *std::max_element(counts.begin(), counts.end());
}

Histogram ivfvis::histogram(std::span<const float> values, int bins, ValueRange range)
{
    Histogram h;
    bins = std::max(bins, 1);
    if (!range.valid())
        for (float v : values)
            range.add(v);
    if (!range.valid())
        return h;
    if (range.max == range.min)
        range.max = range.min + 1.0f; // one value: one bin around it
    h.range = range;
    const float width = (range.max - range.min) / static_cast<float>(bins);
    for (int i = 0; i <= bins; ++i)
        h.edges.push_back(range.min + width * static_cast<float>(i));
    for (int i = 0; i < bins; ++i)
        h.centers.push_back(range.min + width * (static_cast<float>(i) + 0.5f));
    h.counts.assign(static_cast<std::size_t>(bins), 0.0f);
    for (float v : values)
    {
        if (!(v >= range.min && v <= range.max))
            continue; // NaN or outside
        const int bin = std::min(static_cast<int>((v - range.min) / width), bins - 1);
        h.counts[static_cast<std::size_t>(bin)] += 1.0f;
    }
    return h;
}
