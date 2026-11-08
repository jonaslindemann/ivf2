#pragma once

/**
 * @file audio_manager.h
 * @brief Audio playback and capture singleton using miniaudio.
 *
 * @section Requirements
 * Requires miniaudio, which is declared as a vcpkg dependency in vcpkg.json.
 * After running `vcpkg install` the header is available as `<miniaudio.h>`.
 *
 * AudioManager uses a pimpl so this header has zero dependency on miniaudio types.
 * `audio_manager.cpp` is the only translation unit that includes miniaudio.
 *
 * @section Usage
 * @code
 * // In onSetup():
 * auto audio = AudioManager::create();
 * audio->init();
 * int snd = audio->loadSound("sounds/hit.wav");
 * audio->play(snd);
 *
 * // Audio-reactive (call update() each frame from onUpdate()):
 * audio->startCapture();
 * // in onUpdate():
 * AudioManager::instance()->update();
 * float amp = AudioManager::instance()->getAmplitude();
 * auto fft = AudioManager::instance()->getFFT(16);
 *
 * // Music with transport control (e.g. a timeline driven by the song):
 * audio->startFileAnalysis("music/song.mp3", false);
 * audio->seekFileAnalysis(42.0);
 * double t = audio->fileAnalysisPosition();
 * audio->pauseFileAnalysis();
 * audio->resumeFileAnalysis();
 * @endcode
 */

#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace ivf {

struct AudioManagerImpl;

/**
 * @class AudioManager
 * @brief Singleton audio manager for playback and audio-reactivity.
 *
 * Supports loading and playing sounds (WAV, OGG, MP3, FLAC via miniaudio),
 * microphone/loopback capture, RMS amplitude measurement, and a lightweight
 * band-energy frequency analyser suitable for audio-reactive visuals.
 */
class AudioManager {
public:
    ~AudioManager();

    /**
     * @brief Get (or create) the singleton instance.
     */
    static AudioManager* instance();

    /**
     * @brief Create the singleton instance.
     * @return Pointer to the singleton.
     */
    static AudioManager* create();

    /**
     * @brief Destroy the singleton and release all resources.
     */
    static void drop();

    /**
     * @brief Initialise the miniaudio engine. Must be called before any other method.
     * @return true on success, false if miniaudio fails to initialise.
     */
    bool init();

    // ---- Playback ----------------------------------------------------------

    /**
     * @brief Load a sound file.
     * @param path File path relative to the working directory.
     * @return Sound handle (≥ 0) on success, -1 on failure.
     */
    int loadSound(std::string_view path);

    /**
     * @brief Play a loaded sound.
     * @param handle Sound handle returned by loadSound().
     * @param loop   If true, the sound loops indefinitely.
     */
    void play(int handle, bool loop = false);

    /**
     * @brief Stop a playing sound.
     * @param handle Sound handle.
     */
    void stop(int handle);

    /**
     * @brief Pause a playing sound, keeping its playback position.
     * @param handle Sound handle.
     */
    void pause(int handle);

    /**
     * @brief Resume a paused sound from its current position.
     *
     * Unlike play(), this does not rewind to the start.
     * @param handle Sound handle.
     */
    void resume(int handle);

    /**
     * @brief Check whether a sound is currently playing.
     * @param handle Sound handle.
     */
    bool isPlaying(int handle) const;

    /**
     * @brief Get the playback position of a sound.
     * @param handle Sound handle.
     * @return Position in seconds, 0 for an invalid handle.
     */
    double soundPosition(int handle) const;

    /**
     * @brief Get the length of a sound.
     * @param handle Sound handle.
     * @return Length in seconds, 0 if unknown (e.g. still loading asynchronously).
     */
    double soundLength(int handle) const;

    /**
     * @brief Move the playback position of a sound.
     * @param handle  Sound handle.
     * @param seconds New position in seconds.
     */
    void seekSound(int handle, double seconds);

    /**
     * @brief Set the volume of a sound.
     * @param handle Sound handle.
     * @param volume Volume in the range [0, 1].
     */
    void setVolume(int handle, float volume);

    /**
     * @brief Set the spatial position of a sound source.
     * @param handle Sound handle.
     * @param pos    World-space position.
     */
    void setSoundPosition(int handle, glm::vec3 pos);

    // ---- File-based audio reactivity ----------------------------------------

    /**
     * @brief Play an audio file and simultaneously feed its PCM data into the
     * analysis ring-buffer so that getAmplitude() / getFFT() react to the file.
     *
     * The file is played through a custom playback device whose callback both
     * outputs audio to the speakers and pushes mono samples into the ring-buffer.
     * This works with any miniaudio-supported format (WAV, OGG, MP3, FLAC).
     *
     * @param path  File path relative to the working directory.
     * @param loop  If true the file loops.
     * @return true on success.
     */
    bool startFileAnalysis(std::string_view path, bool loop = true);

    /**
     * @brief Stop file-based analysis playback.
     */
    void stopFileAnalysis();

    /**
     * @brief Check whether file-based analysis is currently running.
     */
    bool fileAnalysisRunning() const;

    /**
     * @brief Pause file playback. The device keeps running and outputs silence,
     * so resuming is immediate.
     */
    void pauseFileAnalysis();

    /**
     * @brief Resume paused file playback from its current position.
     */
    void resumeFileAnalysis();

    /**
     * @brief Check whether file playback is paused.
     */
    bool fileAnalysisPaused() const;

    /**
     * @brief Get the playback position of the analysed file.
     *
     * The position is the decoder cursor, updated once per audio callback
     * (typically every ~10 ms), and runs slightly ahead of what is heard by the
     * device latency. For smooth visual sync, interpolate between updates.
     *
     * @return Position in seconds, 0 if no file is playing.
     */
    double fileAnalysisPosition() const;

    /**
     * @brief Get the length of the analysed file.
     * @return Length in seconds, 0 if no file is playing or the length is unknown.
     */
    double fileAnalysisLength() const;

    /**
     * @brief Move the playback position of the analysed file.
     *
     * The seek is applied by the audio thread on its next callback; until then
     * fileAnalysisPosition() reports the requested position.
     *
     * @param seconds New position in seconds, clamped to the file length.
     */
    void seekFileAnalysis(double seconds);

    /**
     * @brief Check whether a non-looping file has played to the end.
     */
    bool fileAnalysisAtEnd() const;

    /**
     * @brief Return a human-readable description of the last error from
     * startFileAnalysis() or startCapture().
     */
    std::string lastError() const;

    // ---- Capture / audio reactivity ----------------------------------------

    /**
     * @brief Start microphone or loopback capture.
     * @return true if capture was started successfully.
     */
    bool startCapture();

    /**
     * @brief Stop capture.
     */
    void stopCapture();

    /**
     * @brief Get the RMS amplitude of the most-recent capture window.
     * @return Amplitude in [0, 1].
     */
    float getAmplitude();

    /**
     * @brief Compute a frequency spectrum of the most recent analysis samples.
     *
     * Applies a Hann window to the last 2048 samples and evaluates a DFT at
     * @p bands logarithmically spaced frequencies between 40 Hz and 16 kHz.
     * Each band is a single-frequency magnitude rather than the energy of a
     * frequency range, which is adequate for audio-reactive visuals without
     * requiring an FFT library.
     *
     * @param bands Number of frequency bands (1–64).
     * @return Vector of @p bands magnitudes.
     */
    std::vector<float> getFFT(int bands = 32);

    /**
     * @brief Per-frame update. Call once per frame from your window's onUpdate().
     *
     * Currently a no-op for engine playback (miniaudio runs on its own thread),
     * but required for future spatial audio position updates and for any
     * amplitude-smoothing calculations.
     */
    void update();

private:
    AudioManager();

    static AudioManager* m_instance;
    std::unique_ptr<AudioManagerImpl> m_impl;
};

} // namespace ivf
