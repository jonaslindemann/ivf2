#include <ivfmusic/glfw_music_scene_window.h>

#include <imgui.h>
#include <ivf/logger.h>
#include <ivf/shader_material.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

using namespace ivfmusic;

namespace {

std::string formatTime(double seconds)
{
    seconds = std::max(seconds, 0.0);
    const int minutes = int(seconds / 60.0);
    char text[32];
    std::snprintf(text, sizeof(text), "%02d:%06.3f", minutes, seconds - minutes * 60.0);
    return text;
}

GLFWmonitor *monitorFor(const MusicSceneConfig &config)
{
    return config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
}

} // namespace

GLFWMusicSceneWindow::GLFWMusicSceneWindow(const MusicSceneConfig &config)
    : GLFWSceneWindow(config.width, config.height, config.title, monitorFor(config)), m_config(config),
      m_pathCamera(ivfui::PathCameraManipulator::create())
{
}

int GLFWMusicSceneWindow::onSetup()
{
    // Keep development panels inside the scene window instead of letting ImGui
    // open separate OS windows (which also keeps panel positions window-relative)
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

    if (!m_config.audioFile.empty())
        m_transport.load(m_config.resolve(m_config.audioFile));

    double bpm = m_config.bpm;
    int beatsPerBar = m_config.beatsPerBar;
    if (!m_config.scoreFile.empty())
    {
        std::string error;
        if (m_score.load(m_config.resolve(m_config.scoreFile), &error))
        {
            ivf::logInfo("Loaded " + m_config.scoreFile + ": " + std::to_string(m_score.tracks().size()) +
                             " tracks, " + std::to_string(m_score.bpm()) + " BPM",
                         "GLFWMusicSceneWindow");
            if (bpm <= 0.0)
            {
                bpm = m_score.bpm();
                beatsPerBar = m_score.beatsPerBar();
            }
            if (m_transport.hasAudio() && std::abs(m_score.length() - m_transport.length()) > 0.5)
                ivf::logWarning("Score is " + std::to_string(m_score.length()) + " s but the audio is " +
                                    std::to_string(m_transport.length()) + " s",
                                "GLFWMusicSceneWindow");
        }
        else
        {
            ivf::logError(error, "GLFWMusicSceneWindow");
        }
    }
    m_transport.setTempo(bpm > 0.0 ? bpm : 120.0, m_config.firstBeat, beatsPerBar);
    m_transport.setVolume(m_config.volume);

    // Without audio the song length comes from the score, or from the config
    if (!m_transport.hasAudio())
        m_transport.setLength(!m_score.empty() ? m_score.length() : m_config.silentLength);

    if (!m_config.automationFile.empty())
    {
        const std::string path = m_config.resolve(m_config.automationFile);
        if (m_automation.loadFile(path, &m_automationError))
            ivf::logInfo("Loaded automation " + path, "GLFWMusicSceneWindow");
        else
            ivf::logError(m_automationError, "GLFWMusicSceneWindow");
    }

    if (m_config.presentationMode || !m_config.exportDir.empty())
    {
        m_showOverlay = false;
        hideMainMenu();
    }

    if (int result = onSceneSetup(); result != 0)
        return result;

    // The path camera owns the view; F4 hands it to the orbit manipulator
    cameraManipulator()->setManipulationBlocked(true);

    m_transport.seek(m_config.startTime);

    if (!m_config.exportDir.empty())
    {
        // Silent, in fixed steps of one video frame, from startTime to endTime
        const double end = m_config.endTime > 0.0 ? m_config.endTime : m_transport.length();
        if (end <= m_config.startTime)
        {
            ivf::logError("Nothing to export: set --end after --start", "GLFWMusicSceneWindow");
            return 1;
        }
        m_transport.setFreeRunning(true);
        m_transport.play();

        ivfui::GLFWWindow::FrameExportSettings settings;
        settings.directory = m_config.exportDir;
        settings.fps = m_config.exportFps;
        // The tolerance keeps e.g. 0.4 s at 5 fps at 2 frames despite rounding (2.0000000001)
        settings.frameCount = int(std::ceil((end - m_config.startTime) * m_config.exportFps - 1e-6));
        settings.closeWhenDone = true;
        startFrameExport(settings);
        return 0;
    }

    if (m_config.autoplay)
        m_transport.play();
    return 0;
}

void GLFWMusicSceneWindow::onFrameExportStarted()
{
    m_holdTransport = true;
}

void GLFWMusicSceneWindow::onUpdate()
{
    const double dt = frameTime();
    if (m_holdTransport)
        m_holdTransport = false;
    else
        m_transport.update(dt);

    // Pick up edits of the automation file while developing
    if (!m_config.presentationMode && (m_reloadTimer += dt) >= 0.5)
    {
        m_reloadTimer = 0.0;
        std::string error;
        if (m_automation.reloadIfChanged(&error))
        {
            m_automationError.clear();
            ivf::logInfo("Reloaded automation " + m_automation.path(), "GLFWMusicSceneWindow");
        }
        else if (!error.empty())
        {
            m_automationError = error;
            ivf::logError(error, "GLFWMusicSceneWindow");
        }
    }

    ivf::ShaderMaterial::setGlobalUniform("time", float(m_transport.time()));
    ivf::ShaderMaterial::setGlobalUniform("beat", float(m_transport.beats()));
    ivf::ShaderMaterial::setGlobalUniform("beatPhase", float(m_transport.beatPhase()));

    onSceneUpdate(m_transport.time(), dt);
    updateCamera(dt);
}

std::optional<ivf::TimelineCamera> GLFWMusicSceneWindow::cameraAt(double time) const
{
    if (!m_automation.has("cameraPos"))
        return std::nullopt;
    const double bar = barAt(time);
    ivf::TimelineCamera camera;
    camera.position = m_automation.vec3("cameraPos", bar);
    camera.target = m_automation.vec3("cameraTarget", bar, camera.position + glm::vec3(0.0f, 0.0f, -1.0f));
    return camera;
}

void GLFWMusicSceneWindow::updateCamera(double dt)
{
    if (m_freeCamera)
        return;
    const auto path = cameraAt(m_transport.time());
    if (!path)
        return;
    m_pathCamera->update(dt);
    m_pathCamera->applyTo(cameraManipulator(), *path);
}

void GLFWMusicSceneWindow::onMouseDrag(double x, double y, double dx, double dy, int button)
{
    GLFWSceneWindow::onMouseDrag(x, y, dx, dy, button);
    if (m_freeCamera || ImGui::GetIO().WantCaptureMouse)
        return;
    m_pathCamera->handleMouseDrag(dx, dy, button);
}

void GLFWMusicSceneWindow::onScroll(double xoffset, double yoffset)
{
    GLFWSceneWindow::onScroll(xoffset, yoffset);
    if (!m_freeCamera && !ImGui::GetIO().WantCaptureMouse)
        m_pathCamera->handleScroll(yoffset);
}

void GLFWMusicSceneWindow::onDrawUi()
{
    if (m_config.presentationMode || !m_showOverlay)
        return;

    drawTransport();
    if (m_showScoreMonitor)
        drawScoreMonitor();
    if (m_showAutomationPanel)
        drawAutomationPanel();
    onSceneUi();
}

void GLFWMusicSceneWindow::drawAutomationPanel()
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({viewport->WorkPos.x + viewport->WorkSize.x - 660.0f, viewport->WorkPos.y + 10.0f},
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({320.0f, 420.0f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Automation", &m_showAutomationPanel);

    if (m_automation.path().empty())
    {
        ImGui::TextUnformatted("No automation file loaded");
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("%s", m_automation.path().c_str());
    if (!m_automationError.empty())
        ImGui::TextColored({1.0f, 0.4f, 0.3f, 1.0f}, "%s", m_automationError.c_str());

    const double bar = barPosition();
    if (ImGui::CollapsingHeader("Markers", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const int current = m_automation.markerIndexAt(bar);
        for (int i = 0; i < int(m_automation.markers().size()); ++i)
        {
            const auto &marker = m_automation.markers()[i];
            char label[160];
            std::snprintf(label, sizeof(label), "%5.1f  %s##marker%d", marker.bar, marker.name.c_str(), i);
            if (ImGui::Selectable(label, i == current))
                seekToBar(marker.bar);
        }
    }

    if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (const auto &[name, keys] : m_automation.lanes())
        {
            const glm::vec4 v = m_automation.vec4(name, bar);
            switch (m_automation.components(name))
            {
            case 1:
                ImGui::Text("%-16s %.3f", name.c_str(), v.x);
                break;
            case 2:
                ImGui::Text("%-16s %.3f %.3f", name.c_str(), v.x, v.y);
                break;
            case 3:
                ImGui::Text("%-16s %.3f %.3f %.3f", name.c_str(), v.x, v.y, v.z);
                break;
            default:
                ImGui::Text("%-16s %.3f %.3f %.3f %.3f", name.c_str(), v.x, v.y, v.z, v.w);
                break;
            }
        }
    }
    ImGui::End();
}

void GLFWMusicSceneWindow::seekToBar(double bar)
{
    m_transport.seek(m_transport.timeAtBar(bar - 1.0));
}

void GLFWMusicSceneWindow::seekToMarker(int direction)
{
    const auto &markers = m_automation.markers();
    if (markers.empty())
        return;

    // Jumping back from just after a marker goes to the one before it, so it can be stepped through
    const double bar = barPosition();
    int index = m_automation.markerIndexAt(bar);
    if (direction > 0)
        index = std::min(index + 1, int(markers.size()) - 1);
    else if (index >= 0 && bar - markers[index].bar < 0.25)
        index = std::max(index - 1, 0);
    else
        index = std::max(index, 0);
    seekToBar(markers[index].bar);
}

int GLFWMusicSceneWindow::sectionIndexAt(double time) const
{
    const auto &sections = m_score.sections();
    for (size_t i = 0; i < sections.size(); ++i)
        if (time < sections[i].end)
            return int(i);
    return sections.empty() ? -1 : int(sections.size()) - 1;
}

void GLFWMusicSceneWindow::drawScoreMonitor()
{
    ImGui::SetNextWindowPos({ImGui::GetMainViewport()->WorkPos.x + ImGui::GetMainViewport()->WorkSize.x - 330.0f,
                             ImGui::GetMainViewport()->WorkPos.y + 10.0f},
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({320.0f, 0.0f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Score", &m_showScoreMonitor);

    if (m_score.empty())
    {
        ImGui::TextUnformatted("No score file loaded");
        ImGui::End();
        return;
    }

    const double time = m_transport.time();
    if (int section = sectionIndexAt(time); section >= 0)
        ImGui::Text("Sequence slot %d: %s", section, m_score.sections()[section].name.c_str());

    // Short, sharp shape so individual notes are visible
    const AdsrShape shape{0.005f, 0.25f, 0.3f, 0.15f, true};
    for (const auto &track : m_score.tracks())
    {
        const float value = track.envelope(time, shape);
        ImGui::ProgressBar(value, {140.0f, 0.0f}, "");
        ImGui::SameLine();
        ImGui::TextUnformatted(track.name.c_str());
    }
    ImGui::End();
}

void GLFWMusicSceneWindow::drawTransport()
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    const float margin = 10.0f;
    ImGui::SetNextWindowPos({viewport->WorkPos.x + margin, viewport->WorkPos.y + viewport->WorkSize.y - margin},
                            ImGuiCond_Always, {0.0f, 1.0f});
    ImGui::SetNextWindowSize({viewport->WorkSize.x - 2.0f * margin, 0.0f}, ImGuiCond_Always);
    ImGui::Begin("Transport", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::Button(m_transport.isPlaying() ? "Pause" : "Play", {60.0f, 0.0f}))
        m_transport.togglePlay();
    ImGui::SameLine();
    if (ImGui::Button("|<"))
        m_transport.seek(0.0);

    ImGui::SameLine();
    const double bars = m_transport.bars();
    const int bar = int(std::floor(bars));
    const int beat = int(std::floor((bars - bar) * m_transport.beatsPerBar()));
    ImGui::Text("%s / %s   bar %3d.%d   %.1f bpm", formatTime(m_transport.time()).c_str(),
                formatTime(m_transport.length()).c_str(), bar + 1, beat + 1, m_transport.bpm());

    if (int marker = m_automation.markerIndexAt(barPosition()); marker >= 0)
    {
        ImGui::SameLine();
        ImGui::TextColored({0.6f, 0.85f, 1.0f, 1.0f}, "  %s", m_automation.markers()[marker].name.c_str());
    }
    if (!m_automationError.empty())
    {
        ImGui::SameLine();
        ImGui::TextColored({1.0f, 0.4f, 0.3f, 1.0f}, "  automation error (F3)");
    }

    ImGui::SameLine();
    bool muted = m_transport.volume() == 0.0f;
    if (ImGui::Checkbox("Mute", &muted))
        toggleMute();

    ImGui::SameLine();
    ImGui::Text("%5.1f fps%s", ImGui::GetIO().Framerate, m_transport.hasAudio() ? "" : "   (no audio)");

    // Scrubbing seeks continuously while dragging
    float position = float(m_transport.time());
    const float length = float(std::max(m_transport.length(), 1.0));
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::SliderFloat("##position", &position, 0.0f, length, ""))
        m_transport.seek(position);

    ImGui::End();
}

void GLFWMusicSceneWindow::seekBy(double seconds)
{
    m_transport.seek(m_transport.time() + seconds);
}

void GLFWMusicSceneWindow::toggleMute()
{
    if (m_transport.volume() > 0.0f)
    {
        m_volumeBeforeMute = m_transport.volume();
        m_transport.setVolume(0.0f);
    }
    else
    {
        m_transport.setVolume(m_volumeBeforeMute > 0.0f ? m_volumeBeforeMute : 1.0f);
    }
}

void GLFWMusicSceneWindow::onKey(int key, int scancode, int action, int mods)
{
    GLFWSceneWindow::onKey(key, scancode, action, mods);
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    if (!m_config.exportDir.empty())
        return; // transport and camera keys would change the exported frames

    const bool shift = (mods & GLFW_MOD_SHIFT) != 0;
    const double barLength = m_transport.timeAtBar(1.0) - m_transport.timeAtBar(0.0);

    switch (key)
    {
    case GLFW_KEY_P:
        if (action == GLFW_PRESS)
            m_transport.togglePlay();
        break;
    case GLFW_KEY_LEFT:
        seekBy(shift ? -barLength : -5.0);
        break;
    case GLFW_KEY_RIGHT:
        seekBy(shift ? barLength : 5.0);
        break;
    case GLFW_KEY_HOME:
        m_transport.seek(0.0);
        break;
    case GLFW_KEY_M:
        if (action == GLFW_PRESS)
            toggleMute();
        break;
    case GLFW_KEY_F1:
        if (action == GLFW_PRESS && !m_config.presentationMode)
            m_showOverlay = !m_showOverlay;
        break;
    case GLFW_KEY_F2:
        if (action == GLFW_PRESS && !m_config.presentationMode)
            m_showScoreMonitor = !m_showScoreMonitor;
        break;
    case GLFW_KEY_F3:
        if (action == GLFW_PRESS && !m_config.presentationMode)
            m_showAutomationPanel = !m_showAutomationPanel;
        break;
    case GLFW_KEY_F4:
        if (action == GLFW_PRESS && !m_config.presentationMode)
        {
            m_freeCamera = !m_freeCamera;
            m_pathCamera->reset();
            cameraManipulator()->setManipulationBlocked(!m_freeCamera);
        }
        break;
    case GLFW_KEY_PAGE_UP:
        seekToMarker(-1);
        break;
    case GLFW_KEY_PAGE_DOWN:
        seekToMarker(1);
        break;
    default:
        break;
    }
}
