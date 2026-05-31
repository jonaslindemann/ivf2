/**
 * @file camera_anim1.cpp
 * @brief CameraAnimation example — spline fly-through with shake.
 *
 * Demonstrates CameraAnimation: a spline-based fly-through over a scene,
 * with auto-look-forward, looping, and a camera shake effect.
 *
 * Controls:
 *   P        — play / stop animation
 *   S        — trigger camera shake
 *   L        — toggle loop / once mode
 *   F        — toggle auto-look-forward vs explicit target spline
 *   ESC      — quit
 */

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfui/camera_animation.h>

#include <imgui.h>
#include <numbers>
#include <vector>

using namespace ivf;
using namespace ivfui;

class ExampleWindow : public GLFWSceneWindow {
private:
    CameraAnimationPtr m_camAnim;
    bool m_looping{true};
    bool m_autoLook{true};

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title) {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup() override
    {
        enableHeadlight();

        // Build a scene: grid of coloured cubes
        auto lightMgr = LightManager::instance();
        auto light = lightMgr->addPointLight();
        light->setEnabled(true);
        light->setDiffuseColor({1.0f, 0.95f, 0.85f});
        light->setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        light->setAttenuation(1.0f, 0.0f, 0.0f);

        constexpr int kGridSize = 5;
        for (int ix = -kGridSize; ix <= kGridSize; ++ix) {
            for (int iz = -kGridSize; iz <= kGridSize; ++iz) {
                float h = static_cast<float>(ix + kGridSize) / (2.0f * kGridSize);
                auto cube = Box::create();
                float height = 0.5f + 2.5f * std::abs(std::sin(ix * 0.7f) * std::cos(iz * 0.5f));
                cube->setSize(glm::vec3(0.8f, height, 0.8f));
                cube->setPos(glm::vec3(static_cast<float>(ix) * 2.5f, height * 0.5f, static_cast<float>(iz) * 2.5f));
                auto mat = Material::create();
                mat->setDiffuseColor(glm::vec4(hsvToRgb(h * 280.0f, 0.7f, 0.85f), 1.0f));
                mat->setShininess(32.0f);
                cube->setMaterial(mat);
                add(cube);
            }
        }

        // Camera animation: fly around and over the scene
        m_camAnim = CameraAnimation::create(cameraManipulator());
        m_camAnim->setMode(SplineAnimMode::Loop);
        m_camAnim->setDuration(12.0f);
        m_camAnim->setAutoLookForward(true);
        m_camAnim->setBlockManipulatorDuringPlayback(true);

        // Position path: wide orbit with altitude variation
        m_camAnim->addKeyframe(glm::vec3(18.0f,  8.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->addKeyframe(glm::vec3( 8.0f, 12.0f, 14.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->addKeyframe(glm::vec3(-5.0f,  5.0f, 18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->addKeyframe(glm::vec3(-18.0f, 9.0f,  3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->addKeyframe(glm::vec3(-10.0f, 6.0f,-14.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->addKeyframe(glm::vec3(  3.0f,14.0f,-16.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->addKeyframe(glm::vec3( 14.0f, 7.0f, -8.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->addKeyframe(glm::vec3( 18.0f, 8.0f,  0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_camAnim->play();

        return 0;
    }

    void onUpdate() override
    {
        m_camAnim->update(TimeController::instance()->delta());
    }

    void onKey(int key, int /*scancode*/, int action, int /*mods*/) override
    {
        if (action != GLFW_PRESS)
            return;
        if (key == GLFW_KEY_P) {
            if (m_camAnim->isPlaying())
                m_camAnim->stop();
            else
                m_camAnim->play();
        } else if (key == GLFW_KEY_S) {
            m_camAnim->shake(0.4f, 0.5f);
        } else if (key == GLFW_KEY_L) {
            m_looping = !m_looping;
            m_camAnim->setMode(m_looping ? SplineAnimMode::Loop : SplineAnimMode::Once);
        } else if (key == GLFW_KEY_F) {
            m_autoLook = !m_autoLook;
            m_camAnim->setAutoLookForward(m_autoLook);
        }
    }

    void onDrawUi() override
    {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(10.0f, io.DisplaySize.y - 10.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.45f);
        ImGui::Begin("##controls", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav);
        ImGui::TextDisabled("P=play/stop  S=shake  L=loop  F=auto-look");
        ImGui::Separator();
        ImGui::Text("Playing  : %s", m_camAnim->isPlaying() ? "yes" : "no");
        ImGui::Text("Progress : %.1f %%", m_camAnim->progress() * 100.0f);
        ImGui::Text("Mode     : %s", m_looping ? "Loop" : "Once");
        ImGui::Text("Look     : %s", m_autoLook ? "Auto-forward" : "Target spline");
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_SAMPLES, 4);
    auto win = ExampleWindow::create(1280, 720, "Camera Animation example");
    app->addWindow(win);
    return app->loop();
}
