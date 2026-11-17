#pragma once

/**
 * @file music_scene_config.h
 * @brief Start-up settings of a music-driven scene, overridable from the command line.
 */

#include <string>

namespace ivfmusic {

/**
 * @struct MusicSceneConfig
 * @brief Settings a GLFWMusicSceneWindow starts with.
 *
 * An application fills in its defaults (title, song, tempo) and then calls
 * parseArgs() so they can be overridden from the command line.
 */
struct MusicSceneConfig {
    /** @brief Outcome of parseArgs(). */
    enum class ParseResult {
        Run,  ///< Start the scene.
        Exit, ///< Exit successfully, e.g. after --help.
        Error ///< Invalid arguments.
    };

    std::string title{"Music Scene"};
    int width{1280};
    int height{720};
    bool fullscreen{false};

    /// Presentation mode: fullscreen, no development UI, plays the song from startTime.
    bool presentationMode{false};

    std::string audioFile;      ///< Rendered song, relative to the working directory.
    std::string scoreFile;      ///< Composition with note events (.xrns), optional.
    std::string automationFile; ///< Automation (.json), optional; reloaded when changed.

    /// Project asset directory. Paths starting with "assets/" are read from here
    /// when the file exists, so edits in the project are picked up without a rebuild.
    std::string assetSourceDir;
    double bpm{0.0};       ///< Tempo for beats and bars; 0 takes it from the score (or 120).
    double firstBeat{0.0}; ///< Song time of the first downbeat in seconds.
    int beatsPerBar{4};

    double startTime{0.0}; ///< Song time to start at in seconds.
    bool autoplay{true};   ///< Start playing immediately.
    float volume{1.0f};

    /// Song length used when there is no audio and no score; 0 is unbounded.
    double silentLength{0.0};

    /// Frame export: when set, renders a PNG sequence into this directory at
    /// exportFps in fixed time steps (no development UI, silent), then exits.
    std::string exportDir;
    int exportFps{60};
    double endTime{0.0}; ///< Song time to stop exporting at in seconds; 0 is the end of the song.

    /**
     * @brief Override settings from command-line arguments.
     *
     * Supported: --fullscreen, --present (alias --show), --size WxH, --start SECONDS,
     * --end SECONDS, --export DIR, --fps FPS, --paused, --mute, --bpm BPM, --help.
     */
    ParseResult parseArgs(int argc, char **argv);

    /**
     * @brief Resolve an asset path, preferring the project copy in assetSourceDir.
     * @param path Path such as "assets/song.wav", relative to the executable directory.
     */
    [[nodiscard]] std::string resolve(const std::string &path) const;

    /** @brief Print the supported command-line arguments. */
    static void printUsage(const char *program);
};

} // namespace ivfmusic
