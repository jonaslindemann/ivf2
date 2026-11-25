/**
 * @file ssao1.cpp
 * @brief Screen-space ambient occlusion on a cluster of blocks, arches and spheres.
 * @ingroup effects_examples
 *
 * Demonstrates SsaoEffect: corners, creases and the ground under objects get darker,
 * which makes their shapes and contacts easier to read. The UI switches the effect,
 * shows the occlusion alone and changes radius, intensity, contrast, samples and blur.
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/ssao_effect.h>
#include <ivfui/ui.h>

#include <cmath>
#include <random>
#include <string>

using namespace ivf;
using namespace ivfui;

class SsaoWindow : public GLFWSceneWindow {
private:
    SsaoEffectPtr m_ssao;
    bool m_enabled{true};

public:
    SsaoWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<SsaoWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<SsaoWindow>(w, h, title);
    }

    static MaterialPtr material(glm::vec3 color)
    {
        auto m = Material::create();
        m->setDiffuseColor(glm::vec4(color, 1.0f));
        m->setAmbientColor(glm::vec4(color, 1.0f));
        m->setSpecularColor({0.15f, 0.15f, 0.15f, 1.0f});
        return m;
    }

    int onSetup() override
    {
        // Soft, mostly ambient light: occlusion matters most where light comes from everywhere
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.4f, -1.0f, -0.6f});
        sun->setDiffuseColor({0.55f, 0.55f, 0.5f});
        sun->setAmbientColor({0.55f, 0.56f, 0.6f});
        sun->setSpecularColor({0.2f, 0.2f, 0.2f});
        lights->apply();

        auto stone = material({0.85f, 0.83f, 0.78f});
        auto clay = material({0.8f, 0.5f, 0.35f});
        auto slate = material({0.45f, 0.5f, 0.6f});

        auto ground = Box::create({12.0f, 0.1f, 12.0f});
        ground->setPos({0.0f, -0.1f, 0.0f});
        ground->setMaterial(stone);
        add(ground);

        // Two walls meeting in a corner
        auto wallA = Box::create({6.0f, 2.0f, 0.15f});
        wallA->setPos({0.0f, 2.0f, -4.0f});
        wallA->setMaterial(stone);
        add(wallA);
        auto wallB = Box::create({0.15f, 2.0f, 4.0f});
        wallB->setPos({-6.0f, 2.0f, 0.0f});
        wallB->setMaterial(stone);
        add(wallB);

        // A stepped pile of blocks (fixed seed)
        std::mt19937 random(3);
        std::uniform_real_distribution<float> unit(0.0f, 1.0f);
        for (int x = 0; x < 6; ++x)
            for (int z = 0; z < 4; ++z)
            {
                const int height = 1 + static_cast<int>(unit(random) * float(4 - z));
                for (int y = 0; y < height; ++y)
                {
                    auto block = Box::create({0.25f, 0.25f, 0.25f});
                    block->setPos({-5.2f + 0.52f * float(x), 0.25f + 0.5f * float(y), -3.4f + 0.52f * float(z)});
                    block->setMaterial(y % 2 == 0 ? clay : stone);
                    add(block);
                }
            }

        // Arches: two pillars and a lintel
        for (int i = 0; i < 3; ++i)
        {
            const float z = -2.5f + 1.6f * float(i);
            for (float x : {1.0f, 2.6f})
            {
                auto pillar = Box::create({0.25f, 1.0f, 0.25f});
                pillar->setPos({x, 1.0f, z});
                pillar->setMaterial(slate);
                add(pillar);
            }
            auto lintel = Box::create({1.05f, 0.2f, 0.3f});
            lintel->setPos({1.8f, 2.2f, z});
            lintel->setMaterial(slate);
            add(lintel);
        }

        // Spheres resting on the ground and against each other
        for (int i = 0; i < 7; ++i)
        {
            const float r = 0.35f + 0.1f * float(i % 3);
            auto sphere = Sphere::create(r, 48, 24);
            sphere->setPos({-2.0f + 0.75f * float(i), r, 2.2f + 0.3f * std::sin(float(i) * 1.7f)});
            sphere->setMaterial(i % 2 == 0 ? clay : slate);
            add(sphere);
        }

        // A torus-like ring of cubes lying on the ground
        for (int i = 0; i < 16; ++i)
        {
            const float a = float(i) * glm::two_pi<float>() / 16.0f;
            auto cube = Box::create({0.18f, 0.18f, 0.18f});
            cube->setPos({4.0f + 1.0f * std::cos(a), 0.18f, 1.8f + 1.0f * std::sin(a)});
            cube->setEulerAngles(0.0f, -glm::degrees(a), 0.0f);
            cube->setMaterial(clay);
            add(cube);
        }

        setRenderToTexture(true);
        m_ssao = SsaoEffect::create();
        m_ssao->load();
        addEffect(m_ssao);

        cameraManipulator()->setCameraPosition({4.5f, 5.0f, 9.5f});
        cameraManipulator()->setCameraTarget({-0.5f, 0.5f, -0.5f});
        return 0;
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({340, 260}, ImGuiCond_FirstUseEver);
        ImGui::Begin("SSAO");

        if (ImGui::Checkbox("Enabled", &m_enabled))
            m_enabled ? enableEffect(0) : disableEffect(0);

        bool show = m_ssao->showOcclusion();
        if (ImGui::Checkbox("Show occlusion", &show))
            m_ssao->setShowOcclusion(show);

        float radius = m_ssao->radius();
        if (ImGui::SliderFloat("Radius", &radius, 0.05f, 3.0f, "%.2f"))
            m_ssao->setRadius(radius);
        float intensity = m_ssao->intensity();
        if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f, "%.2f"))
            m_ssao->setIntensity(intensity);
        float power = m_ssao->power();
        if (ImGui::SliderFloat("Power", &power, 0.5f, 4.0f, "%.2f"))
            m_ssao->setPower(power);
        float bias = m_ssao->bias();
        if (ImGui::SliderFloat("Bias", &bias, 0.0f, 0.2f, "%.3f"))
            m_ssao->setBias(bias);
        int samples = m_ssao->samples();
        if (ImGui::SliderInt("Samples", &samples, 1, 64))
            m_ssao->setSamples(samples);
        int blur = m_ssao->blurRadius();
        if (ImGui::SliderInt("Blur radius", &blur, 0, 4))
            m_ssao->setBlurRadius(blur);

        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = SsaoWindow::create(1280, 720, "Screen-space ambient occlusion");
    app->addWindow(win);
    return app->loop();
}
