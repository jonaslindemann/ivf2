/**
 * @file cascaded_shadows1.cpp
 * @brief Cascaded shadow maps and soft shadow filters for a sun over a large scene.
 * @ingroup effects_examples
 *
 * Demonstrates:
 *  - DirectionalLight::setShadowCascades(): the camera's view range split into up to four
 *    shadow maps, sharp shadows close to the camera and shadows far away from one light
 *  - Light::setShadowFilter(): hard, PCF and PCSS (soft shadows whose penumbra grows with
 *    the distance between caster and receiver)
 *  - Shadow distance, cascade split placement and the cascade debug view (debugShadow 5)
 *  - Node::setCastShadows(false) for a node that receives shadows but casts none
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <array>
#include <cmath>
#include <random>
#include <string>

using namespace ivf;
using namespace ivfui;

class CascadeWindow : public GLFWSceneWindow {
private:
    DirectionalLightPtr m_sun;

    float m_sunElevation{32.0f}; ///< Degrees above the horizon.
    float m_sunAzimuth{-40.0f};  ///< Degrees around the vertical axis.
    int m_cascades{4};
    int m_filter{static_cast<int>(ShadowFilter::Pcss)};
    int m_filterRadius{1};
    float m_softness{0.02f};
    float m_shadowDistance{0.0f};
    float m_splitLambda{0.75f};
    int m_mapSize{2}; ///< Index into mapSizes.
    bool m_showCascades{false};

    static constexpr std::array<int, 4> mapSizes{1024, 2048, 4096, 8192};

public:
    CascadeWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<CascadeWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<CascadeWindow>(w, h, title);
    }

    glm::vec3 sunDirection() const
    {
        const float e = glm::radians(m_sunElevation);
        const float a = glm::radians(m_sunAzimuth);
        return -glm::vec3(std::cos(e) * std::cos(a), std::sin(e), std::cos(e) * std::sin(a));
    }

    void updateSun()
    {
        m_sun->setDirection(sunDirection());
        m_sun->setShadowCascades(m_cascades);
        m_sun->setShadowFilter(static_cast<ShadowFilter>(m_filter));
        m_sun->setShadowFilterRadius(m_filterRadius);
        m_sun->setShadowSoftness(m_softness);
        m_sun->setShadowDistance(m_shadowDistance);
        m_sun->setCascadeSplitLambda(m_splitLambda);
        const int size = mapSizes[static_cast<std::size_t>(m_mapSize)];
        m_sun->setShadowMapSize(size, size);
        renderContext()->lights()->setDebugShadow(m_showCascades ? 5 : 0);
        renderContext()->lights()->apply();
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        lights->setUseShadows(true);

        m_sun = lights->addDirectionalLight();
        m_sun->setAmbientColor({0.32f, 0.34f, 0.4f});
        m_sun->setDiffuseColor({1.0f, 0.95f, 0.85f});
        m_sun->setSpecularColor({0.4f, 0.4f, 0.4f});
        m_sun->setCastShadows(true);
        m_sun->setShadowStrength(0.85f);
        m_sun->setShadowNormalBias(1.0f);
        updateSun();

        // Ground
        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.62f, 0.6f, 0.55f, 1.0f});
        groundMaterial->setAmbientColor({0.62f, 0.6f, 0.55f, 1.0f});
        auto ground = Plane::create(300.0, 300.0, 4, 4);
        ground->setMaterial(groundMaterial);
        add(ground);

        auto material = [](glm::vec3 color) {
            auto m = Material::create();
            m->setDiffuseColor(glm::vec4(color, 1.0f));
            m->setAmbientColor(glm::vec4(color, 1.0f));
            m->setSpecularColor({0.2f, 0.2f, 0.2f, 1.0f});
            return m;
        };
        auto stone = material({0.75f, 0.72f, 0.68f});
        auto wood = material({0.45f, 0.3f, 0.2f});
        auto leaves = material({0.3f, 0.55f, 0.25f});
        auto metal = material({0.35f, 0.45f, 0.6f});

        // Near the camera: a pergola of thin beams, a fine pattern that needs a sharp cascade
        auto pergola = CompositeNode::create();
        for (int i = -4; i <= 4; ++i)
        {
            auto beam = Box::create({0.06f, 0.06f, 3.0f});
            beam->setPos({float(i) * 0.6f, 3.0f, 0.0f});
            pergola->add(beam);
        }
        for (float x : {-2.6f, 2.6f})
            for (float z : {-2.8f, 2.8f})
            {
                auto post = Cylinder::create(0.08, 3.0, 12, 1);
                post->setPos({x, 1.5f, z});
                pergola->add(post);
            }
        pergola->setMaterial(wood);
        pergola->setPos({0.0f, 0.0f, 6.0f});
        add(pergola);

        // A bench and a sphere under it
        auto bench = Box::create({1.2f, 0.05f, 0.3f});
        bench->setPos({1.0f, 0.5f, 5.5f});
        bench->setMaterial(wood);
        add(bench);
        auto ball = Sphere::create(0.3);
        ball->setPos({-1.0f, 0.3f, 5.0f});
        ball->setMaterial(metal);
        add(ball);

        // A glass-like plate: receives shadows, casts none
        auto plate = Box::create({1.0f, 0.02f, 1.0f});
        plate->setPos({-1.0f, 1.2f, 3.5f});
        plate->setMaterial(metal);
        plate->setCastShadows(false);
        add(plate);

        // Further away: trees and pillars up to 140 units from the camera (fixed seed)
        std::mt19937 random(7);
        std::uniform_real_distribution<float> unit(0.0f, 1.0f);
        for (int i = 0; i < 260; ++i)
        {
            const float x = -120.0f + 240.0f * unit(random);
            const float z = 2.0f - 140.0f * unit(random);
            if (std::abs(x) < 8.0f && z > -6.0f)
                continue; // keep the view open
            if (unit(random) < 0.6f)
            {
                const float height = 2.0f + 4.0f * unit(random);
                auto trunk = Cylinder::create(0.2, height, 10, 1);
                trunk->setPos({x, height * 0.5f, z});
                trunk->setMaterial(wood);
                add(trunk);
                auto crown = Sphere::create(1.0 + 1.2 * unit(random), 16, 8);
                crown->setPos({x, height + 0.6f, z});
                crown->setMaterial(leaves);
                add(crown);
            }
            else
            {
                const float height = 1.5f + 6.0f * unit(random);
                auto pillar = Box::create({0.5f, height * 0.5f, 0.5f});
                pillar->setPos({x, height * 0.5f, z});
                pillar->setMaterial(stone);
                add(pillar);
            }
        }

        cameraManipulator()->setFarZ(400.0);
        cameraManipulator()->setCameraPosition({5.0f, 3.2f, 17.0f});
        cameraManipulator()->setCameraTarget({0.0f, 1.0f, 2.0f});
        return 0;
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({380, 380}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Cascaded shadows");

        bool changed = ImGui::SliderInt("Cascades", &m_cascades, 1, 4);
        changed |= ImGui::Checkbox("Show cascades", &m_showCascades);

        const char *sizes[] = {"1024", "2048", "4096", "8192"};
        changed |= ImGui::Combo("Map size", &m_mapSize, sizes, 4);
        changed |= ImGui::SliderFloat("Shadow distance", &m_shadowDistance, 0.0f, 300.0f,
                                      m_shadowDistance == 0.0f ? "auto" : "%.0f");
        changed |= ImGui::SliderFloat("Split lambda", &m_splitLambda, 0.0f, 1.0f, "%.2f");

        ImGui::SeparatorText("Filter");
        const char *filters[ShadowFilterCount];
        for (int i = 0; i < ShadowFilterCount; ++i)
            filters[i] = shadowFilterName(static_cast<ShadowFilter>(i));
        changed |= ImGui::Combo("Filter", &m_filter, filters, ShadowFilterCount);
        if (m_filter == static_cast<int>(ShadowFilter::Pcf))
            changed |= ImGui::SliderInt("PCF radius", &m_filterRadius, 0, 4);
        if (m_filter == static_cast<int>(ShadowFilter::Pcss))
            changed |= ImGui::SliderFloat("Softness", &m_softness, 0.0f, 0.1f, "%.3f");

        ImGui::SeparatorText("Sun");
        changed |= ImGui::SliderFloat("Elevation", &m_sunElevation, 5.0f, 90.0f, "%.0f");
        changed |= ImGui::SliderFloat("Azimuth", &m_sunAzimuth, -180.0f, 180.0f, "%.0f");

        if (changed)
            updateSun();

        const auto &stats = renderStats().shadow;
        ImGui::TextDisabled("Shadow pass: %zu items drawn, %zu culled", stats.drawn, stats.culled);
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = CascadeWindow::create(1280, 720, "Cascaded shadows");
    app->addWindow(win);
    return app->loop();
}
