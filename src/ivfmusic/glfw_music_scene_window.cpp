#include <ivfmusic/glfw_music_scene_window.h>

#include <glad/glad.h>
#include <imgui.h>
#include <ivf/glsl_es.h>
#include <ivf/logger.h>
#include <ivf/shader_material.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <limits>
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

// A triangle covering the screen, generated from gl_VertexID
const char *overlay_vert = R"(
#version 330 core
void main()
{
    vec2 p = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
})";

const char *overlay_frag = R"(
#version 330 core
uniform vec4 color;
out vec4 fragColor;
void main()
{
    fragColor = color;
})";

unsigned int compileOverlayProgram()
{
    auto compile = [](GLenum type, const char *source) {
        const GLuint shader = glCreateShader(type);
        const std::string code = ivf::prepareShaderSource(source); // GLSL ES on OpenGL ES
        const char *codePtr = code.c_str();
        glShaderSource(shader, 1, &codePtr, nullptr);
        glCompileShader(shader);
        GLint ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            glDeleteShader(shader);
            return 0u;
        }
        return shader;
    };
    const GLuint vert = compile(GL_VERTEX_SHADER, overlay_vert);
    const GLuint frag = compile(GL_FRAGMENT_SHADER, overlay_frag);
    GLuint program = 0;
    if (vert && frag)
    {
        program = glCreateProgram();
        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);
        GLint ok = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            glDeleteProgram(program);
            program = 0;
        }
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

std::string joined(const std::vector<std::string> &names)
{
    std::string result;
    for (const auto &name : names)
        result += (result.empty() ? "" : ", ") + name;
    return result;
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

    loadAutomation();

    // MIDI only with a bindings file: opening ports would take them from other programs
    m_liveEnabled = m_config.liveControls && m_config.exportDir.empty() && !m_config.controlsFile.empty();
    if (m_liveEnabled)
    {
        const std::string path = m_config.resolve(m_config.controlsFile);
        if (m_live.loadFile(path, &m_liveError))
            ivf::logInfo("Loaded live controls " + path + ": " + std::to_string(m_live.bindings().size()) +
                             " bindings",
                         "GLFWMusicSceneWindow");
        else
            ivf::logError(m_liveError, "GLFWMusicSceneWindow");
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

    // Pick up edits of the automation, take and controls files while developing
    if (!m_config.presentationMode && (m_reloadTimer += dt) >= 0.5)
    {
        m_reloadTimer = 0.0;
        bool changed = false;
        for (Automation *automation : {&m_baseAutomation, &m_take})
        {
            std::string error;
            if (automation->reloadIfChanged(&error))
            {
                changed = true;
                ivf::logInfo("Reloaded " + automation->path(), "GLFWMusicSceneWindow");
            }
            else if (!error.empty())
            {
                m_automationError = error;
                ivf::logError(error, "GLFWMusicSceneWindow");
            }
        }
        if (changed)
        {
            m_automationError.clear();
            m_automation = m_baseAutomation;
            m_automation.merge(m_take);
        }

        if (m_liveEnabled)
        {
            std::string error;
            if (m_live.reloadIfChanged(&error))
            {
                m_liveError.clear();
                ivf::logInfo("Reloaded live controls " + m_live.path(), "GLFWMusicSceneWindow");
            }
            else if (!error.empty())
            {
                m_liveError = error;
                ivf::logError(error, "GLFWMusicSceneWindow");
            }
        }
    }

    updateLive(dt);

    // Frozen: the picture holds while the music plays on
    if (m_frozen)
        return;

    ivf::ShaderMaterial::setGlobalUniform("time", float(m_transport.time()));
    ivf::ShaderMaterial::setGlobalUniform("beat", float(m_transport.beats()));
    ivf::ShaderMaterial::setGlobalUniform("beatPhase", float(m_transport.beatPhase()));

    onSceneUpdate(m_transport.time(), dt);
    updateCamera(dt);
}

void GLFWMusicSceneWindow::loadAutomation()
{
    std::string automationPath;
    if (!m_config.automationFile.empty())
    {
        automationPath = m_config.resolve(m_config.automationFile);
        if (m_baseAutomation.loadFile(automationPath, &m_automationError))
            ivf::logInfo("Loaded automation " + automationPath, "GLFWMusicSceneWindow");
        else
            ivf::logError(m_automationError, "GLFWMusicSceneWindow");
    }

    // Takes are saved next to the automation file, in the project when it is read from there
    if (!m_config.takeFile.empty())
        m_takePath = m_config.resolve(m_config.takeFile);
    else if (!automationPath.empty())
        m_takePath = std::filesystem::path(automationPath).replace_extension(".take.json").generic_string();
    else
        m_takePath = "take.json";

    std::error_code ec;
    if (std::filesystem::exists(m_takePath, ec))
    {
        std::string error;
        if (m_take.loadFile(m_takePath, &error))
            ivf::logInfo("Loaded take " + m_takePath, "GLFWMusicSceneWindow");
        else
            ivf::logError(error, "GLFWMusicSceneWindow");
    }

    m_automation = m_baseAutomation;
    m_automation.merge(m_take);
}

glm::vec4 GLFWMusicSceneWindow::paramValue(std::string_view name, glm::vec4 fallback, int components) const
{
    glm::vec4 value = m_automation.vec4(name, barPosition(), fallback);
    if (!m_liveEnabled)
        return value;
    value = m_live.apply(name, value, components);
    if (m_recorder.recording() && m_live.active(name))
        m_recorder.sample(std::string(name), value, components);
    return value;
}

double GLFWMusicSceneWindow::triggerAge(std::string_view name) const
{
    const double now = m_transport.time();
    double last = -std::numeric_limits<double>::infinity();
    if (auto hit = m_liveHits.find(name); hit != m_liveHits.end() && hit->second.time <= now)
        last = hit->second.time;
    if (const AutomationKey *key = m_automation.keyAtOrBefore(name, barPosition()))
        last = std::max(last, m_transport.timeAtBar(key->bar - 1.0));
    return std::isinf(last) ? std::numeric_limits<double>::infinity() : now - last;
}

float GLFWMusicSceneWindow::triggerVelocity(std::string_view name) const
{
    const double now = m_transport.time();
    double last = -std::numeric_limits<double>::infinity();
    float velocity = 0.0f;
    if (auto hit = m_liveHits.find(name); hit != m_liveHits.end() && hit->second.time <= now)
    {
        last = hit->second.time;
        velocity = hit->second.velocity;
    }
    if (const AutomationKey *key = m_automation.keyAtOrBefore(name, barPosition()))
        if (m_transport.timeAtBar(key->bar - 1.0) >= last)
            velocity = key->value.x;
    return velocity;
}

void GLFWMusicSceneWindow::updateLive(double dt)
{
    m_recorder.frame(barPosition(), m_transport.isPlaying());
    if (!m_liveEnabled)
        return;

    for (const LiveEvent &event : m_live.update(dt))
    {
        if (event.target == LiveBinding::Target::Trigger)
        {
            m_liveHits[event.name] = Hit{m_transport.time(), event.value};
            m_recorder.trigger(event.name, event.value);
            onLiveEvent(event);
        }
        else
        {
            handleAction(event);
        }
    }
}

void GLFWMusicSceneWindow::handleAction(const LiveEvent &event)
{
    // Commands that switch something on and off: a toggle sets the state, a button flips it
    auto switchState = [&](bool current) { return event.toggle ? event.pressed : !current; };
    const bool press = event.pressed || event.toggle;

    if (event.name == "freeze")
        m_frozen = event.pressed; // while held, or toggled
    else if (event.name == "play")
    {
        if (press && switchState(m_transport.isPlaying()) != m_transport.isPlaying())
            m_transport.togglePlay();
    }
    else if (event.name == "record")
    {
        if (press)
            setRecording(switchState(recording()));
    }
    else if (!event.pressed)
        return; // the remaining actions happen on press
    else if (event.name == "release")
        m_live.release();
    else if (event.name == "resetCamera")
        m_pathCamera->reset();
    else if (event.name == "nextMarker")
        seekToMarker(1);
    else if (event.name == "previousMarker")
        seekToMarker(-1);
    else if (event.name == "undoTake")
        undoTake();
    else
        onLiveEvent(event);
}

void GLFWMusicSceneWindow::setRecording(bool recording)
{
    if (recording == m_recorder.recording() || !m_config.exportDir.empty())
        return;

    if (recording)
    {
        m_recorder.start();
        ivf::logInfo("Recording a take", "GLFWMusicSceneWindow");
        return;
    }

    m_recorder.stop();
    if (!m_recorder.hasData())
    {
        ivf::logInfo("Recording stopped; no live control was used while the song played", "GLFWMusicSceneWindow");
        return;
    }

    Automation before = m_take;
    const std::vector<std::string> changed = m_recorder.commit(m_take);
    std::string error;
    if (!m_take.saveFile(m_takePath, &error))
    {
        ivf::logError("Take not saved: " + error, "GLFWMusicSceneWindow");
        m_take = std::move(before);
        return;
    }
    m_takeUndo = std::move(before);
    m_hasTakeUndo = true;
    m_automation = m_baseAutomation;
    m_automation.merge(m_take);

    // The take now plays the performance; live values on top would apply it twice
    for (const auto &name : changed)
        m_live.release(name);
    ivf::logInfo("Recorded " + joined(changed) + " into " + m_takePath, "GLFWMusicSceneWindow");
}

void GLFWMusicSceneWindow::undoTake()
{
    if (!m_hasTakeUndo)
        return;
    std::string error;
    Automation current = m_take;
    m_take = m_takeUndo;
    if (!m_take.saveFile(m_takePath, &error))
    {
        ivf::logError("Take not restored: " + error, "GLFWMusicSceneWindow");
        m_take = std::move(current);
        return;
    }
    m_hasTakeUndo = false;
    m_automation = m_baseAutomation;
    m_automation.merge(m_take);
    ivf::logInfo("Restored " + m_takePath + " as it was before the last recording", "GLFWMusicSceneWindow");
}

void GLFWMusicSceneWindow::doDraw()
{
    GLFWSceneWindow::doDraw();
    drawOverlay();
}

void GLFWMusicSceneWindow::drawOverlay()
{
    const float blackout = std::clamp(param("blackout"), 0.0f, 1.0f);
    float flash = 0.0f;
    if (const float strobe = std::clamp(param("strobe"), 0.0f, 1.0f); strobe > 0.0f)
    {
        const double rate = std::max(double(param("strobeRate", 0.25f)), 1.0 / 64.0);
        const double phase = std::fmod(m_transport.beats() / rate, 1.0);
        flash = (phase >= 0.0 && phase < 0.35) ? strobe : 0.0f;
    }
    if (blackout <= 0.0f && flash <= 0.0f)
        return;

    if (!m_overlayProgram)
    {
        m_overlayProgram = compileOverlayProgram();
        if (!m_overlayProgram)
        {
            ivf::logError("Overlay shader failed to compile", "GLFWMusicSceneWindow");
            return;
        }
        glGenVertexArrays(1, &m_overlayVao);
    }

    GLint program = 0, vao = 0, srcRgb = 0, dstRgb = 0, srcAlpha = 0, dstAlpha = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    glGetIntegerv(GL_BLEND_SRC_RGB, &srcRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &dstRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &dstAlpha);
    const GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blend = glIsEnabled(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(m_overlayProgram);
    glBindVertexArray(m_overlayVao);
    const GLint color = glGetUniformLocation(m_overlayProgram, "color");
    if (flash > 0.0f)
    {
        glUniform4f(color, 1.0f, 1.0f, 1.0f, flash);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    if (blackout > 0.0f)
    {
        glUniform4f(color, 0.0f, 0.0f, 0.0f, blackout);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glBindVertexArray(GLuint(vao));
    glUseProgram(GLuint(program));
    glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
    blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
    depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
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
    if (m_showLivePanel)
        drawLivePanel();
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
        for (const auto &name : m_automation.names())
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
    if (!m_liveError.empty())
    {
        ImGui::SameLine();
        ImGui::TextColored({1.0f, 0.4f, 0.3f, 1.0f}, "  controls error (F5)");
    }
    if (recording())
    {
        ImGui::SameLine();
        ImGui::TextColored({1.0f, 0.25f, 0.2f, 1.0f}, "  REC");
    }
    if (m_frozen)
    {
        ImGui::SameLine();
        ImGui::TextColored({0.5f, 0.8f, 1.0f, 1.0f}, "  FROZEN");
    }
    if (m_liveEnabled)
    {
        if (const size_t active = m_live.activeParams().size(); active > 0)
        {
            ImGui::SameLine();
            ImGui::TextColored({1.0f, 0.8f, 0.3f, 1.0f}, "  live: %zu", active);
        }
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
    case GLFW_KEY_F5:
        if (action == GLFW_PRESS && !m_config.presentationMode)
            m_showLivePanel = !m_showLivePanel;
        break;
    case GLFW_KEY_F6:
        if (action == GLFW_PRESS)
            setRecording(!recording());
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

void GLFWMusicSceneWindow::drawLivePanel()
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({viewport->WorkPos.x + 10.0f, viewport->WorkPos.y + 30.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({380.0f, 520.0f}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Live", &m_showLivePanel);

    if (!m_liveEnabled)
    {
        ImGui::TextWrapped(m_config.controlsFile.empty() ? "No controls file: start with --controls FILE"
                                                         : "Live control is off (--no-live)");
    }
    else
    {
        ImGui::TextWrapped("%s", m_live.path().c_str());
        if (!m_liveError.empty())
            ImGui::TextColored({1.0f, 0.4f, 0.3f, 1.0f}, "%s", m_liveError.c_str());
    }

    // Recording works with every live control, so it is shown first
    if (recording())
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.15f, 0.12f, 1.0f));
        if (ImGui::Button("Stop recording (F6)"))
            setRecording(false);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("%zu samples", m_recorder.sampleCount());
    }
    else if (ImGui::Button("Record take (F6)"))
    {
        setRecording(true);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_hasTakeUndo);
    if (ImGui::Button("Undo take"))
        undoTake();
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Release all"))
        m_live.release();
    ImGui::TextDisabled("Takes: %s", m_takePath.c_str());

    if (!m_liveEnabled)
    {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Devices", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (const auto &device : m_live.devices())
        {
            if (device.portName.empty())
                ImGui::TextColored({1.0f, 0.6f, 0.3f, 1.0f}, "%s: \"%s\" not connected", device.alias.c_str(),
                                   device.match.c_str());
            else
                ImGui::Text("%s: %s", device.alias.c_str(), device.portName.c_str());
        }
        if (ImGui::TreeNode("Available ports"))
        {
            for (const auto &port : m_live.ports())
                ImGui::BulletText("%s", port.c_str());
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Bindings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const auto &bindings = m_live.bindings();
        for (size_t i = 0; i < bindings.size(); ++i)
        {
            const LiveBinding &binding = bindings[i];
            char source[48];
            std::snprintf(source, sizeof(source), "%s %s %d", binding.device.c_str(), binding.cc >= 0 ? "cc" : "note",
                          binding.cc >= 0 ? binding.cc : binding.note);
            if (binding.target == LiveBinding::Target::Param)
            {
                const float range = binding.max - binding.min;
                const float fraction =
                    std::abs(range) > 1e-6f ? (m_live.bindingValue(i) - binding.min) / range : 0.0f;
                char value[32];
                std::snprintf(value, sizeof(value), "%.3f", m_live.bindingValue(i));
                ImGui::BeginDisabled(!m_live.bindingActive(i));
                ImGui::ProgressBar(std::clamp(fraction, 0.0f, 1.0f), {110.0f, 0.0f}, value);
                ImGui::EndDisabled();
                ImGui::SameLine();
                const char *mode = binding.mode == LiveBinding::Mode::Override ? ""
                                   : binding.mode == LiveBinding::Mode::Offset ? " (+)"
                                                                               : " (x)";
                ImGui::Text("%s%s  [%s]", binding.name.c_str(), mode, source);
            }
            else
            {
                ImGui::TextDisabled("%-8s", binding.target == LiveBinding::Target::Trigger ? "trigger" : "action");
                ImGui::SameLine();
                ImGui::Text("%s  [%s]", binding.name.c_str(), source);
            }
        }
    }

    // A MIDI monitor, to find the numbers a controller sends
    if (ImGui::CollapsingHeader("MIDI monitor", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const auto &recent = m_live.recentMessages();
        int shown = 0;
        for (auto it = recent.rbegin(); it != recent.rend() && shown < 12; ++it, ++shown)
        {
            const ivf::MidiMessage &m = it->message;
            const char *type = m.type == ivf::MidiMessage::ControlChange ? "cc"
                               : m.type == ivf::MidiMessage::NoteOn      ? "note on"
                               : m.type == ivf::MidiMessage::NoteOff     ? "note off"
                               : m.type == ivf::MidiMessage::PitchBend   ? "pitch bend"
                                                                         : "other";
            ImGui::Text("%-12s ch %2d  %-10s %3d  %3d", it->device.c_str(), m.channel + 1, type, m.data1, m.data2);
        }
        if (recent.empty())
            ImGui::TextDisabled("Move a control to see its messages");
    }
    ImGui::End();
}
