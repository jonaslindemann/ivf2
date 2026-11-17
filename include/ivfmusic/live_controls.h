#pragma once

/**
 * @file live_controls.h
 * @brief MIDI controllers (faders, knobs, pads) mapped to scene parameters for live performance.
 */

#include <ivf/midi_controller.h>

#include <glm/glm.hpp>

#include <deque>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ivfmusic {

/**
 * @struct LiveBinding
 * @brief Maps one MIDI control to a parameter, a trigger or an action.
 */
struct LiveBinding {
    /** @brief What the control drives. */
    enum class Target {
        Param,   ///< A continuous parameter, combined with the automation value.
        Trigger, ///< A one-shot event of the scene, e.g. a shockwave.
        Action   ///< A built-in command of the window, e.g. "record" or "freeze".
    };

    /** @brief How a parameter control combines with the automation value. */
    enum class Mode {
        Override, ///< Replaces the automation value once the control has been moved.
        Offset,   ///< Added to the automation value.
        Scale     ///< Multiplies the automation value.
    };

    std::string device; ///< Device alias from the "devices" section; empty matches any device.
    int channel{-1};    ///< MIDI channel 0-15; -1 matches any channel.
    int cc{-1};         ///< Controller number, or -1.
    int note{-1};       ///< Note number (pads, keys), or -1.

    Target target{Target::Param};
    std::string name; ///< Parameter, trigger or action name.

    Mode mode{Mode::Override};
    float min{0.0f};        ///< Value at the bottom of the control (or when a pad is released).
    float max{1.0f};        ///< Value at the top of the control (or when a pad is pressed).
    int component{-1};      ///< Component of a vector parameter (0-3); -1 applies to all.
    float smoothing{0.08f}; ///< Time constant of the smoothing in seconds; 0 follows the control at once.
    bool pickup{false};     ///< Override only: take over when the control passes the current value.
    bool toggle{false};     ///< Pads and buttons: switch on and off with each press instead of while held.
    bool velocity{false};   ///< Pads: scale the pressed value by the velocity.
};

/**
 * @struct LiveEvent
 * @brief A trigger or action fired by a control in this frame.
 */
struct LiveEvent {
    LiveBinding::Target target{LiveBinding::Target::Trigger};
    std::string name;
    float value{1.0f};  ///< Velocity of a pad [0, 1], or 1 for a button.
    bool pressed{true}; ///< Actions: false when a held button is released, or a toggle is switched off.
    bool toggle{false}; ///< Actions: the binding is a toggle, so @p pressed is the new on/off state.
};

/**
 * @struct LiveMessage
 * @brief A received MIDI message with the alias of the device it came from (for the monitor).
 */
struct LiveMessage {
    std::string device;
    ivf::MidiMessage message;
};

/**
 * @class LiveControls
 * @brief Live layer on top of the automation: MIDI faders, knobs and pads mapped by a binding file.
 *
 * The bindings are read from a JSON file that is reloaded when it changes:
 *
 * @code{.json}
 * {
 *   "devices": { "mixer": "nanoKONTROL", "pads": "MPD218" },
 *   "bindings": [
 *     { "device": "mixer", "cc": 0, "param": "fog",   "range": [0.0, 0.2], "pickup": true },
 *     { "device": "mixer", "cc": 1, "param": "bloom", "range": [-0.5, 0.5], "mode": "offset" },
 *     { "device": "mixer", "cc": 2, "param": "color", "component": 0, "range": [0, 1] },
 *     { "device": "pads", "note": 36, "param": "blackout", "smoothing": 0.15 },
 *     { "device": "pads", "note": 37, "param": "strobe", "toggle": true },
 *     { "device": "pads", "note": 38, "trigger": "shockwave" },
 *     { "device": "mixer", "cc": 45, "action": "record" }
 *   ]
 * }
 * @endcode
 *
 * - **devices** maps an alias to part of a MIDI port name (case-insensitive). Each
 *   device is opened when it is found, also when it is plugged in later. Without a
 *   "devices" section all ports are opened, with the port name as alias.
 * - A binding listens to a "cc" or a "note", optionally only from one "device"
 *   and "channel" (1-16, as on the device), and drives a "param", a "trigger" or an "action".
 * - Parameters: "mode" is "override" (default), "offset" or "scale"; "range" maps
 *   the control to [min, max]; "component" picks one component of a vector
 *   parameter; "smoothing" is a time constant in seconds (default 0.08). With
 *   "pickup", an override only takes over when the control passes the current
 *   value, so it never jumps.
 * - Pads (notes) drive a parameter to max while held and back to min when
 *   released, or switch with "toggle". With "velocity" the pressed value follows
 *   how hard the pad is hit. Buttons sending CC 127/0 behave the same way.
 * - Triggers and actions fire when a pad is hit or a button is pressed.
 *
 * A control has no effect before it is first moved, so a scene starts exactly as
 * its automation describes. An override stays in effect until it is released with
 * release(), e.g. by the "release" action.
 *
 * GLFWMusicSceneWindow owns a LiveControls object and applies it in param();
 * applications rarely use this class directly.
 */
class LiveControls {
public:
    LiveControls();
    ~LiveControls();

    /**
     * @brief Load bindings from a JSON file, replacing the current ones, and open the devices.
     *
     * On error the current bindings are kept.
     */
    bool loadFile(const std::string &path, std::string *error = nullptr);

    /** @brief Reload the file loaded with loadFile() if it changed on disk. */
    bool reloadIfChanged(std::string *error = nullptr);

    [[nodiscard]] const std::string &path() const { return m_path; }

    /** @brief Replace the bindings (instead of loading a file). */
    void setBindings(std::vector<LiveBinding> bindings);
    [[nodiscard]] const std::vector<LiveBinding> &bindings() const { return m_bindings; }

    /**
     * @brief Set the devices to open: alias -> part of the port name.
     *
     * An empty map opens all ports.
     */
    void setDevices(std::map<std::string, std::string> devices);

    /**
     * @brief Read pending MIDI messages, advance the smoothing and return the triggers and actions fired.
     * @param dt Frame time in seconds.
     */
    std::vector<LiveEvent> update(double dt);

    /**
     * @brief Handle a message as if it came from @p device.
     *
     * update() calls this for every received message; it is public so a scene can
     * feed messages from elsewhere (another input, a test).
     */
    void handleMessage(const std::string &device, const ivf::MidiMessage &message);

    /**
     * @brief Combine the live controls of a parameter with its automation value.
     * @param name Parameter name.
     * @param base Value from the automation (or the scene's fallback).
     * @param components Number of components the scene uses (1-4).
     * @return @p base with the active controls of @p name applied in file order.
     */
    [[nodiscard]] glm::vec4 apply(std::string_view name, glm::vec4 base, int components) const;

    /** @brief true if a control of parameter @p name is currently in effect. */
    [[nodiscard]] bool active(std::string_view name) const;

    /** @brief Names of the parameters with a control in effect. */
    [[nodiscard]] std::vector<std::string> activeParams() const;

    /** @brief Hand all overrides back to the automation and reset offsets and scales. */
    void release();

    /** @brief Release the controls of one parameter. */
    void release(std::string_view name);

    /** @brief Close all devices. They are opened again by the next update(). */
    void closeDevices();

    /** @brief Status of a configured device. */
    struct DeviceStatus {
        std::string alias;
        std::string match;    ///< Part of the port name searched for.
        std::string portName; ///< Port in use; empty if the device is not connected.
    };
    [[nodiscard]] std::vector<DeviceStatus> devices() const;

    /** @brief Names of all MIDI input ports. */
    [[nodiscard]] std::vector<std::string> ports() const;

    /** @brief The most recent messages, newest last (for a MIDI monitor). */
    [[nodiscard]] const std::deque<LiveMessage> &recentMessages() const { return m_recent; }

    /** @brief Current output of binding @p index (after smoothing) and whether it is in effect. */
    [[nodiscard]] float bindingValue(size_t index) const;
    [[nodiscard]] bool bindingActive(size_t index) const;

private:
    struct State {
        float target{0.0f};       ///< Value the control is set to, in [min, max].
        float value{0.0f};        ///< Smoothed value.
        bool active{false};       ///< In effect (moved since start or the last release).
        bool armed{false};        ///< Pickup: control moved but has not yet passed the current value.
        float armedFrom{0.0f};    ///< Pickup: the control value when it was first moved.
        bool on{false};           ///< Pads and buttons: currently on.
        bool buttonDown{false};   ///< CC buttons: last value was >= 64.
        mutable float base{0.0f}; ///< Override: the value it replaces, from the last apply().
        mutable bool hasBase{false};
    };

    void openDevices();
    bool matches(const LiveBinding &binding, const std::string &device, const ivf::MidiMessage &message) const;
    void setTarget(size_t index, float value);
    void press(size_t index, float velocity, bool down);

    std::vector<LiveBinding> m_bindings;
    std::vector<State> m_states;
    std::vector<LiveEvent> m_events;

    std::map<std::string, std::string> m_deviceMatches;
    struct Device {
        std::string portName;
        ivf::MidiControllerPtr controller;
    };
    std::map<std::string, Device> m_devices;
    std::unique_ptr<ivf::MidiController> m_scanner; ///< Lists ports without opening one.
    double m_scanTimer{0.0};
    bool m_scanned{false};

    std::deque<LiveMessage> m_recent;

    std::string m_path;
    std::filesystem::file_time_type m_fileTime{};
};

} // namespace ivfmusic
