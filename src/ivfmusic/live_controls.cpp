#include <ivfmusic/live_controls.h>

#include <ivf/logger.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>

using namespace ivfmusic;
using ivf::MidiMessage;

namespace {

constexpr double kScanInterval = 2.0; // seconds between looking for devices that are not connected
constexpr size_t kRecentMessages = 32;

std::string lowercase(std::string_view text)
{
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });
    return result;
}

bool parseBinding(const nlohmann::json &json, LiveBinding &binding, std::string &error)
{
    if (!json.is_object())
    {
        error = "a binding must be an object";
        return false;
    }

    int targets = 0;
    for (const auto &[key, target] : {std::pair{"param", LiveBinding::Target::Param},
                                      std::pair{"trigger", LiveBinding::Target::Trigger},
                                      std::pair{"action", LiveBinding::Target::Action}})
    {
        if (json.contains(key))
        {
            if (!json[key].is_string())
            {
                error = std::string("\"") + key + "\" must be a name";
                return false;
            }
            binding.target = target;
            binding.name = json[key].get<std::string>();
            ++targets;
        }
    }
    if (targets != 1)
    {
        error = "a binding needs exactly one of \"param\", \"trigger\" or \"action\"";
        return false;
    }
    const std::string label = "binding \"" + binding.name + "\": ";

    const bool hasCc = json.contains("cc"), hasNote = json.contains("note");
    if (hasCc == hasNote)
    {
        error = label + "give either \"cc\" or \"note\"";
        return false;
    }
    const auto &source = hasCc ? json["cc"] : json["note"];
    if (!source.is_number_integer() || source.get<int>() < 0 || source.get<int>() > 127)
    {
        error = label + "\"cc\" and \"note\" are numbers 0-127";
        return false;
    }
    (hasCc ? binding.cc : binding.note) = source.get<int>();

    try
    {
        binding.device = json.value("device", std::string());
        if (json.contains("channel"))
        {
            const int channel = json["channel"].get<int>();
            if (channel < 1 || channel > 16)
            {
                error = label + "\"channel\" is 1-16";
                return false;
            }
            binding.channel = channel - 1;
        }
        if (json.contains("range"))
        {
            const auto &range = json["range"];
            if (!range.is_array() || range.size() != 2)
            {
                error = label + "\"range\" is [min, max]";
                return false;
            }
            binding.min = range[0].get<float>();
            binding.max = range[1].get<float>();
        }
        if (json.contains("mode"))
        {
            const std::string mode = lowercase(json["mode"].get<std::string>());
            if (mode == "override")
                binding.mode = LiveBinding::Mode::Override;
            else if (mode == "offset")
                binding.mode = LiveBinding::Mode::Offset;
            else if (mode == "scale")
                binding.mode = LiveBinding::Mode::Scale;
            else
            {
                error = label + "\"mode\" is override, offset or scale";
                return false;
            }
        }
        binding.component = json.value("component", -1);
        if (binding.component > 3)
        {
            error = label + "\"component\" is 0-3";
            return false;
        }
        binding.smoothing = std::max(json.value("smoothing", binding.smoothing), 0.0f);
        binding.pickup = json.value("pickup", false);
        binding.toggle = json.value("toggle", false);
        binding.velocity = json.value("velocity", false);
    }
    catch (const nlohmann::json::exception &e)
    {
        error = label + e.what();
        return false;
    }
    return true;
}

} // namespace

LiveControls::LiveControls() = default;
LiveControls::~LiveControls() = default;

bool LiveControls::loadFile(const std::string &path, std::string *error)
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

    std::map<std::string, std::string> devices;
    if (json.contains("devices"))
    {
        if (!json["devices"].is_object())
            return fail("\"devices\" maps an alias to part of a port name");
        for (const auto &[alias, match] : json["devices"].items())
        {
            if (!match.is_string())
                return fail("device \"" + alias + "\" must be part of a port name");
            devices[alias] = match.get<std::string>();
        }
    }

    std::vector<LiveBinding> bindings;
    if (json.contains("bindings"))
    {
        if (!json["bindings"].is_array())
            return fail("\"bindings\" must be a list");
        for (const auto &entry : json["bindings"])
        {
            LiveBinding binding;
            std::string message;
            if (!parseBinding(entry, binding, message))
                return fail(message);
            if (!binding.device.empty() && !devices.empty() && !devices.count(binding.device))
                return fail("binding \"" + binding.name + "\": unknown device \"" + binding.device + "\"");
            bindings.push_back(std::move(binding));
        }
    }

    setDevices(std::move(devices));
    setBindings(std::move(bindings));
    m_path = path;
    std::error_code ec;
    m_fileTime = std::filesystem::last_write_time(path, ec);
    return true;
}

bool LiveControls::reloadIfChanged(std::string *error)
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

void LiveControls::setBindings(std::vector<LiveBinding> bindings)
{
    m_bindings = std::move(bindings);
    m_states.assign(m_bindings.size(), State{});
    for (size_t i = 0; i < m_bindings.size(); ++i)
        m_states[i].target = m_states[i].value = m_bindings[i].min;
}

void LiveControls::setDevices(std::map<std::string, std::string> devices)
{
    if (devices == m_deviceMatches && m_scanned)
        return;
    closeDevices();
    m_deviceMatches = std::move(devices);
}

void LiveControls::closeDevices()
{
    m_devices.clear(); // MidiController closes its port when destroyed
    m_scanned = false;
}

void LiveControls::openDevices()
{
    if (!m_scanner)
        m_scanner = std::make_unique<ivf::MidiController>();
    const std::vector<std::string> ports = m_scanner->listPorts();

    // Forget devices that were unplugged, so they are opened again when they come back
    for (auto it = m_devices.begin(); it != m_devices.end();)
    {
        if (std::find(ports.begin(), ports.end(), it->second.portName) == ports.end())
        {
            ivf::logInfo("MIDI device \"" + it->first + "\" disconnected", "LiveControls");
            it = m_devices.erase(it);
        }
        else
        {
            ++it;
        }
    }

    auto inUse = [&](const std::string &port) {
        return std::any_of(m_devices.begin(), m_devices.end(),
                           [&](const auto &device) { return device.second.portName == port; });
    };

    auto open = [&](const std::string &alias, unsigned int index) {
        auto controller = ivf::MidiController::create();
        if (!controller->openPort(index))
        {
            ivf::logWarning("Cannot open MIDI port " + ports[index] + ": " + controller->lastError(),
                            "LiveControls");
            return;
        }
        ivf::logInfo("MIDI device \"" + alias + "\" on " + ports[index], "LiveControls");
        m_devices[alias] = Device{ports[index], controller};
    };

    if (m_deviceMatches.empty())
    {
        for (unsigned int i = 0; i < ports.size(); ++i)
            if (!m_devices.count(ports[i]) && !inUse(ports[i]))
                open(ports[i], i);
        return;
    }

    for (const auto &[alias, match] : m_deviceMatches)
    {
        if (m_devices.count(alias))
            continue;
        const std::string wanted = lowercase(match);
        for (unsigned int i = 0; i < ports.size(); ++i)
        {
            if (lowercase(ports[i]).find(wanted) != std::string::npos && !inUse(ports[i]))
            {
                open(alias, i);
                break;
            }
        }
    }
}

std::vector<LiveEvent> LiveControls::update(double dt)
{
    m_scanTimer += dt;
    if (!m_scanned || m_scanTimer >= kScanInterval)
    {
        m_scanned = true;
        m_scanTimer = 0.0;
        openDevices();
    }

    for (auto &[alias, device] : m_devices)
        for (const auto &message : device.controller->poll())
            handleMessage(alias, message);

    for (size_t i = 0; i < m_bindings.size(); ++i)
    {
        State &state = m_states[i];
        const float smoothing = m_bindings[i].smoothing;
        if (smoothing <= 0.0f)
            state.value = state.target;
        else
            state.value += (state.target - state.value) * float(1.0 - std::exp(-dt / smoothing));
    }

    std::vector<LiveEvent> events;
    events.swap(m_events);
    return events;
}

bool LiveControls::matches(const LiveBinding &binding, const std::string &device, const MidiMessage &message) const
{
    if (!binding.device.empty() && binding.device != device)
        return false;
    if (binding.channel >= 0 && binding.channel != message.channel)
        return false;
    if (binding.cc >= 0)
        return message.type == MidiMessage::ControlChange && message.data1 == binding.cc;
    return (message.type == MidiMessage::NoteOn || message.type == MidiMessage::NoteOff) &&
           message.data1 == binding.note;
}

void LiveControls::handleMessage(const std::string &device, const MidiMessage &message)
{
    m_recent.push_back(LiveMessage{device, message});
    while (m_recent.size() > kRecentMessages)
        m_recent.pop_front();

    for (size_t i = 0; i < m_bindings.size(); ++i)
    {
        const LiveBinding &binding = m_bindings[i];
        if (!matches(binding, device, message))
            continue;

        const float amount = float(message.data2) / 127.0f;
        if (binding.note >= 0)
        {
            press(i, amount, message.type == MidiMessage::NoteOn);
        }
        else if (binding.target == LiveBinding::Target::Param && !binding.toggle)
        {
            // A fader or knob
            setTarget(i, binding.min + (binding.max - binding.min) * amount);
        }
        else
        {
            // A button sending 127 when pressed and 0 when released
            State &state = m_states[i];
            const bool down = message.data2 >= 64;
            if (down != state.buttonDown)
            {
                state.buttonDown = down;
                press(i, 1.0f, down);
            }
        }
    }
}

void LiveControls::setTarget(size_t index, float value)
{
    const LiveBinding &binding = m_bindings[index];
    State &state = m_states[index];
    state.target = value;
    if (state.active)
        return;

    if (binding.mode == LiveBinding::Mode::Override && binding.pickup && state.hasBase)
    {
        // Take over only once the control reaches the value in effect, so nothing jumps
        const float tolerance = 0.02f * std::abs(binding.max - binding.min);
        if (!state.armed)
        {
            state.armed = true;
            state.armedFrom = value;
        }
        const bool crossed = (state.armedFrom - state.base) * (value - state.base) <= 0.0f;
        if (!crossed && std::abs(value - state.base) > tolerance)
            return;
    }

    state.active = true;
    state.armed = false;
    state.value = value;
}

void LiveControls::press(size_t index, float velocity, bool down)
{
    const LiveBinding &binding = m_bindings[index];
    State &state = m_states[index];

    switch (binding.target)
    {
    case LiveBinding::Target::Param: {
        if (binding.toggle)
        {
            if (!down)
                return;
            state.on = !state.on;
        }
        else
        {
            state.on = down;
        }
        const float pressed = binding.velocity ? binding.min + (binding.max - binding.min) * velocity : binding.max;
        if (!state.active)
        {
            // Smooth in from the released value
            state.active = true;
            state.value = binding.min;
        }
        state.target = state.on ? pressed : binding.min;
        break;
    }
    case LiveBinding::Target::Trigger:
        if (down)
            m_events.push_back(LiveEvent{binding.target, binding.name, velocity, true, false});
        break;
    case LiveBinding::Target::Action:
        if (binding.toggle)
        {
            if (!down)
                return;
            state.on = !state.on;
            m_events.push_back(LiveEvent{binding.target, binding.name, 1.0f, state.on, true});
        }
        else
        {
            m_events.push_back(LiveEvent{binding.target, binding.name, velocity, down, false});
        }
        break;
    }
}

glm::vec4 LiveControls::apply(std::string_view name, glm::vec4 base, int components) const
{
    components = std::clamp(components, 1, 4);
    for (size_t i = 0; i < m_bindings.size(); ++i)
    {
        const LiveBinding &binding = m_bindings[i];
        if (binding.target != LiveBinding::Target::Param || binding.name != name)
            continue;

        const State &state = m_states[i];
        state.base = base[binding.component >= 0 ? binding.component : 0];
        state.hasBase = true;
        if (!state.active)
            continue;

        const int first = binding.component >= 0 ? binding.component : 0;
        const int last = binding.component >= 0 ? binding.component : components - 1;
        for (int c = first; c <= last; ++c)
        {
            switch (binding.mode)
            {
            case LiveBinding::Mode::Override:
                base[c] = state.value;
                break;
            case LiveBinding::Mode::Offset:
                base[c] += state.value;
                break;
            case LiveBinding::Mode::Scale:
                base[c] *= state.value;
                break;
            }
        }
    }
    return base;
}

bool LiveControls::active(std::string_view name) const
{
    for (size_t i = 0; i < m_bindings.size(); ++i)
        if (m_bindings[i].target == LiveBinding::Target::Param && m_bindings[i].name == name && m_states[i].active)
            return true;
    return false;
}

std::vector<std::string> LiveControls::activeParams() const
{
    std::vector<std::string> names;
    for (size_t i = 0; i < m_bindings.size(); ++i)
        if (m_bindings[i].target == LiveBinding::Target::Param && m_states[i].active &&
            std::find(names.begin(), names.end(), m_bindings[i].name) == names.end())
            names.push_back(m_bindings[i].name);
    return names;
}

void LiveControls::release()
{
    for (size_t i = 0; i < m_bindings.size(); ++i)
        if (m_bindings[i].target == LiveBinding::Target::Param)
            release(m_bindings[i].name);
}

void LiveControls::release(std::string_view name)
{
    for (size_t i = 0; i < m_bindings.size(); ++i)
    {
        if (m_bindings[i].target != LiveBinding::Target::Param || m_bindings[i].name != name)
            continue;
        State &state = m_states[i];
        state.active = false;
        state.armed = false;
        state.on = false;
        state.target = state.value = m_bindings[i].min;
    }
}

std::vector<LiveControls::DeviceStatus> LiveControls::devices() const
{
    std::vector<DeviceStatus> result;
    if (m_deviceMatches.empty())
    {
        for (const auto &[alias, device] : m_devices)
            result.push_back(DeviceStatus{alias, {}, device.portName});
        return result;
    }
    for (const auto &[alias, match] : m_deviceMatches)
    {
        auto found = m_devices.find(alias);
        result.push_back(DeviceStatus{alias, match, found != m_devices.end() ? found->second.portName : ""});
    }
    return result;
}

std::vector<std::string> LiveControls::ports() const
{
    return m_scanner ? m_scanner->listPorts() : std::vector<std::string>{};
}

float LiveControls::bindingValue(size_t index) const
{
    return index < m_states.size() ? m_states[index].value : 0.0f;
}

bool LiveControls::bindingActive(size_t index) const
{
    return index < m_states.size() && m_states[index].active;
}
