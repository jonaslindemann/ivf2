/**
 * @file skybox1.cpp
 * @brief Skybox / cubemap example.
 * @ingroup texture_examples
 *
 * Demonstrates the Skybox and Cubemap classes:
 *  - Procedural gradient sky generated entirely in code (no asset files needed)
 *  - How to swap in a real cubemap from disk with loadFromDirectory()
 *  - Live sky color adjustment via ImGui color pickers
 *  - Exposure control
 *
 * To use real skybox images, place them in a directory named "skybox/" inside
 * the assets folder, with files named right, left, top, bottom, front, back
 * (jpg/png/hdr).  Then call cm->loadFromDirectory("assets/skybox") instead
 * of cm->loadGradient().
 *
 * The Skybox node must be added to the scene BEFORE other geometry so it
 * draws first (depth-write is disabled during skybox rendering).
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <array>
#include <string>

using namespace ivf;
using namespace ivfui;

class SkyboxWindow : public GLFWSceneWindow {
private:
    CubemapPtr m_cubemap;
    SkyboxPtr  m_skybox;

    // Color controls (stored as float[3] for ImGui)
    std::array<float, 3> m_topColor     {0.05f, 0.15f, 0.45f};
    std::array<float, 3> m_horizonColor {0.45f, 0.65f, 0.85f};
    std::array<float, 3> m_bottomColor  {0.18f, 0.12f, 0.08f};
    float m_exposure{1.0f};
    float m_envReflectivity{0.3f};
    bool m_needsRegen{false};

public:
    SkyboxWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<SkyboxWindow> create(int w, int h, std::string title)
    { return std::make_shared<SkyboxWindow>(w, h, title); }

    int onSetup() override
    {
        // --- Procedural gradient cubemap -----------------------------------
        m_cubemap = Cubemap::create();
        m_cubemap->loadFromDirectory("assets/skybox");
        /*
        m_cubemap->loadGradient(
            {m_topColor[0],     m_topColor[1],     m_topColor[2]},
            {m_horizonColor[0], m_horizonColor[1], m_horizonColor[2]},
            {m_bottomColor[0],  m_bottomColor[1],  m_bottomColor[2]},
            128);
        */

        m_skybox = Skybox::create(m_cubemap);
        m_skybox->setExposure(m_exposure);

        // Skybox must be added FIRST so it renders behind everything
        add(m_skybox);

        // --- Scene geometry -------------------------------------------------
        auto setupMat = [](glm::vec3 diff, float shine = 32.0f) {
            auto m = Material::create();
            m->setDiffuseColor({diff, 1.0f});
            m->setAmbientColor({diff * 0.1f, 1.0f});
            m->setShininess(shine);
            return m;
        };

        // Central sphere
        auto sphere = Sphere::create(1.2f, 32, 32);
        sphere->setMaterial(setupMat({0.8f, 0.8f, 0.85f}, 128.0f));
        add(sphere);

        // Surrounding cubes
        const int N = 8;
        for (int i = 0; i < N; ++i) {
            float a  = float(i) / float(N) * glm::two_pi<float>();
            float r  = 4.0f;
            float hue = float(i) / float(N);
            glm::vec3 col = hsvToRgb(hue * 360.0f, 0.7f, 0.85f);

            auto cube = Cube::create();
            cube->setSize(0.8f);
            cube->setPos({std::cos(a) * r, 0, std::sin(a) * r});
            cube->setMaterial(setupMat(col));
            add(cube);
        }

        // Ground disc
        auto disc = Disk::create(6.0f, 0.0f, 64);
        disc->setPos({0, -1.5f, 0});
        disc->setMaterial(setupMat({0.3f, 0.28f, 0.26f}, 8.0f));
        add(disc);

        // Light
        enableHeadlight();

        cameraManipulator()->setCameraPosition({0, 3, 12});
        return 0;
    }

    void regenSky()
    {
        m_cubemap->loadGradient(
            {m_topColor[0],     m_topColor[1],     m_topColor[2]},
            {m_horizonColor[0], m_horizonColor[1], m_horizonColor[2]},
            {m_bottomColor[0],  m_bottomColor[1],  m_bottomColor[2]},
            128);
        m_skybox->setExposure(m_exposure);
        m_needsRegen = false;
    }

    void onUpdate() override
    {
        if (m_needsRegen) regenSky();
    }

    void onDraw() override
    {
        auto prog = ShaderManager::instance()->program("basic");
        if (prog && m_cubemap)
        {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap->id());
            glActiveTexture(GL_TEXTURE0);
            prog->uniformBool("useEnvMap", true);
            prog->uniformFloat("envReflectivity", m_envReflectivity);
            prog->uniformInt("envMap", 5);
        }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({300, 280}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({16, 16}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Skybox Settings");

        ImGui::SeparatorText("Gradient colors");
        if (ImGui::ColorEdit3("Sky top",     m_topColor.data()))     m_needsRegen = true;
        if (ImGui::ColorEdit3("Horizon",     m_horizonColor.data())) m_needsRegen = true;
        if (ImGui::ColorEdit3("Ground",      m_bottomColor.data()))  m_needsRegen = true;

        ImGui::Spacing();
        ImGui::SeparatorText("Exposure");
        if (ImGui::SliderFloat("##expo", &m_exposure, 0.1f, 3.0f, "%.2f"))
            m_needsRegen = true;

        ImGui::Spacing();
        ImGui::SeparatorText("Reflection");
        ImGui::SliderFloat("Reflectivity", &m_envReflectivity, 0.0f, 1.0f, "%.2f");

        ImGui::Spacing();
        ImGui::SeparatorText("Presets");
        if (ImGui::Button("Sunset")) {
            m_topColor     = {0.04f, 0.06f, 0.22f};
            m_horizonColor = {0.85f, 0.38f, 0.12f};
            m_bottomColor  = {0.12f, 0.08f, 0.06f};
            m_needsRegen   = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Overcast")) {
            m_topColor     = {0.40f, 0.42f, 0.46f};
            m_horizonColor = {0.62f, 0.64f, 0.67f};
            m_bottomColor  = {0.22f, 0.20f, 0.18f};
            m_needsRegen   = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Night")) {
            m_topColor     = {0.01f, 0.01f, 0.06f};
            m_horizonColor = {0.04f, 0.04f, 0.12f};
            m_bottomColor  = {0.02f, 0.02f, 0.04f};
            m_exposure     = 2.0f;
            m_needsRegen   = true;
        }

        ImGui::Spacing();
        ImGui::SeparatorText("From files");
        ImGui::TextWrapped(
            "Place skybox images in assets/skybox/ named:\n"
            "  right, left, top, bottom, front, back\n"
            "  (.jpg / .png / .hdr)\n"
            "Then call:\n"
            "  cm->loadFromDirectory(\"assets/skybox\");");

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

    auto win = SkyboxWindow::create(1280, 800, "Skybox");
    win->maximize();
    app->addWindow(win);
    return app->loop();
}
