#include <ivfmusic/automation.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

using namespace ivfmusic;
using ivf::Easing;

namespace {

bool parseValue(const nlohmann::json &json, glm::vec4 &value, int &components)
{
    if (json.is_number())
    {
        value = glm::vec4(json.get<float>(), 0.0f, 0.0f, 0.0f);
        components = 1;
        return true;
    }
    if (json.is_array() && json.size() >= 1 && json.size() <= 4)
    {
        value = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        for (size_t i = 0; i < json.size(); ++i)
        {
            if (!json[i].is_number())
                return false;
            value[int(i)] = json[i].get<float>();
        }
        components = int(json.size());
        return true;
    }
    return false;
}

std::string lowercase(std::string_view text)
{
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });
    return result;
}

glm::vec4 evaluate(const std::vector<AutomationKey> &keys, double bar)
{
    auto next = std::upper_bound(keys.begin(), keys.end(), bar,
                                 [](double b, const AutomationKey &k) { return b < k.bar; });
    if (next == keys.begin())
        return keys.front().value;
    if (next == keys.end())
        return keys.back().value;

    const AutomationKey &a = *(next - 1);
    const AutomationKey &b = *next;
    const float t = float((bar - a.bar) / (b.bar - a.bar));
    if (!b.spline)
        return glm::mix(a.value, b.value, ivf::detail::applyEasing(b.easing, std::clamp(t, 0.0f, 1.0f)));

    // Cubic Hermite segment. The tangent at a key between two spline segments is
    // the Catmull-Rom slope (per bar) through its neighbours; anywhere else it is zero.
    auto tangent = [&](size_t k) {
        if (k == 0 || k + 1 >= keys.size() || !keys[k].spline || !keys[k + 1].spline)
            return glm::vec4(0.0f);
        return (keys[k + 1].value - keys[k - 1].value) / float(keys[k + 1].bar - keys[k - 1].bar);
    };
    const size_t ia = size_t(next - keys.begin()) - 1;
    const float h = float(b.bar - a.bar);
    const float t2 = t * t, t3 = t2 * t;
    return (2.0f * t3 - 3.0f * t2 + 1.0f) * a.value + (t3 - 2.0f * t2 + t) * h * tangent(ia) +
           (-2.0f * t3 + 3.0f * t2) * b.value + (t3 - t2) * h * tangent(ia + 1);
}

bool inside(const std::vector<std::pair<double, double>> &regions, double bar)
{
    return std::any_of(regions.begin(), regions.end(),
                       [&](const auto &region) { return bar >= region.first && bar <= region.second; });
}

} // namespace

bool Automation::parseEasing(std::string_view name, ivf::Easing &easing, bool &spline)
{
    static const std::map<std::string, Easing, std::less<>> names = [] {
        // Short names of automation files
        std::map<std::string, Easing, std::less<>> result = {
            {"linear", Easing::Linear}, {"step", Easing::Step}, {"smooth", Easing::SmoothStep},
            {"sine", Easing::SineInOut}, {"in", Easing::QuadIn}, {"out", Easing::QuadOut},
            {"cubic", Easing::CubicInOut}};
        // Every ivf::Easing name, lowercase
        for (int i = 0; i < ivf::EasingCount; ++i)
            result.emplace(lowercase(ivf::easingName(Easing(i))), Easing(i));
        return result;
    }();

    const std::string key = lowercase(name);
    if (key == "spline")
    {
        easing = Easing::Linear;
        spline = true;
        return true;
    }
    auto it = names.find(key);
    if (it == names.end())
        return false;
    easing = it->second;
    spline = false;
    return true;
}

bool Automation::loadFile(const std::string &path, std::string *error)
{
    auto fail = [&](const std::string &message) {
        if (error)
            *error = path + ": " + message;
        return false;
    };

    std::ifstream file(path);
    if (!file)
        return fail("cannot open");

    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(file, nullptr, true, true); // allow comments
    }
    catch (const nlohmann::json::parse_error &e)
    {
        return fail(e.what());
    }

    // Parse into a new automation so an error keeps the current contents
    Automation loaded;

    if (json.contains("markers"))
    {
        for (const auto &marker : json["markers"])
        {
            if (!marker.contains("bar") || !marker["bar"].is_number() || !marker.contains("name"))
                return fail("markers need a numeric \"bar\" and a \"name\"");
            loaded.addMarker(marker["bar"].get<double>(), marker["name"].get<std::string>());
        }
    }

    if (json.contains("params"))
    {
        for (const auto &[name, keys] : json["params"].items())
        {
            if (!keys.is_array())
                return fail("parameter \"" + name + "\" must be a list of keys");
            for (const auto &key : keys)
            {
                if (!key.is_array() || key.size() < 2 || key.size() > 3 || !key[0].is_number())
                    return fail("parameter \"" + name + "\": keys are [bar, value] or [bar, value, \"easing\"]");

                AutomationKey parsed;
                parsed.bar = key[0].get<double>();
                if (!parseValue(key[1], parsed.value, parsed.components))
                    return fail("parameter \"" + name + "\": a value is a number or a list of 1-4 numbers");

                if (key.size() == 3 &&
                    (!key[2].is_string() || !parseEasing(key[2].get<std::string>(), parsed.easing, parsed.spline)))
                    return fail("parameter \"" + name +
                                "\": unknown easing (use linear, step, smooth, sine, in, out, cubic, spline "
                                "or an ivf::Easing name)");

                loaded.setKey(name, parsed);
            }
        }
    }

    if (json.contains("regions"))
    {
        for (const auto &[name, regions] : json["regions"].items())
        {
            if (!regions.is_array())
                return fail("regions of \"" + name + "\" must be a list of [from, to]");
            for (const auto &region : regions)
            {
                if (!region.is_array() || region.size() != 2 || !region[0].is_number() || !region[1].is_number())
                    return fail("regions of \"" + name + "\" must be a list of [from, to]");
                loaded.addRegion(name, region[0].get<double>(), region[1].get<double>());
            }
        }
    }

    loaded.m_path = path;
    std::error_code ec;
    loaded.m_fileTime = std::filesystem::last_write_time(path, ec);
    *this = std::move(loaded);
    return true;
}

bool Automation::reloadIfChanged(std::string *error)
{
    if (m_path.empty())
        return false;

    std::error_code ec;
    const auto time = std::filesystem::last_write_time(m_path, ec);
    if (ec || time == m_fileTime)
        return false;

    // Remember the time even on failure, so a broken file is reported once per save
    m_fileTime = time;
    const std::string path = m_path;
    return loadFile(path, error);
}

bool Automation::saveFile(const std::string &path, std::string *error)
{
    // Values to float precision; positions with more digits, so keys just before a bar stay distinct
    auto number = [](double value, bool position = false) {
        char text[32];
        std::snprintf(text, sizeof(text), position ? "%.10g" : "%.6g", value);
        return std::string(text);
    };

    // One key per line keeps the file readable and diffable
    std::ostringstream out;
    out << "{\n  \"markers\": [";
    for (size_t i = 0; i < m_markers.size(); ++i)
        out << (i ? ",\n" : "\n") << "    { \"bar\": " << number(m_markers[i].bar, true)
            << ", \"name\": " << nlohmann::json(m_markers[i].name).dump() << " }";
    out << (m_markers.empty() ? "],\n" : "\n  ],\n");

    out << "  \"params\": {";
    bool firstLane = true;
    for (const auto &[name, keys] : m_lanes)
    {
        out << (firstLane ? "\n" : ",\n") << "    " << nlohmann::json(name).dump() << ": [";
        firstLane = false;
        const int count = components(name);
        for (size_t i = 0; i < keys.size(); ++i)
        {
            const AutomationKey &key = keys[i];
            out << (i ? ",\n      " : "\n      ") << "[" << number(key.bar, true) << ", ";
            if (count == 1)
            {
                out << number(key.value.x);
            }
            else
            {
                out << "[";
                for (int c = 0; c < count; ++c)
                    out << (c ? ", " : "") << number(key.value[c]);
                out << "]";
            }
            if (key.spline)
                out << ", \"spline\"";
            else if (key.easing != Easing::Linear)
                out << ", \"" << ivf::easingName(key.easing) << "\"";
            out << "]";
        }
        out << (keys.empty() ? "]" : "\n    ]");
    }
    out << (m_lanes.empty() ? "}" : "\n  }");

    if (!m_regions.empty())
    {
        out << ",\n  \"regions\": {";
        bool firstRegion = true;
        for (const auto &[name, regions] : m_regions)
        {
            out << (firstRegion ? "\n" : ",\n") << "    " << nlohmann::json(name).dump() << ": [";
            firstRegion = false;
            for (size_t i = 0; i < regions.size(); ++i)
                out << (i ? ", " : "") << "[" << number(regions[i].first, true) << ", "
                    << number(regions[i].second, true) << "]";
            out << "]";
        }
        out << "\n  }";
    }
    out << "\n}\n";

    const std::string temporary = path + ".tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file || !(file << out.str()) || !file.flush())
        {
            if (error)
                *error = path + ": cannot write";
            return false;
        }
    }
    std::error_code ec;
    std::filesystem::rename(temporary, path, ec);
    if (ec)
    {
        // rename() does not replace an existing file on every platform
        std::filesystem::remove(path, ec);
        std::filesystem::rename(temporary, path, ec);
    }
    if (ec)
    {
        if (error)
            *error = path + ": " + ec.message();
        return false;
    }

    m_path = path;
    m_fileTime = std::filesystem::last_write_time(path, ec);
    return true;
}

void Automation::clear()
{
    m_lanes.clear();
    m_markers.clear();
    m_regions.clear();
    m_overlays.clear();
}

void Automation::merge(const Automation &other)
{
    for (const auto &[name, keys] : other.m_lanes)
    {
        auto regions = other.m_regions.find(name);
        if (regions == other.m_regions.end() || regions->second.empty())
        {
            m_lanes[name] = keys;
            m_overlays.erase(name);
        }
        else
        {
            m_overlays[name] = Overlay{regions->second, keys};
        }
    }
}

void Automation::addRegion(const std::string &name, double fromBar, double toBar)
{
    if (toBar < fromBar)
        std::swap(fromBar, toBar);
    Regions &regions = m_regions[name];
    Regions merged;
    for (const auto &region : regions)
    {
        if (region.second < fromBar || region.first > toBar)
        {
            merged.push_back(region);
        }
        else
        {
            fromBar = std::min(fromBar, region.first);
            toBar = std::max(toBar, region.second);
        }
    }
    merged.emplace_back(fromBar, toBar);
    std::sort(merged.begin(), merged.end());
    regions = std::move(merged);
}

std::vector<std::pair<double, double>> Automation::regions(std::string_view name) const
{
    auto found = m_regions.find(name);
    return found != m_regions.end() ? found->second : Regions{};
}

std::vector<std::string> Automation::names() const
{
    std::vector<std::string> result;
    for (const auto &[name, keys] : m_lanes)
        result.push_back(name);
    for (const auto &[name, overlay] : m_overlays)
        if (!m_lanes.count(name))
            result.push_back(name);
    std::sort(result.begin(), result.end());
    return result;
}

const Automation::Overlay *Automation::overlayAt(std::string_view name, double bar) const
{
    auto found = m_overlays.find(name);
    if (found == m_overlays.end() || found->second.keys.empty() || !inside(found->second.regions, bar))
        return nullptr;
    return &found->second;
}

void Automation::eraseKeys(const std::string &name, double fromBar, double toBar)
{
    auto found = m_lanes.find(name);
    if (found == m_lanes.end())
        return;
    auto &keys = found->second;
    keys.erase(std::remove_if(keys.begin(), keys.end(),
                              [&](const AutomationKey &key) { return key.bar >= fromBar && key.bar <= toBar; }),
               keys.end());
}

void Automation::removeLane(const std::string &name)
{
    m_lanes.erase(name);
    m_regions.erase(name);
    m_overlays.erase(name);
}

const AutomationKey *Automation::keyAtOrBefore(std::string_view name, double bar) const
{
    auto lastAtOrBefore = [&](const std::vector<AutomationKey> &keys, auto &&accept) -> const AutomationKey * {
        auto next = std::upper_bound(keys.begin(), keys.end(), bar,
                                     [](double b, const AutomationKey &k) { return b < k.bar; });
        while (next != keys.begin())
        {
            --next;
            if (accept(*next))
                return &*next;
        }
        return nullptr;
    };

    const AutomationKey *result = nullptr;
    auto overlay = m_overlays.find(name);
    const Regions *regions = overlay != m_overlays.end() ? &overlay->second.regions : nullptr;

    // Keys of the lane count only outside the regions of a merged take
    if (auto found = m_lanes.find(name); found != m_lanes.end())
        result = lastAtOrBefore(found->second,
                                [&](const AutomationKey &key) { return !regions || !inside(*regions, key.bar); });

    if (overlay != m_overlays.end())
    {
        const AutomationKey *taken = lastAtOrBefore(
            overlay->second.keys, [&](const AutomationKey &key) { return inside(overlay->second.regions, key.bar); });
        if (taken && (!result || taken->bar >= result->bar))
            result = taken;
    }
    return result;
}

void Automation::setKey(const std::string &name, const AutomationKey &key)
{
    AutomationKey stored = key;
    stored.components = std::clamp(stored.components, 1, 4);
    auto &keys = m_lanes[name];
    auto it = std::lower_bound(keys.begin(), keys.end(), stored.bar,
                               [](const AutomationKey &k, double b) { return k.bar < b; });
    if (it != keys.end() && it->bar == stored.bar)
        *it = stored;
    else
        keys.insert(it, stored);
}

void Automation::setKey(const std::string &name, double bar, glm::vec4 value, ivf::Easing easing, int components)
{
    setKey(name, AutomationKey{bar, value, easing, false, components});
}

void Automation::setKey(const std::string &name, double bar, float value, ivf::Easing easing)
{
    setKey(name, AutomationKey{bar, glm::vec4(value, 0.0f, 0.0f, 0.0f), easing, false, 1});
}

void Automation::setSplineKey(const std::string &name, double bar, glm::vec4 value, int components)
{
    setKey(name, AutomationKey{bar, value, Easing::Linear, true, components});
}

int Automation::components(std::string_view name) const
{
    int result = 0;
    if (auto found = m_lanes.find(name); found != m_lanes.end())
        for (const auto &key : found->second)
            result = std::max(result, key.components);
    if (auto found = m_overlays.find(name); found != m_overlays.end())
        for (const auto &key : found->second.keys)
            result = std::max(result, key.components);
    return result;
}

void Automation::addMarker(double bar, std::string name)
{
    auto it = std::upper_bound(m_markers.begin(), m_markers.end(), bar,
                               [](double b, const SongMarker &m) { return b < m.bar; });
    m_markers.insert(it, SongMarker{bar, std::move(name)});
}

bool Automation::has(std::string_view name) const
{
    return m_lanes.find(name) != m_lanes.end() || m_overlays.find(name) != m_overlays.end();
}

glm::vec4 Automation::vec4(std::string_view name, double bar, glm::vec4 fallback) const
{
    if (const Overlay *overlay = overlayAt(name, bar))
        return evaluate(overlay->keys, bar);

    auto found = m_lanes.find(name);
    if (found == m_lanes.end() || found->second.empty())
        return fallback;
    return evaluate(found->second, bar);
}

glm::vec3 Automation::vec3(std::string_view name, double bar, glm::vec3 fallback) const
{
    return glm::vec3(vec4(name, bar, glm::vec4(fallback, 0.0f)));
}

float Automation::value(std::string_view name, double bar, float fallback) const
{
    return vec4(name, bar, glm::vec4(fallback, 0.0f, 0.0f, 0.0f)).x;
}

double Automation::integral(std::string_view name, double fromBar, double toBar, float fallback, double step) const
{
    if (toBar <= fromBar)
        return 0.0;
    if (!has(name))
        return double(fallback) * (toBar - fromBar);
    const int steps = std::max(1, int(std::ceil((toBar - fromBar) / step)));
    const double h = (toBar - fromBar) / steps;
    double sum = 0.5 * (value(name, fromBar, fallback) + value(name, toBar, fallback));
    for (int i = 1; i < steps; ++i)
        sum += value(name, fromBar + i * h, fallback);
    return sum * h;
}

int Automation::markerIndexAt(double bar) const
{
    auto it = std::upper_bound(m_markers.begin(), m_markers.end(), bar,
                               [](double b, const SongMarker &m) { return b < m.bar; });
    return int(it - m_markers.begin()) - 1;
}
