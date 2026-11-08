/**
 * @file shader_material1.cpp
 * @brief Custom shaders on nodes with ShaderMaterial.
 * @ingroup general_examples
 *
 * Demonstrates ShaderMaterial:
 *  - A glowing, breathing sphere and a rippling plane, each with its own shader
 *  - A stock-lit cube rendered alongside them with the default shader
 *  - A global "time" uniform shared by all shader materials
 *  - Per-material uniforms driven from ImGui
 *  - Hot reload: edit shaders/glow.* or shaders/ripple.* next to the executable
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

class ShaderMaterialWindow : public GLFWSceneWindow {
private:
    ShaderMaterialPtr m_glow;
    ShaderMaterialPtr m_ripple;
    glm::vec3 m_glowColor{0.3f, 1.0f, 0.8f};
    glm::vec3 m_rippleColor{0.6f, 0.3f, 1.0f};
    float m_pulse{1.0f};
    float m_amplitude{0.3f};

public:
    ShaderMaterialWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<ShaderMaterialWindow> create(int w, int h, std::string title)
    { return std::make_shared<ShaderMaterialWindow>(w, h, title); }

    int onSetup() override
    {
        m_glow = ShaderMaterial::createFromFiles("shaders/glow.vert", "shaders/glow.frag", "glow", true);
        m_ripple = ShaderMaterial::createFromFiles("shaders/ripple.vert", "shaders/ripple.frag", "ripple", true);
        if (!m_glow || !m_ripple)
            return -1;

        auto sphere = Sphere::create(1.0, 64, 32);
        sphere->setPos({0.0f, 1.5f, 0.0f});
        sphere->setMaterial(m_glow);
        add(sphere);

        auto ground = Plane::create(20.0, 20.0, 128, 128);
        ground->setMaterial(m_ripple);
        add(ground);

        auto cubeMaterial = Material::create();
        cubeMaterial->setDiffuseColor({0.8f, 0.5f, 0.2f, 1.0f});
        auto cube = Cube::create();
        cube->setPos({3.0f, 0.75f, 0.0f});
        cube->setMaterial(cubeMaterial);
        add(cube);

        cameraManipulator()->setCameraPosition({0.0f, 4.0f, 9.0f});
        return 0;
    }

    void onUpdate() override
    {
        ShaderMaterial::setGlobalUniform("time", float(elapsedTime()));

        m_glow->setUniform("glowColor", m_glowColor);
        m_glow->setUniform("pulse", m_pulse);
        m_ripple->setUniform("glowColor", m_rippleColor);
        m_ripple->setUniform("amplitude", m_amplitude);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({320, 170}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Shader Material");
        ImGui::ColorEdit3("Glow color", &m_glowColor.x);
        ImGui::SliderFloat("Pulse", &m_pulse, 0.0f, 3.0f);
        ImGui::ColorEdit3("Ripple color", &m_rippleColor.x);
        ImGui::SliderFloat("Ripple amplitude", &m_amplitude, 0.0f, 1.0f);
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto win = ShaderMaterialWindow::create(1280, 720, "Shader Material");
    app->addWindow(win);
    return app->loop();
}
