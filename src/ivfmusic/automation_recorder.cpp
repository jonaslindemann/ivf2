#include <ivfmusic/automation_recorder.h>

#include <algorithm>
#include <cmath>

using namespace ivfmusic;

namespace {

constexpr double kGap = 1.0 / 16.0;    // a longer pause between samples of a lane starts a new stretch
constexpr double kJump = 0.5;          // a forward jump of the position larger than this is a seek

struct Point {
    double bar;
    glm::vec4 value;
};

// Ramer-Douglas-Peucker: keep the points needed to stay within tolerance of the original curve
void simplify(const std::vector<Point> &points, size_t first, size_t last, int components, float tolerance,
              std::vector<bool> &keep)
{
    if (last <= first + 1)
        return;
    const Point &a = points[first];
    const Point &b = points[last];
    float worst = -1.0f;
    size_t worstIndex = first;
    for (size_t i = first + 1; i < last; ++i)
    {
        const float t = float((points[i].bar - a.bar) / std::max(b.bar - a.bar, 1e-9));
        const glm::vec4 expected = glm::mix(a.value, b.value, t);
        float error = 0.0f;
        for (int c = 0; c < components; ++c)
            error = std::max(error, std::abs(points[i].value[c] - expected[c]));
        if (error > worst)
        {
            worst = error;
            worstIndex = i;
        }
    }
    if (worst <= tolerance)
        return;
    keep[worstIndex] = true;
    simplify(points, first, worstIndex, components, tolerance, keep);
    simplify(points, worstIndex, last, components, tolerance, keep);
}

std::vector<Point> simplified(const std::vector<Point> &points, int components)
{
    if (points.size() <= 2)
        return points;

    glm::vec4 low = points.front().value, high = low;
    for (const Point &p : points)
    {
        low = glm::min(low, p.value);
        high = glm::max(high, p.value);
    }
    float span = 0.0f;
    for (int c = 0; c < components; ++c)
        span = std::max(span, high[c] - low[c]);
    const float tolerance = std::max(1e-4f, 0.002f * span);

    std::vector<bool> keep(points.size(), false);
    keep.front() = keep.back() = true;
    simplify(points, 0, points.size() - 1, components, tolerance, keep);

    std::vector<Point> result;
    for (size_t i = 0; i < points.size(); ++i)
        if (keep[i])
            result.push_back(points[i]);
    return result;
}

} // namespace

void AutomationRecorder::start()
{
    clear();
    m_recording = true;
}

void AutomationRecorder::stop()
{
    m_recording = false;
}

void AutomationRecorder::clear()
{
    m_samples.clear();
    m_triggers.clear();
    m_stretches.clear();
    m_stretch = 0;
    m_hasBar = false;
}

void AutomationRecorder::frame(double bar, bool playing)
{
    m_playing = playing;
    if (!m_recording || !playing)
        return;

    // A seek (backwards, or a jump forwards) starts a new stretch
    if (!m_hasBar || bar < m_bar || bar > m_bar + kJump)
    {
        m_stretches.emplace_back(bar, bar);
        m_stretch = int(m_stretches.size()) - 1;
    }
    m_stretches.back().second = bar;
    m_bar = bar;
    m_hasBar = true;
}

void AutomationRecorder::sample(const std::string &name, glm::vec4 value, int components)
{
    if (!m_recording || !m_playing || !m_hasBar)
        return;
    Lane &lane = m_samples[name];
    lane.components = std::max(lane.components, std::clamp(components, 1, 4));
    // Several reads in one frame give one sample
    if (!lane.samples.empty() && lane.samples.back().bar == m_bar && lane.samples.back().stretch == m_stretch)
        lane.samples.back().value = value;
    else
        lane.samples.push_back(Sample{m_stretch, m_bar, value});
}

void AutomationRecorder::trigger(const std::string &name, float velocity)
{
    if (!m_recording || !m_playing || !m_hasBar)
        return;
    m_triggers[name].push_back(TriggerHit{m_stretch, m_bar, velocity});
}

size_t AutomationRecorder::sampleCount() const
{
    size_t count = 0;
    for (const auto &[name, lane] : m_samples)
        count += lane.samples.size();
    for (const auto &[name, hits] : m_triggers)
        count += hits.size();
    return count;
}

std::vector<std::string> AutomationRecorder::commit(Automation &take)
{
    std::vector<std::string> changed;

    for (const auto &[name, lane] : m_samples)
    {
        if (lane.samples.empty())
            continue;

        // Split into stretches: a seek or a pause in live control ends one
        std::vector<std::vector<Point>> stretches;
        for (size_t i = 0; i < lane.samples.size(); ++i)
        {
            const Sample &s = lane.samples[i];
            if (i == 0 || s.stretch != lane.samples[i - 1].stretch || s.bar - lane.samples[i - 1].bar > kGap)
                stretches.emplace_back();
            stretches.back().push_back(Point{s.bar, s.value});
        }

        for (const auto &points : stretches)
        {
            const double from = points.front().bar, to = points.back().bar;
            take.eraseKeys(name, from, to);
            for (const Point &p : simplified(points, lane.components))
                take.setKey(name, p.bar, p.value, ivf::Easing::Linear, lane.components);
            take.addRegion(name, from, to);
        }
        changed.push_back(name);
    }

    for (const auto &[name, hits] : m_triggers)
    {
        // A new take of a trigger replaces its earlier hits in the stretches recorded
        for (const auto &[from, to] : m_stretches)
        {
            take.eraseKeys(name, from, to);
            take.addRegion(name, from, to);
        }
        for (const TriggerHit &hit : hits)
            take.setKey(name, hit.bar, glm::vec4(hit.velocity, 0.0f, 0.0f, 0.0f), ivf::Easing::Step, 1);
        changed.push_back(name);
    }

    clear();
    return changed;
}
