/**
 * @file time_control1.cpp
 * @brief TimeController example — global pause, slow-motion, and time scaling.
 *
 * Demonstrates ivf::TimeController: a single control point for pausing and
 * scaling time across animations, deformers, and effects.
 *
 * Controls:
 *   Space       — pause / resume
 *   +  / -      — increase / decrease time scale (0.1 steps)
 *   R           — reset elapsed time to 0
 *   ESC         — quit
 */

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/deformable_mesh_node.h>
#include <ivf/deformable_primitive.h>
#include <ivf/wave_deformer.h>
#include <ivfui/ui.h>
#include <ivfui/time_control_panel.h>

#include <imgui.h>
#include <numbers>
#include <vector>

using namespace ivf;
using namespace ivfui;

class ExampleWindow : public GLFWSceneWindow {
private:
    std::shared_ptr<DeformablePrimitive<Plane>> m_ocean;
    std::shared_ptr<WaveDeformer> m_wave1;
    std::shared_ptr<WaveDeformer> m_wave2;

    std::vector<BoxPtr> m_cubes;
    static constexpr int k_cubeCount = 9;

    TimeControlPanelPtr m_panel;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title) {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup() override
    {
        cameraManipulator()->setCameraPosition(glm::vec3(0.0f, 8.0f, 18.0f));
        cameraManipulator()->setCameraTarget(glm::vec3(0.0f, 0.0f, 0.0f));

        enableHeadlight();

        // Wave-deformed plane
        m_ocean = DeformablePrimitive<Plane>::create();
        m_ocean->primitive()->set(16.0f, 16.0f, 80, 80);
        m_ocean->refresh();

        auto mat = Material::create();
        mat->setDiffuseColor(glm::vec4(0.1f, 0.4f, 0.8f, 1.0f));
        mat->setSpecularColor(glm::vec4(0.8f, 0.8f, 1.0f, 1.0f));
        mat->setShininess(64.0f);
        m_ocean->setMaterial(mat);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_wave1 = WaveDeformer::create(0.3f, 0.6f, 1.1f, glm::normalize(glm::vec3(1.0f, 0.0f, 0.3f)), up);
        m_wave2 = WaveDeformer::create(0.15f, 1.0f, 0.8f, glm::normalize(glm::vec3(-0.6f, 0.0f, 1.0f)), up);
        m_ocean->addDeformer(m_wave1);
        m_ocean->addDeformer(m_wave2);
        add(m_ocean);

        // Orbiting cubes (Box inherits TransformNode — setPos/setRotAxis/setRotAngle available)
        for (int i = 0; i < k_cubeCount; ++i) {
            auto cube = Box::create();
            cube->setSize(glm::vec3(0.6f, 0.6f, 0.6f));

            auto cm = Material::create();
            float hue = static_cast<float>(i) / k_cubeCount;
            cm->setDiffuseColor(glm::vec4(hsvToRgb(hue * 360.0f, 0.8f, 0.9f), 1.0f));
            cube->setMaterial(cm);

            m_cubes.push_back(cube);
            add(cube);
        }

        m_panel = TimeControlPanel::create();
        addUiWindow(m_panel);

        TimeController::instance()->setScale(1.0f);
        return 0;
    }

    void onUpdate() override
    {
        double t = TimeController::instance()->elapsed();

        m_wave1->setTime(static_cast<float>(t));
        m_wave2->setTime(static_cast<float>(t));
        m_ocean->applyDeformers();

        for (int i = 0; i < k_cubeCount; ++i) {
            float angle  = static_cast<float>(t) * 0.8f + i * (2.0f * std::numbers::pi_v<float> / k_cubeCount);
            float radius = 5.5f;
            float y      = 1.5f + 0.6f * std::sin(static_cast<float>(t) * 1.3f + i);
            m_cubes[i]->setPos(glm::vec3(radius * std::cos(angle), y, radius * std::sin(angle)));
            m_cubes[i]->setRotAxis(glm::vec3(0.3f, 1.0f, 0.2f));
            m_cubes[i]->setRotAngle(static_cast<float>(t) * 60.0f + i * 40.0f);
        }
    }

    void onKey(int key, int /*scancode*/, int action, int /*mods*/) override
    {
        if (action != GLFW_PRESS)
            return;
        auto *tc = TimeController::instance();
        if (key == GLFW_KEY_SPACE)
            tc->togglePause();
        else if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
            tc->setScale(std::min(4.0f, tc->scale() + 0.1f));
        else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
            tc->setScale(std::max(0.0f, tc->scale() - 0.1f));
        else if (key == GLFW_KEY_R)
            tc->reset();
    }

    void onDrawUi() override
    {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, io.DisplaySize.y - 10.0f),
                                ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.4f);
        ImGui::Begin("##hint", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav);
        ImGui::TextDisabled("Space=pause  +/-=scale  R=reset");
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_SAMPLES, 4);
    auto win = ExampleWindow::create(1280, 720, "TimeController example");
    app->addWindow(win);
    return app->loop();
}
