/**
 * @file fog1.cpp
 * @brief Depth-based distance and height fog with FogEffect.
 * @ingroup effect_examples
 *
 * Demonstrates FogEffect:
 *  - Distance fog reconstructed from the scene depth buffer
 *  - Height fog (ground mist) that thins out above a base height
 *  - Fog on the background via the sky amount
 *  - All parameters adjustable from ImGui
 */

#include <imgui.h>
#include <ivf/fog_effect.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <cmath>

using namespace ivf;
using namespace ivfui;

class FogWindow : public GLFWSceneWindow {
private:
    FogEffectPtr m_fog;
    glm::vec3 m_fogColor{0.08f, 0.10f, 0.18f};
    float m_density{0.06f};
    float m_start{2.0f};
    float m_heightFalloff{0.0f};
    float m_baseHeight{0.0f};
    float m_maxOpacity{1.0f};
    float m_skyAmount{1.0f};
    bool m_enabled{true};

public:
    FogWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<FogWindow> create(int w, int h, std::string title)
    { return std::make_shared<FogWindow>(w, h, title); }

    int onSetup() override
    {
        auto light = LightManager::instance()->addDirectionalLight();
        light->setDiffuseColor({0.9f, 0.85f, 0.8f});
        light->setDirection({-0.4f, -1.0f, -0.3f});
        light->setEnabled(true);

        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.25f, 0.3f, 0.25f, 1.0f});
        auto ground = Plane::create(200.0, 200.0, 4, 4);
        ground->setMaterial(groundMaterial);
        add(ground);

        // Rows of pillars and spheres receding into the distance
        auto pillarMaterial = Material::create();
        pillarMaterial->setDiffuseColor({0.8f, 0.6f, 0.4f, 1.0f});
        auto sphereMaterial = Material::create();
        sphereMaterial->setDiffuseColor({0.4f, 0.7f, 0.9f, 1.0f});

        for (int row = 0; row < 12; ++row)
        {
            float z = -row * 6.0f;
            for (int side = -1; side <= 1; side += 2)
            {
                float height = 2.0f + 1.5f * std::sin(row * 0.9f);
                auto pillar = Box::create({0.8f, height, 0.8f});
                pillar->setPos({side * 4.0f, height * 0.5f, z});
                pillar->setMaterial(pillarMaterial);
                add(pillar);

                auto sphere = Sphere::create(0.6);
                sphere->setPos({side * 1.5f, 0.6f + (row % 3) * 1.2f, z - 3.0f});
                sphere->setMaterial(sphereMaterial);
                add(sphere);
            }
        }

        m_fog = FogEffect::create();
        m_fog->load();
        addEffect(m_fog);
        setRenderToTexture(true);

        setClearColor({0.0f, 0.0f, 0.0f, 1.0f});
        cameraManipulator()->setCameraPosition({0.0f, 2.5f, 10.0f});
        return 0;
    }

    void onUpdate() override
    {
        m_fog->setColor(m_fogColor);
        m_fog->setDensity(m_density);
        m_fog->setStart(m_start);
        m_fog->setHeightFalloff(m_heightFalloff);
        m_fog->setBaseHeight(m_baseHeight);
        m_fog->setMaxOpacity(m_maxOpacity);
        m_fog->setSkyAmount(m_skyAmount);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({320, 250}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Fog");
        if (ImGui::Checkbox("Enabled", &m_enabled))
            m_enabled ? enableEffect(0) : disableEffect(0);
        ImGui::ColorEdit3("Color", &m_fogColor.x);
        ImGui::SliderFloat("Density", &m_density, 0.0f, 0.3f);
        ImGui::SliderFloat("Start", &m_start, 0.0f, 30.0f);
        ImGui::SliderFloat("Height falloff", &m_heightFalloff, 0.0f, 3.0f);
        ImGui::SliderFloat("Base height", &m_baseHeight, -2.0f, 5.0f);
        ImGui::SliderFloat("Max opacity", &m_maxOpacity, 0.0f, 1.0f);
        ImGui::SliderFloat("Sky amount", &m_skyAmount, 0.0f, 1.0f);
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

    auto win = FogWindow::create(1280, 720, "Fog Effect");
    app->addWindow(win);
    return app->loop();
}
