/**
 * @file frame_export1.cpp
 * @brief Deterministic PNG sequence export for rendering video.
 * @ingroup general_examples
 *
 * Demonstrates GLFWWindow::startFrameExport():
 *  - Fixed time step: every frame advances exactly 1/fps, so animation driven by
 *    elapsedTime(), frameTime(), tweens and shader time is identical between runs
 *  - onFrameExportStarted() resets animation state for frame 0
 *  - Frames are captured without the UI and written on background threads
 *
 * Interactive: E exports 4 seconds starting at t = 0, S saves a screenshot.
 * Batch:       frame_export1 --export <dir> [--fps N] [--frames N]
 *              renders the frames and exits.
 *
 * Make a video with:
 *   ffmpeg -framerate 30 -i <dir>/frame_%06d.png -c:v libx264 -pix_fmt yuv420p -crf 16 out.mp4
 */

#include <imgui.h>
#include <ivf/fog_effect.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/tween.h>
#include <ivfui/ui.h>

#include <cmath>
#include <cstdlib>
#include <string>

using namespace ivf;
using namespace ivfui;

struct ExportOptions {
    std::string directory;
    int fps{30};
    int frames{120};
};

class ExportWindow : public GLFWSceneWindow {
private:
    ExportOptions m_batch;
    std::vector<TransformNodePtr> m_orbs;
    TransformNodePtr m_center;
    float m_centerScale{1.0f};
    std::string m_status;

    void pulse(float to)
    {
        // Ping-pong tween driven by frameTime()
        TweenManager::instance()->to(m_centerScale, to, 0.75f, Easing::SineInOut)
            ->setOnComplete([this, to] { pulse(to > 1.0f ? 1.0f : 1.6f); });
    }

    void exportFrames(const std::string &directory, int fps, int frames, bool closeWhenDone)
    {
        FrameExportSettings settings;
        settings.directory = directory;
        settings.fps = fps;
        settings.frameCount = frames;
        settings.startTime = 0.0;
        settings.closeWhenDone = closeWhenDone;
        startFrameExport(settings);
    }

public:
    ExportWindow(int w, int h, std::string title, ExportOptions batch)
        : GLFWSceneWindow(w, h, title), m_batch(std::move(batch))
    {}

    static std::shared_ptr<ExportWindow> create(int w, int h, std::string title, ExportOptions batch)
    { return std::make_shared<ExportWindow>(w, h, title, std::move(batch)); }

    int onSetup() override
    {
        auto light = LightManager::instance()->addDirectionalLight();
        light->setDiffuseColor({0.9f, 0.85f, 0.8f});
        light->setDirection({-0.4f, -1.0f, -0.3f});
        light->setEnabled(true);

        auto glow = ShaderMaterial::createFromFiles("shaders/glow.vert", "shaders/glow.frag", "glow");
        if (!glow)
            return -1;
        glow->setUniform("glowColor", glm::vec3(0.3f, 1.0f, 0.8f));
        glow->setUniform("pulse", 1.0f);

        auto center = Sphere::create(1.0, 48, 24);
        center->setMaterial(glow);
        add(center);
        m_center = center;

        auto orbMaterial = Material::create();
        orbMaterial->setDiffuseColor({0.9f, 0.5f, 0.3f, 1.0f});
        for (int i = 0; i < 8; ++i)
        {
            auto orb = Cube::create();
            orb->setMaterial(orbMaterial);
            add(orb);
            m_orbs.push_back(orb);
        }

        auto fog = FogEffect::create();
        fog->setColor({0.03f, 0.03f, 0.07f});
        fog->setDensity(0.04f);
        fog->load();
        addEffect(fog);
        setRenderToTexture(true);

        setClearColor({0.03f, 0.03f, 0.07f, 1.0f});
        cameraManipulator()->setCameraPosition({0.0f, 4.0f, 12.0f});

        pulse(1.6f);

        if (!m_batch.directory.empty())
            exportFrames(m_batch.directory, m_batch.fps, m_batch.frames, true);
        return 0;
    }

    void onFrameExportStarted() override
    {
        // Same starting state for every export
        TweenManager::instance()->clear();
        m_centerScale = 1.0f;
        pulse(1.6f);
    }

    void onFrameExportFinished() override
    {
        m_status = "Exported " + std::to_string(recordedFrameCount()) + " frames";
    }

    void onUpdate() override
    {
        float t = float(elapsedTime());
        ShaderMaterial::setGlobalUniform("time", t);

        m_center->setScale(glm::vec3(m_centerScale));
        for (size_t i = 0; i < m_orbs.size(); ++i)
        {
            float a = 6.2831853f * i / m_orbs.size() + t * 0.8f;
            m_orbs[i]->setPos({4.0f * std::cos(a), std::sin(t * 2.0f + i), 4.0f * std::sin(a)});
            m_orbs[i]->setEulerAngles(t * 90.0f, t * 60.0f + i * 20.0f, 0.0f);
        }
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        GLFWSceneWindow::onKey(key, scancode, action, mods);
        if (action != GLFW_PRESS || isExporting())
            return;
        if (key == GLFW_KEY_E)
        {
            m_status = "Exporting...";
            exportFrames("export", 30, 120, false);
        }
        else if (key == GLFW_KEY_S)
        {
            saveScreenshot("screenshot.png");
            m_status = "Saved screenshot.png";
        }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({300, 110}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Frame Export");
        ImGui::Text("E: export 4 s (30 fps), S: screenshot");
        ImGui::Text("Time: %.3f s", elapsedTime());
        if (isExporting())
            ImGui::Text("Exporting frame %d", recordedFrameCount());
        else
            ImGui::TextUnformatted(m_status.c_str());
        ImGui::End();
    }
};

int main(int argc, char **argv)
{
    ExportOptions batch;
    for (int i = 1; i + 1 < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--export")
            batch.directory = argv[++i];
        else if (arg == "--fps")
            batch.fps = std::atoi(argv[++i]);
        else if (arg == "--frames")
            batch.frames = std::atoi(argv[++i]);
    }

    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto win = ExportWindow::create(1280, 720, "Frame Export", batch);
    app->addWindow(win);
    return app->loop();
}
