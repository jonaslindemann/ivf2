#pragma once

/**
 * @file midi_controller.h
 * @brief MIDI input wrapper using RtMidi.
 *
 * @section Requirements
 * Requires RtMidi, declared as a vcpkg dependency in vcpkg.json.
 * After running `vcpkg install` the header is available as `<RtMidi.h>`.
 *
 * MidiController uses a pimpl so this header has zero dependency on RtMidi types.
 * `midi_controller.cpp` is the only translation unit that includes RtMidi.
 *
 * @section Threading
 * RtMidi delivers messages on its own thread. MidiController parses them into
 * MidiMessage values and pushes them onto an internal mutex-guarded queue. The
 * application drains the queue from its main/render thread via poll(), so all
 * downstream parameter writes happen on the main thread.
 *
 * @section Usage
 * @code
 * // In onSetup():
 * auto midi = MidiController::create();
 * auto ports = midi->listPorts();
 * if (!ports.empty())
 *     midi->openPort(0);
 *
 * // In onUpdate():
 * for (const auto& msg : midi->poll())
 *     if (msg.type == MidiMessage::ControlChange)
 *         handleCC(msg.channel, msg.data1, msg.data2);
 * @endcode
 */

#include <memory>
#include <string>
#include <vector>

namespace ivf {

struct MidiControllerImpl;

/**
 * @struct MidiMessage
 * @brief A parsed MIDI channel-voice message.
 */
struct MidiMessage {
    enum Type { NoteOff, NoteOn, ControlChange, PitchBend, Other };

    Type type{Other};
    int channel{0}; ///< MIDI channel 0-15.
    int data1{0};   ///< First data byte (e.g. CC number, note number).
    int data2{0};   ///< Second data byte (e.g. CC value, velocity).
    int status{0};  ///< Raw status byte.
};

class MidiController;
using MidiControllerPtr = std::shared_ptr<MidiController>;

/**
 * @class MidiController
 * @brief Receives MIDI input and exposes parsed messages for the main thread.
 */
class MidiController {
public:
    MidiController();
    ~MidiController();

    /**
     * @brief Create a MidiController instance.
     */
    static MidiControllerPtr create();

    /**
     * @brief List the names of available MIDI input ports.
     * @return Port names indexed by port number.
     */
    std::vector<std::string> listPorts() const;

    /**
     * @brief Open a MIDI input port by index.
     * @param index Port index from listPorts().
     * @return true on success.
     */
    bool openPort(unsigned int index);

    /**
     * @brief Close the currently open port (if any).
     */
    void closePort();

    /**
     * @brief Whether an input port is currently open.
     */
    bool isOpen() const;

    /**
     * @brief Index of the open port, or -1 if none.
     */
    int openPortIndex() const;

    /**
     * @brief Drain and return all messages received since the previous call.
     *
     * Thread-safe: swaps the internal queue under a mutex. Call once per frame
     * from the main thread.
     */
    std::vector<MidiMessage> poll();

    /**
     * @brief Human-readable description of the last error, if any.
     */
    std::string lastError() const;

private:
    std::unique_ptr<MidiControllerImpl> m_impl;
};

} // namespace ivf
