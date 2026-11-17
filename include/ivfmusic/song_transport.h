#pragma once

/**
 * @file song_transport.h
 * @brief Song playback whose position is the master clock of a music-driven scene.
 */

#include <string>

namespace ivfmusic {

/**
 * @class SongTransport
 * @brief Plays the song and provides the song time that drives the scene.
 *
 * All animation reads time() rather than wall-clock time, so the visuals stay
 * in sync with the music while scrubbing, pausing and seeking. The audio
 * position only updates once per audio callback (~10 ms), so time() advances
 * smoothly by the frame time and is pulled towards the audio position, snapping
 * to it after a seek or a larger drift.
 *
 * Without audio (missing file, no device) or in free-running mode the transport
 * advances by the frame time alone. Free-running is used for frame export,
 * where time must advance in exact fixed steps.
 *
 * Musical time (beats, bars) is derived from a constant tempo.
 *
 * Playback goes through the music stream of ivf::AudioManager (startMusic()),
 * which is initialised on first use. Because the stream also feeds the analyser,
 * AudioManager::getAmplitude() and getFFT() react to the song as well.
 */
class SongTransport {
public:
    /**
     * @brief Load the song. Playback starts paused at 0.
     * @param path Audio file path.
     * @return true if the audio is playing through a device; false means silent free-running time.
     */
    bool load(const std::string &path);

    [[nodiscard]] bool hasAudio() const { return m_hasAudio; }

    void play();
    void pause();
    void togglePlay();
    [[nodiscard]] bool isPlaying() const { return m_playing; }

    /** @brief Move to a song position, clamped to the song length. */
    void seek(double seconds);

    /**
     * @brief Advance the transport. Call once per frame.
     * @param dt Frame time in seconds.
     */
    void update(double dt);

    /** @brief Song time of the current frame in seconds. */
    [[nodiscard]] double time() const { return m_time; }

    /** @brief Song length in seconds, 0 if unknown. */
    [[nodiscard]] double length() const { return m_length; }

    /**
     * @brief Set the length used without audio, e.g. from the score. Ignored when audio is loaded.
     */
    void setLength(double seconds);

    /**
     * @brief Advance by the frame time only and keep the audio paused.
     *
     * Used for frame export. When switched off, the audio resumes at the current time.
     */
    void setFreeRunning(bool freeRunning);
    [[nodiscard]] bool freeRunning() const { return m_freeRunning; }

    /** @brief Output volume [0, 1]; does not affect the audio analysis. */
    void setVolume(float volume);
    [[nodiscard]] float volume() const;

    /**
     * @brief Set the tempo used for musical time.
     * @param bpm Beats per minute.
     * @param firstBeat Song time of the first downbeat in seconds.
     * @param beatsPerBar Beats per bar.
     */
    void setTempo(double bpm, double firstBeat = 0.0, int beatsPerBar = 4);
    [[nodiscard]] double bpm() const { return m_bpm; }
    [[nodiscard]] double firstBeat() const { return m_firstBeat; }
    [[nodiscard]] int beatsPerBar() const { return m_beatsPerBar; }

    /** @brief Beats since the first downbeat (fractional, negative before it). */
    [[nodiscard]] double beats() const { return beatsAt(m_time); }
    /** @brief Bars since the first downbeat (fractional, negative before it). */
    [[nodiscard]] double bars() const { return beats() / m_beatsPerBar; }

    [[nodiscard]] double beatsAt(double seconds) const;
    [[nodiscard]] double timeAtBeat(double beat) const;
    [[nodiscard]] double timeAtBar(double bar) const;

    /** @brief Phase within the current beat in [0, 1), e.g. for a pulse on every beat. */
    [[nodiscard]] double beatPhase() const;

private:
    double m_time{0.0};
    double m_length{0.0};
    bool m_hasAudio{false};
    bool m_playing{false};
    bool m_freeRunning{false};
    float m_silentVolume{1.0f}; ///< Volume setting without audio, so mute state survives.

    double m_bpm{120.0};
    double m_firstBeat{0.0};
    int m_beatsPerBar{4};
};

} // namespace ivfmusic
