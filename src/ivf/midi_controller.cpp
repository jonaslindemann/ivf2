/**
 * @file midi_controller.cpp
 * @brief MidiController implementation using RtMidi.
 *
 * RtMidi is declared as a vcpkg dependency (vcpkg.json).
 * After `vcpkg install` the header is available as <RtMidi.h>.
 *
 * If RtMidi.h is absent the library still compiles — all methods return
 * safe no-op/failure values so the rest of the application is unaffected.
 */

// The vcpkg layout installs the header as <rtmidi/RtMidi.h>; some other
// distributions place it at <RtMidi.h>. Support both.
#if __has_include(<rtmidi/RtMidi.h>) || __has_include(<RtMidi.h>)
    #define IVF_HAS_RTMIDI 1
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #ifdef _MSC_VER
    #pragma warning(push, 0)
    #endif
    #if __has_include(<rtmidi/RtMidi.h>)
        #include <rtmidi/RtMidi.h>
    #else
        #include <RtMidi.h>
    #endif
    #ifdef _MSC_VER
    #pragma warning(pop)
    #endif
#else
    #define IVF_HAS_RTMIDI 0
#endif

#include <ivf/midi_controller.h>

#include <mutex>

namespace ivf {

struct MidiControllerImpl {
#if IVF_HAS_RTMIDI
    std::unique_ptr<RtMidiIn> midiIn;
#endif
    std::mutex queueMutex;
    std::vector<MidiMessage> queue;
    int openPort{-1};
    std::string lastError;

    void pushMessage(const std::vector<unsigned char>& bytes)
    {
        if (bytes.empty())
            return;

        const unsigned char status = bytes[0];
        const unsigned char high = status & 0xF0u;

        // Ignore system/realtime messages (status >= 0xF0).
        if (high == 0xF0u)
            return;

        MidiMessage msg;
        msg.status = static_cast<int>(status);
        msg.channel = static_cast<int>(status & 0x0Fu);
        msg.data1 = bytes.size() > 1 ? static_cast<int>(bytes[1]) : 0;
        msg.data2 = bytes.size() > 2 ? static_cast<int>(bytes[2]) : 0;

        switch (high) {
        case 0x80u:
            msg.type = MidiMessage::NoteOff;
            break;
        case 0x90u:
            // NoteOn with velocity 0 is conventionally a NoteOff.
            msg.type = msg.data2 == 0 ? MidiMessage::NoteOff : MidiMessage::NoteOn;
            break;
        case 0xB0u:
            msg.type = MidiMessage::ControlChange;
            break;
        case 0xE0u:
            msg.type = MidiMessage::PitchBend;
            break;
        default:
            msg.type = MidiMessage::Other;
            break;
        }

        std::lock_guard<std::mutex> lock(queueMutex);
        queue.push_back(msg);
    }
};

#if IVF_HAS_RTMIDI
static void rtMidiCallback(double /*timeStamp*/, std::vector<unsigned char>* message, void* userData)
{
    auto* impl = static_cast<MidiControllerImpl*>(userData);
    if (impl && message)
        impl->pushMessage(*message);
}
#endif

MidiController::MidiController() : m_impl(std::make_unique<MidiControllerImpl>())
{
#if IVF_HAS_RTMIDI
    try {
        m_impl->midiIn = std::make_unique<RtMidiIn>();
    } catch (RtMidiError& error) {
        m_impl->lastError = error.getMessage();
    }
#else
    m_impl->lastError = "RtMidi not available at build time";
#endif
}

MidiController::~MidiController()
{
    closePort();
}

MidiControllerPtr MidiController::create()
{
    return std::make_shared<MidiController>();
}

std::vector<std::string> MidiController::listPorts() const
{
    std::vector<std::string> ports;
#if IVF_HAS_RTMIDI
    if (!m_impl->midiIn)
        return ports;
    try {
        const unsigned int count = m_impl->midiIn->getPortCount();
        ports.reserve(count);
        for (unsigned int i = 0; i < count; ++i)
            ports.push_back(m_impl->midiIn->getPortName(i));
    } catch (RtMidiError& error) {
        m_impl->lastError = error.getMessage();
    }
#endif
    return ports;
}

bool MidiController::openPort(unsigned int index)
{
#if IVF_HAS_RTMIDI
    if (!m_impl->midiIn)
        return false;
    try {
        closePort();
        m_impl->midiIn->openPort(index);
        // Don't ignore sysex/timing/active-sensing differently; we filter in pushMessage.
        m_impl->midiIn->ignoreTypes(true, true, true);
        m_impl->midiIn->setCallback(&rtMidiCallback, m_impl.get());
        m_impl->openPort = static_cast<int>(index);
        return true;
    } catch (RtMidiError& error) {
        m_impl->lastError = error.getMessage();
        m_impl->openPort = -1;
        return false;
    }
#else
    (void)index;
    return false;
#endif
}

void MidiController::closePort()
{
#if IVF_HAS_RTMIDI
    if (m_impl->midiIn && m_impl->openPort >= 0) {
        m_impl->midiIn->cancelCallback();
        m_impl->midiIn->closePort();
    }
#endif
    m_impl->openPort = -1;
}

bool MidiController::isOpen() const
{
    return m_impl->openPort >= 0;
}

int MidiController::openPortIndex() const
{
    return m_impl->openPort;
}

std::vector<MidiMessage> MidiController::poll()
{
    std::vector<MidiMessage> out;
    std::lock_guard<std::mutex> lock(m_impl->queueMutex);
    out.swap(m_impl->queue);
    return out;
}

std::string MidiController::lastError() const
{
    return m_impl->lastError;
}

} // namespace ivf
