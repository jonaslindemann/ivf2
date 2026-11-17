#pragma once

/**
 * @file glfw_music_scene_window.h
 * @brief Scene window driven by a song: transport, automation, score and presentation mode.
 */

#include <ivfmusic/automation.h>
#include <ivfmusic/automation_recorder.h>
#include <ivfmusic/live_controls.h>
#include <ivfmusic/music_scene_config.h>
#include <ivfmusic/score.h>
#include <ivfmusic/song_transport.h>

#include <ivf/scene_timeline.h>
#include <ivfui/glfw_application.h>
#include <ivfui/glfw_scene_window.h>
#include <ivfui/path_camera_manipulator.h>

#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace ivfmusic {

/**
 * @class GLFWMusicSceneWindow
 * @brief Scene window driven by the song: derive from it to make a music-synchronised scene.
 *
 * Owns the SongTransport, advances it every frame and publishes the song time to
 * all shader materials as the global uniforms `time` (seconds), `beat`
 * (fractional beats) and `beatPhase` ([0, 1) within the current beat).
 *
 * With a score file (MusicSceneConfig::scoreFile) the note events of every track
 * are available through score(), and the tempo is taken from it. With an
 * automation file (MusicSceneConfig::automationFile) keyframed parameters are
 * available through param(); the file is reloaded when it changes while the
 * scene runs.
 *
 * In development mode a transport bar shows the song position, bar, beat and
 * current marker, and allows play/pause and scrubbing. Keys: P play/pause,
 * Left/Right seek 5 s (Shift: one bar), PgUp/PgDn previous/next marker, Home
 * restart, M mute, F1 toggle the overlay, F2 the score track monitor, F3 the
 * automation panel, F4 switch between the path camera and the free orbit camera,
 * F5 the live control panel, F6 start/stop recording a take.
 * Presentation mode (--present) hides all development UI.
 *
 * @section music_live Live control
 *
 * With a controls file (MusicSceneConfig::controlsFile) MIDI faders, knobs and
 * pads change parameters on top of the automation; see LiveControls for the
 * file format. param() returns the automation value with the live controls
 * applied, so a scene needs no changes to be played live. Triggers (one-shot
 * events such as a shockwave) are read with triggerAge(). Every parameter,
 * trigger and action can also be bound to a key of an automation file.
 *
 * Built-in parameters, drawn over the finished frame: `blackout` (0-1 fade to
 * black), `strobe` (0-1 white flashes) and `strobeRate` (beats per flash,
 * default 0.25). Built-in actions: `play`, `record`, `release` (hand all
 * overrides back to the automation), `freeze` (hold the picture while the
 * music plays on), `resetCamera`, `nextMarker`, `previousMarker` and `undoTake`.
 * Other actions are passed to onLiveEvent().
 *
 * @section music_takes Recording takes
 *
 * Recording (F6 or the `record` action) captures what the live controls do
 * while the song plays. When it stops, every parameter that was under live
 * control and every trigger that fired becomes automation keys, saved to the
 * take file (MusicSceneConfig::takeFile, by default `<automation>.take.json`).
 * Like a punch-in, a take is used only in the regions that were performed;
 * everywhere else the automation file applies exactly as written, and it is
 * never rewritten, so it can still be edited. The recorded controls are
 * released, so the take plays back what was performed, and frame export (which
 * ignores MIDI) renders it frame-exact. `undoTake` restores the take file as it
 * was before the last recording; to discard a recorded lane, delete it and its
 * regions from the take file.
 *
 * The path camera follows cameraAt(), by default the automation parameters
 * `cameraPos` and `cameraTarget` (use "spline" keys for smooth motion). The
 * viewer can look around by dragging with the left mouse button and float away
 * from the path with the right button and the wheel; the camera returns to the
 * path when left alone.
 *
 * Subclasses implement onSceneSetup() to build the scene and onSceneUpdate() to
 * animate it from the song time:
 *
 * @code
 * class MyScene : public ivfmusic::GLFWMusicSceneWindow {
 * public:
 *     using GLFWMusicSceneWindow::GLFWMusicSceneWindow;
 * protected:
 *     int onSceneSetup() override { add(ivf::Sphere::create()); return 0; }
 *     void onSceneUpdate(double time, double dt) override { ... param("glow") ... }
 * };
 *
 * int main(int argc, char **argv)
 * {
 *     ivfmusic::MusicSceneConfig config;
 *     config.audioFile = "assets/song.mp3";
 *     config.automationFile = "assets/automation.json";
 *     return ivfmusic::run<MyScene>(config, argc, argv);
 * }
 * @endcode
 */
class GLFWMusicSceneWindow : public ivfui::GLFWSceneWindow {
public:
    explicit GLFWMusicSceneWindow(const MusicSceneConfig &config);

    [[nodiscard]] SongTransport &transport() { return m_transport; }
    [[nodiscard]] const SongTransport &transport() const { return m_transport; }
    [[nodiscard]] const Score &score() const { return m_score; }
    [[nodiscard]] const Automation &automation() const { return m_automation; }
    [[nodiscard]] const MusicSceneConfig &config() const { return m_config; }

    /** @brief Current musical position in bars: 1.0 is the start of the song. */
    [[nodiscard]] double barPosition() const { return m_transport.bars() + 1.0; }

    /** @brief Musical position in bars of any song time. */
    [[nodiscard]] double barAt(double time) const
    {
        return m_transport.beatsAt(time) / m_transport.beatsPerBar() + 1.0;
    }

    /**
     * @brief Parameter at the current position: the automation value with the live controls applied.
     *
     * @p fallback is used when the automation does not define the parameter.
     */
    [[nodiscard]] float param(std::string_view name, float fallback = 0.0f) const
    {
        return paramValue(name, glm::vec4(fallback, 0.0f, 0.0f, 0.0f), 1).x;
    }
    [[nodiscard]] glm::vec3 param3(std::string_view name, glm::vec3 fallback = glm::vec3(0.0f)) const
    {
        return glm::vec3(paramValue(name, glm::vec4(fallback, 0.0f), 3));
    }
    [[nodiscard]] glm::vec4 param4(std::string_view name, glm::vec4 fallback = glm::vec4(0.0f)) const
    {
        return paramValue(name, fallback, 4);
    }

    /**
     * @brief Seconds since trigger @p name last fired, at or before the current song position.
     *
     * Covers live hits and hits recorded in the automation (step keys of a lane
     * named after the trigger), so a scene that animates from the age looks the
     * same live, in playback and in an export. Infinity if it has not fired.
     */
    [[nodiscard]] double triggerAge(std::string_view name) const;

    /** @brief Velocity [0, 1] of the most recent hit of trigger @p name, 0 if it has not fired. */
    [[nodiscard]] float triggerVelocity(std::string_view name) const;

    [[nodiscard]] LiveControls &liveControls() { return m_live; }
    [[nodiscard]] const LiveControls &liveControls() const { return m_live; }
    [[nodiscard]] bool recording() const { return m_recorder.recording(); }
    [[nodiscard]] bool frozen() const { return m_frozen; }

    /** @brief Start or stop recording a take; stopping saves it to the take file. */
    void setRecording(bool recording);

    [[nodiscard]] bool presentationMode() const { return m_config.presentationMode; }

    [[nodiscard]] ivfui::PathCameraManipulatorPtr pathCamera() const { return m_pathCamera; }
    [[nodiscard]] bool freeCamera() const { return m_freeCamera; }

    /**
     * @brief The authored camera path at a song time, without viewer input.
     *
     * Must be a pure function of @p time. The default reads the automation
     * parameters `cameraPos` and `cameraTarget`; returns nothing when there is no
     * path, and the camera is then left to the subclass.
     */
    [[nodiscard]] virtual std::optional<ivf::TimelineCamera> cameraAt(double time) const;

protected:
    /** @brief Build the scene. Return non-zero to abort. */
    virtual int onSceneSetup() = 0;

    /**
     * @brief Animate the scene. Called every frame after the transport has advanced.
     * @param time Song time in seconds.
     * @param dt Frame time in seconds.
     */
    virtual void onSceneUpdate(double time, double dt) {}

    /** @brief Draw scene-specific development UI (not called in presentation mode). */
    virtual void onSceneUi() {}

    /**
     * @brief A live trigger fired, or a control sent an action the window does not handle.
     *
     * Triggers are also available through triggerAge(), which includes recorded
     * hits; prefer it for anything that should appear in exports.
     */
    virtual void onLiveEvent(const LiveEvent &event) {}

    int onSetup() final;
    void onUpdate() final;
    void onDrawUi() final;
    void onKey(int key, int scancode, int action, int mods) override;
    void onFrameExportStarted() override;
    void doDraw() override;
    void onMouseDrag(double x, double y, double dx, double dy, int button) override;
    void onScroll(double xoffset, double yoffset) override;

private:
    void updateCamera(double dt);
    void drawTransport();
    void drawScoreMonitor();
    void drawAutomationPanel();
    void drawLivePanel();
    void drawOverlay();
    void loadAutomation();
    void updateLive(double dt);
    void handleAction(const LiveEvent &event);
    void undoTake();
    [[nodiscard]] glm::vec4 paramValue(std::string_view name, glm::vec4 fallback, int components) const;
    void seekBy(double seconds);
    void seekToBar(double bar);
    void seekToMarker(int direction);
    void toggleMute();
    int sectionIndexAt(double time) const;

    MusicSceneConfig m_config;
    SongTransport m_transport;
    Score m_score;
    Automation m_baseAutomation; ///< The hand-written automation file.
    Automation m_take;           ///< Recorded takes, laid over the automation file.
    Automation m_takeUndo;       ///< The take before the last recording.
    bool m_hasTakeUndo{false};
    std::string m_takePath;
    Automation m_automation; ///< In effect: the automation file with the take laid over it.
    std::string m_automationError;

    LiveControls m_live;
    bool m_liveEnabled{false};
    std::string m_liveError;
    bool m_showLivePanel{false};
    bool m_frozen{false};
    mutable AutomationRecorder m_recorder;
    struct Hit {
        double time;
        float velocity;
    };
    std::map<std::string, Hit, std::less<>> m_liveHits; ///< Most recent live hit of each trigger, in song time.
    unsigned int m_overlayProgram{0};
    unsigned int m_overlayVao{0};
    double m_reloadTimer{0.0};
    bool m_showOverlay{true};
    bool m_holdTransport{false}; ///< Keep the transport for one frame, so the first exported frame is at startTime.
    bool m_showScoreMonitor{false};
    bool m_showAutomationPanel{false};
    float m_volumeBeforeMute{1.0f};
    ivfui::PathCameraManipulatorPtr m_pathCamera;
    bool m_freeCamera{false};
};

/**
 * @brief Parse the command line, open the scene window and run it.
 * @tparam Window A GLFWMusicSceneWindow subclass constructible from a MusicSceneConfig.
 * @param config Defaults of the scene, overridden by command-line arguments.
 * @param argc Argument count from main().
 * @param argv Arguments from main(); see MusicSceneConfig::parseArgs().
 * @return Process exit code.
 */
template <typename Window> int run(MusicSceneConfig config, int argc, char **argv)
{
#ifdef IVFMUSIC_ASSET_SOURCE_DIR
    // Defined by ivfmusic_add_executable() for the application's own sources
    if (config.assetSourceDir.empty())
        config.assetSourceDir = IVFMUSIC_ASSET_SOURCE_DIR;
#endif

    switch (config.parseArgs(argc, argv))
    {
    case MusicSceneConfig::ParseResult::Exit:
        return 0;
    case MusicSceneConfig::ParseResult::Error:
        return 1;
    case MusicSceneConfig::ParseResult::Run:
        break;
    }

    auto app = ivfui::GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = std::make_shared<Window>(config);
    app->addWindow(window);
    return app->loop();
}

} // namespace ivfmusic
