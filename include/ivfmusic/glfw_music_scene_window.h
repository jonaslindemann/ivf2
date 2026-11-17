#pragma once

/**
 * @file glfw_music_scene_window.h
 * @brief Scene window driven by a song: transport, automation, score and presentation mode.
 */

#include <ivfmusic/automation.h>
#include <ivfmusic/music_scene_config.h>
#include <ivfmusic/score.h>
#include <ivfmusic/song_transport.h>

#include <ivf/scene_timeline.h>
#include <ivfui/glfw_application.h>
#include <ivfui/glfw_scene_window.h>
#include <ivfui/path_camera_manipulator.h>

#include <memory>
#include <optional>

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
 * automation panel, F4 switch between the path camera and the free orbit camera.
 * Presentation mode (--present) hides all development UI.
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

    /** @brief Automation parameter at the current position, @p fallback if it is not defined. */
    [[nodiscard]] float param(std::string_view name, float fallback = 0.0f) const
    {
        return m_automation.value(name, barPosition(), fallback);
    }
    [[nodiscard]] glm::vec3 param3(std::string_view name, glm::vec3 fallback = glm::vec3(0.0f)) const
    {
        return m_automation.vec3(name, barPosition(), fallback);
    }
    [[nodiscard]] glm::vec4 param4(std::string_view name, glm::vec4 fallback = glm::vec4(0.0f)) const
    {
        return m_automation.vec4(name, barPosition(), fallback);
    }

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

    int onSetup() final;
    void onUpdate() final;
    void onDrawUi() final;
    void onKey(int key, int scancode, int action, int mods) override;
    void onFrameExportStarted() override;
    void onMouseDrag(double x, double y, double dx, double dy, int button) override;
    void onScroll(double xoffset, double yoffset) override;

private:
    void updateCamera(double dt);
    void drawTransport();
    void drawScoreMonitor();
    void drawAutomationPanel();
    void seekBy(double seconds);
    void seekToBar(double bar);
    void seekToMarker(int direction);
    void toggleMute();
    int sectionIndexAt(double time) const;

    MusicSceneConfig m_config;
    SongTransport m_transport;
    Score m_score;
    Automation m_automation;
    std::string m_automationError;
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
