/**
 * @file dof1.cpp
 * @brief Depth of field with DepthOfFieldEffect.
 * @ingroup effect_examples
 *
 * Demonstrates DepthOfFieldEffect:
 *  - Manual focus distance with a sharp focus range and a blur ramp
 *  - Auto focus on the surface at the center of the screen
 *  - Small bright spheres that show the bokeh shape when out of focus
 *  - A blur visualization for tuning (red: in front of focus, blue: behind)
 */

#include <imgui.h>
#include <ivf/depth_of_field_effect.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <cmath>

using namespace ivf;
using namespace ivfui;

class DofWindow : public GLFWSceneWindow {
private:
    DepthOfFieldEffectPtr m_dof;
    float m_focusDistance{12.0f};
    float m_focusRange{1.0f};
    float m_blurRange{8.0f};
    float m_maxBlur{12.0f};
    float m_radiusStep{0.75f};
    bool m_autoFocus{false};
    bool m_showBlur{false};
    bool m_enabled{true};

public:
    DofWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<DofWindow> create(int w, int h, std::string title)
    { return std::make_shared<DofWindow>(w, h, title); }

    int onSetup() override
    {
        auto light = LightManager::instance()->addDirectionalLight();
        light->setDiffuseColor({0.9f, 0.85f, 0.8f});
        light->setDirection({-0.4f, -1.0f, -0.3f});
        light->setEnabled(true);

        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.2f, 0.22f, 0.25f, 1.0f});
        auto ground = Plane::create(200.0, 200.0, 4, 4);
        ground->setMaterial(groundMaterial);
        add(ground);

        auto pillarMaterial = Material::create();
        pillarMaterial->setDiffuseColor({0.8f, 0.6f, 0.4f, 1.0f});

        // Unlit white spheres act as small lights that show the bokeh
        auto lightMaterial = Material::create();
        lightMaterial->setDiffuseColor({1.0f, 0.95f, 0.8f, 1.0f});
        lightMaterial->setUseLighting(false);

        for (int row = 0; row < 12; ++row)
        {
            float z = 8.0f - row * 5.0f;
            for (int side = -1; side <= 1; side += 2)
            {
                auto pillar = Box::create({0.8f, 3.0f, 0.8f});
                pillar->setPos({side * 3.0f, 1.5f, z});
                pillar->setMaterial(pillarMaterial);
                add(pillar);

                for (int k = 0; k < 3; ++k)
                {
                    auto bulb = Sphere::create(0.06, 12, 6);
                    bulb->setPos({side * (1.2f + 0.4f * k), 2.0f + 0.5f * std::sin(row + k * 2.0f), z - 1.5f});
                    bulb->setMaterial(lightMaterial);
                    add(bulb);
                }
            }
        }

        m_dof = DepthOfFieldEffect::create();
        m_dof->load();
        addEffect(m_dof);
        setRenderToTexture(true);

        setClearColor({0.03f, 0.03f, 0.06f, 1.0f});
        cameraManipulator()->setCameraPosition({0.0f, 2.0f, 16.0f});
        return 0;
    }

    void onUpdate() override
    {
        m_dof->setFocusDistance(m_focusDistance);
        m_dof->setFocusRange(m_focusRange);
        m_dof->setBlurRange(m_blurRange);
        m_dof->setMaxBlur(m_maxBlur);
        m_dof->setRadiusStep(m_radiusStep);
        m_dof->setAutoFocus(m_autoFocus);
        m_dof->setShowBlur(m_showBlur);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({320, 260}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Depth of Field");
        if (ImGui::Checkbox("Enabled", &m_enabled))
            m_enabled ? enableEffect(0) : disableEffect(0);
        ImGui::Checkbox("Auto focus", &m_autoFocus);
        ImGui::BeginDisabled(m_autoFocus);
        ImGui::SliderFloat("Focus distance", &m_focusDistance, 1.0f, 60.0f);
        ImGui::EndDisabled();
        ImGui::SliderFloat("Focus range", &m_focusRange, 0.0f, 10.0f);
        ImGui::SliderFloat("Blur range", &m_blurRange, 0.5f, 30.0f);
        ImGui::SliderFloat("Max blur (px)", &m_maxBlur, 0.0f, 32.0f);
        ImGui::SliderFloat("Radius step", &m_radiusStep, 0.25f, 2.0f);
        ImGui::Checkbox("Show blur", &m_showBlur);
        ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
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

    auto win = DofWindow::create(1280, 720, "Depth of Field");
    app->addWindow(win);
    return app->loop();
}
