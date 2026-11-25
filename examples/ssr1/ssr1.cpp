/**
 * @file ssr1.cpp
 * @brief Screen-space reflections on a polished floor.
 * @ingroup effects_examples
 *
 * Demonstrates ScreenSpaceReflectionEffect: columns, spheres and turning blocks are
 * reflected in a dark floor. Surfaces facing up reflect; rough floors blur the reflection,
 * and Fresnel makes it stronger at grazing angles. The effect is combined with SsaoEffect
 * (added first, so the reflections show the occluded image).
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/screen_space_reflection_effect.h>
#include <ivf/ssao_effect.h>
#include <ivfui/ui.h>

#include <array>
#include <cmath>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

class SsrWindow : public GLFWSceneWindow {
private:
    ScreenSpaceReflectionEffectPtr m_ssr;
    SsaoEffectPtr m_ssao;
    std::vector<TransformNodePtr> m_spinners;
    bool m_ssrEnabled{true};
    bool m_ssaoEnabled{true};
    bool m_animate{true};

public:
    SsrWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<SsrWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<SsrWindow>(w, h, title);
    }

    static MaterialPtr material(glm::vec3 color, float shininess = 40.0f)
    {
        auto m = Material::create();
        m->setDiffuseColor(glm::vec4(color, 1.0f));
        m->setAmbientColor(glm::vec4(color, 1.0f));
        m->setSpecularColor({0.6f, 0.6f, 0.6f, 1.0f});
        m->setShininess(shininess);
        return m;
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto key = lights->addDirectionalLight();
        key->setDirection({-0.3f, -1.0f, -0.5f});
        key->setDiffuseColor({0.8f, 0.78f, 0.72f});
        key->setAmbientColor({0.25f, 0.26f, 0.3f});
        key->setSpecularColor({0.8f, 0.8f, 0.8f});
        auto fill = lights->addPointLight();
        fill->setPosition({3.0f, 3.0f, 3.0f});
        fill->setDiffuseColor({0.5f, 0.6f, 0.9f});
        fill->setAmbientColor({0.0f, 0.0f, 0.0f});
        fill->setAttenuation(1.0f, 0.1f, 0.02f);
        lights->apply();

        // Dark, polished floor
        auto floor = Box::create({10.0f, 0.05f, 10.0f});
        floor->setPos({0.0f, -0.05f, 0.0f});
        floor->setMaterial(material({0.08f, 0.08f, 0.09f}, 120.0f));
        add(floor);

        // Columns along the back
        auto marble = material({0.85f, 0.84f, 0.8f});
        for (int i = 0; i < 7; ++i)
        {
            auto column = Cylinder::create(0.25, 4.0, 32, 1);
            column->setPos({-4.5f + 1.5f * float(i), 2.0f, -4.0f});
            column->setMaterial(marble);
            add(column);
        }

        // Coloured spheres and turning blocks
        const std::array<glm::vec3, 5> colors{glm::vec3(0.9f, 0.25f, 0.2f), glm::vec3(0.95f, 0.75f, 0.2f),
                                              glm::vec3(0.3f, 0.8f, 0.4f), glm::vec3(0.25f, 0.5f, 0.95f),
                                              glm::vec3(0.75f, 0.35f, 0.9f)};
        for (std::size_t i = 0; i < colors.size(); ++i)
        {
            const float x = -3.0f + 1.5f * float(i);
            auto sphere = Sphere::create(0.45, 48, 24);
            sphere->setPos({x, 0.45f, -1.2f});
            sphere->setMaterial(material(colors[i]));
            add(sphere);

            auto block = Box::create({0.3f, 0.6f, 0.3f});
            block->setPos({x + 0.4f, 0.6f, 1.0f});
            block->setMaterial(material(colors[colors.size() - 1 - i]));
            add(block);
            m_spinners.push_back(block);
        }

        setRenderToTexture(true);
        m_ssao = SsaoEffect::create();
        m_ssao->setIntensity(0.8f);
        m_ssao->load();
        addEffect(m_ssao);

        m_ssr = ScreenSpaceReflectionEffect::create();
        m_ssr->setStrength(0.7f);
        m_ssr->setRoughness(0.1f);
        m_ssr->load();
        addEffect(m_ssr);

        cameraManipulator()->setCameraPosition({1.5f, 2.2f, 7.5f});
        cameraManipulator()->setCameraTarget({0.0f, 0.8f, -1.0f});
        return 0;
    }

    void onUpdate() override
    {
        if (!m_animate)
            return;
        const float t = static_cast<float>(elapsedTime());
        for (std::size_t i = 0; i < m_spinners.size(); ++i)
            m_spinners[i]->setEulerAngles(0.0f, 40.0f * t + 30.0f * float(i), 0.0f);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({360, 330}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Screen-space reflections");

        if (ImGui::Checkbox("Reflections", &m_ssrEnabled))
            m_ssrEnabled ? enableEffect(1) : disableEffect(1);
        ImGui::SameLine();
        if (ImGui::Checkbox("SSAO", &m_ssaoEnabled))
            m_ssaoEnabled ? enableEffect(0) : disableEffect(0);
        ImGui::SameLine();
        ImGui::Checkbox("Animate", &m_animate);

        bool show = m_ssr->showReflections();
        if (ImGui::Checkbox("Show reflections only", &show))
            m_ssr->setShowReflections(show);

        float strength = m_ssr->strength();
        if (ImGui::SliderFloat("Strength", &strength, 0.0f, 1.0f, "%.2f"))
            m_ssr->setStrength(strength);
        float roughness = m_ssr->roughness();
        if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.2f"))
            m_ssr->setRoughness(roughness);
        float fresnel = m_ssr->fresnel();
        if (ImGui::SliderFloat("Fresnel", &fresnel, 0.0f, 1.0f, "%.2f"))
            m_ssr->setFresnel(fresnel);
        float distance = m_ssr->maxDistance();
        if (ImGui::SliderFloat("Max distance", &distance, 1.0f, 50.0f, "%.1f"))
            m_ssr->setMaxDistance(distance);
        int steps = m_ssr->steps();
        if (ImGui::SliderInt("Steps", &steps, 4, 128))
            m_ssr->setSteps(steps);
        float thickness = m_ssr->thickness();
        if (ImGui::SliderFloat("Thickness", &thickness, 0.05f, 2.0f, "%.2f"))
            m_ssr->setThickness(thickness);
        float angle = m_ssr->surfaceAngle();
        if (ImGui::SliderFloat("Surface angle", &angle, 0.0f, 90.0f, "%.0f"))
            m_ssr->setSurfaceAngle(angle);

        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = SsrWindow::create(1280, 720, "Screen-space reflections");
    app->addWindow(win);
    return app->loop();
}
