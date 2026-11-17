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
 * // Music with transport control (e.g. a scene driven by the song);
 * // getAmplitude() / getFFT() then react to the music:
 * audio->startMusic("music/song.mp3", false);
 * audio->seekMusic(42.0);
 * double t = audio->musicPosition();
 * audio->pauseMusic();
 * audio->resumeMusic();
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
 * one streamed music track with transport control, microphone/loopback capture,
 * RMS amplitude measurement, and a lightweight band-energy frequency analyser
 * suitable for audio-reactive visuals.
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

    // ---- Music stream ------------------------------------------------------

    /**
     * @brief Stream an audio file as the music track, with transport control and analysis.
     *
     * There is one music stream. It is played through its own playback device,
     * whose callback both outputs audio to the speakers and feeds mono samples
     * into the analysis ring-buffer, so getAmplitude() / getFFT() react to the
     * music. Unlike loadSound(), the stream offers a sample-accurate position,
     * seeking, pausing and an end-of-file state, which makes it suitable as the
     * master clock of a music-driven scene (see ivfmusic::SongTransport).
     *
     * Works with any miniaudio-supported format (WAV, OGG, MP3, FLAC). Starting
     * a new file replaces the current one.
     *
     * @param path  File path relative to the working directory.
     * @param loop  If true the file loops.
     * @return true on success; see lastError() otherwise.
     */
    bool startMusic(std::string_view path, bool loop = true);

    /** @brief Stop the music stream. */
    void stopMusic();

    /** @brief Check whether a music stream is running (playing or paused). */
    bool musicRunning() const;

    /**
     * @brief Pause the music. The device keeps running and outputs silence,
     * so resuming is immediate.
     */
    void pauseMusic();

    /** @brief Resume paused music from its current position. */
    void resumeMusic();

    /** @brief Check whether the music is paused. */
    bool musicPaused() const;

    /**
     * @brief Get the playback position of the music.
     *
     * The position is the decoder cursor, updated once per audio callback
     * (typically every ~10 ms), and runs slightly ahead of what is heard by the
     * device latency. For smooth visual sync, interpolate between updates.
     *
     * @return Position in seconds, 0 if no music is running.
     */
    double musicPosition() const;

    /**
     * @brief Get the length of the music.
     * @return Length in seconds, 0 if no music is running or the length is unknown.
     */
    double musicLength() const;

    /**
     * @brief Move the playback position of the music.
     *
     * The seek is applied by the audio thread on its next callback; until then
     * musicPosition() reports the requested position.
     *
     * @param seconds New position in seconds, clamped to the file length.
     */
    void seekMusic(double seconds);

    /** @brief Check whether non-looping music has played to the end. */
    bool musicAtEnd() const;

    /**
     * @brief Set the output volume of the music.
     *
     * Only affects what is heard; analysis (getAmplitude(), getFFT()) uses the
     * unscaled signal. The volume is kept when a new file is started.
     * @param volume Volume in [0, 1].
     */
    void setMusicVolume(float volume);

    /** @brief Get the output volume of the music. */
    float musicVolume() const;

    // ---- Music stream, original names ----------------------------------------
    //
    // The music stream was first introduced for file-based audio reactivity.
    // These names are kept for compatibility and behave exactly like the
    // music functions above.

    /** @brief Same as startMusic(). */
    bool startFileAnalysis(std::string_view path, bool loop = true);
    /** @brief Same as stopMusic(). */
    void stopFileAnalysis();
    /** @brief Same as musicRunning(). */
    bool fileAnalysisRunning() const;
    /** @brief Same as pauseMusic(). */
    void pauseFileAnalysis();
    /** @brief Same as resumeMusic(). */
    void resumeFileAnalysis();
    /** @brief Same as musicPaused(). */
    bool fileAnalysisPaused() const;
    /** @brief Same as musicPosition(). */
    double fileAnalysisPosition() const;
    /** @brief Same as musicLength(). */
    double fileAnalysisLength() const;
    /** @brief Same as seekMusic(). */
    void seekFileAnalysis(double seconds);
    /** @brief Same as musicAtEnd(). */
    bool fileAnalysisAtEnd() const;
    /** @brief Same as setMusicVolume(). */
    void setFileAnalysisVolume(float volume);
    /** @brief Same as musicVolume(). */
    float fileAnalysisVolume() const;

    /**
     * @brief Return a human-readable description of the last error from
     * startMusic() or startCapture().
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
