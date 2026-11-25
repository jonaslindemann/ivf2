/**
 * @file point_shadows1.cpp
 * @brief Point and spot lights casting shadows onto PBR materials.
 * @ingroup effects_examples
 *
 * Demonstrates:
 *  - PointLight::setCastShadows(): six shadow map faces around the light (a cube map
 *    packed into one texture), so shadows fall on the floor and on every wall
 *  - A spot light casting shadows from one perspective shadow map covering its cone
 *  - Soft shadows with ShadowFilter::Pcss, the light radius set by setShadowSoftness()
 *  - PBRMaterial receiving shadows from point and directional lights
 *  - Node::setCastShadows(false) for the lamp bulbs, which would otherwise enclose their
 *    lights and put everything in shadow
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/pbr_material.h>
#include <ivfui/ui.h>

#include <array>
#include <cmath>
#include <string>

using namespace ivf;
using namespace ivfui;

class PointShadowWindow : public GLFWSceneWindow {
private:
    std::array<PointLightPtr, 2> m_lights;
    SpotLightPtr m_spot;
    std::array<TransformNodePtr, 2> m_lamps;
    TransformNodePtr m_cage;

    int m_filter{static_cast<int>(ShadowFilter::Pcss)};
    int m_filterRadius{1};
    float m_lightRadius{0.08f};
    float m_strength{1.0f};
    bool m_animate{true};
    bool m_secondLight{true};
    bool m_spotLight{true};
    float m_angle{0.0f};

public:
    PointShadowWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<PointShadowWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<PointShadowWindow>(w, h, title);
    }

    static PBRMaterialPtr pbr(glm::vec3 albedo, float roughness, float metallic = 0.0f)
    {
        auto material = PBRMaterial::create();
        material->setAlbedo(glm::vec4(albedo, 1.0f));
        material->setRoughness(roughness);
        material->setMetallic(metallic);
        return material;
    }

    void updateLights()
    {
        for (auto &light : m_lights)
        {
            light->setShadowFilter(static_cast<ShadowFilter>(m_filter));
            light->setShadowFilterRadius(m_filterRadius);
            light->setShadowSoftness(m_lightRadius);
            light->setShadowStrength(m_strength);
        }
        m_spot->setShadowFilter(static_cast<ShadowFilter>(m_filter));
        m_spot->setShadowFilterRadius(m_filterRadius);
        m_spot->setShadowSoftness(m_lightRadius);
        m_spot->setShadowStrength(m_strength);
        m_spot->setEnabled(m_spotLight);
        m_lights[1]->setEnabled(m_secondLight);
        m_lamps[1]->setVisible(m_secondLight);
        placeLights();
    }

    void placeLights()
    {
        // The caged lamp circles the room's centre; the second one moves along the back wall
        const glm::vec3 p0{2.2f * std::cos(m_angle), 1.6f, 2.2f * std::sin(m_angle)};
        const glm::vec3 p1{3.5f * std::sin(m_angle * 0.7f), 2.6f, -3.2f};
        m_lights[0]->setPosition(p0);
        m_lights[1]->setPosition(p1);
        m_lamps[0]->setPos(p0);
        m_cage->setPos(p0);
        m_cage->setEulerAngles(0.0f, glm::degrees(m_angle) * 0.5f, 0.0f);
        m_lamps[1]->setPos(p1);
        renderContext()->lights()->apply();
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        lights->setUseShadows(true);

        const std::array<glm::vec3, 2> colors{glm::vec3(6.0f, 4.2f, 2.6f), glm::vec3(1.6f, 2.4f, 4.5f)};
        for (std::size_t i = 0; i < 2; ++i)
        {
            m_lights[i] = lights->addPointLight();
            m_lights[i]->setDiffuseColor(colors[i]);
            m_lights[i]->setSpecularColor(colors[i]);
            m_lights[i]->setAmbientColor({0.0f, 0.0f, 0.0f});
            m_lights[i]->setAttenuation(1.0f, 0.14f, 0.07f);
            m_lights[i]->setCastShadows(true);
            m_lights[i]->setShadowMapSize(1024, 1024); // six faces of 512 x 512

            // The bulb: emissive, and it must not cast shadows (it encloses the light)
            auto bulbMaterial = pbr({0.0f, 0.0f, 0.0f}, 1.0f);
            bulbMaterial->setEmissive(glm::normalize(colors[i]) * 3.0f);
            auto bulb = Sphere::create(0.08, 16, 8);
            bulb->setMaterial(bulbMaterial);
            bulb->setCastShadows(false);
            m_lamps[i] = bulb;
            add(bulb);
        }

        // A spot light low on the left wall, aimed at the sphere: long shadows to the right
        m_spot = lights->addSpotLight();
        const glm::vec3 spotPos{-5.6f, 2.2f, 1.6f};
        m_spot->setPosition(spotPos);
        m_spot->setDirection(glm::normalize(glm::vec3(-1.2f, 0.4f, -0.6f) - spotPos));
        m_spot->setCutoff(18.0f, 24.0f);
        m_spot->setDiffuseColor({9.0f, 9.0f, 8.0f});
        m_spot->setSpecularColor({9.0f, 9.0f, 8.0f});
        m_spot->setAmbientColor({0.0f, 0.0f, 0.0f});
        m_spot->setAttenuation(1.0f, 0.09f, 0.032f);
        m_spot->setCastShadows(true);
        m_spot->setShadowMapSize(1024, 1024);

        // Room: floor and three walls
        auto plaster = pbr({0.8f, 0.78f, 0.74f}, 0.9f);
        auto floor = Box::create({6.0f, 0.05f, 6.0f});
        floor->setPos({0.0f, -0.05f, 0.0f});
        floor->setMaterial(pbr({0.55f, 0.42f, 0.32f}, 0.6f));
        add(floor);
        auto wall = [&](glm::vec3 size, glm::vec3 pos) {
            auto box = Box::create(size);
            box->setPos(pos);
            box->setMaterial(plaster);
            add(box);
        };
        wall({6.0f, 2.5f, 0.05f}, {0.0f, 2.5f, -6.0f});
        wall({0.05f, 2.5f, 6.0f}, {-6.0f, 2.5f, 0.0f});
        wall({0.05f, 2.5f, 6.0f}, {6.0f, 2.5f, 0.0f});

        // Objects around the room
        auto brass = pbr({1.0f, 0.78f, 0.45f}, 0.3f, 1.0f);
        auto red = pbr({0.7f, 0.1f, 0.08f}, 0.4f);
        auto blue = pbr({0.15f, 0.3f, 0.7f}, 0.25f);
        auto stone = pbr({0.6f, 0.6f, 0.58f}, 0.8f);
        for (int i = 0; i < 6; ++i)
        {
            const float a = float(i) * glm::two_pi<float>() / 6.0f;
            auto column = Cylinder::create(0.18, 3.0, 24, 1);
            column->setPos({4.2f * std::cos(a), 1.5f, 4.2f * std::sin(a)});
            column->setMaterial(stone);
            add(column);
        }
        auto sphere = Sphere::create(0.6, 48, 24);
        sphere->setPos({-1.2f, 0.6f, -0.6f});
        sphere->setMaterial(brass);
        add(sphere);
        auto box = Box::create({0.5f, 0.5f, 0.5f});
        box->setPos({1.3f, 0.5f, -1.0f});
        box->setEulerAngles(0.0f, 25.0f, 0.0f);
        box->setMaterial(red);
        add(box);
        auto capsule = Capsule::create(0.3, 0.6, 32, 4);
        capsule->setPos({0.2f, 0.6f, 1.4f});
        capsule->setMaterial(blue);
        add(capsule);

        // A cage of bars around the first lamp: it throws a pattern on all walls
        auto cage = CompositeNode::create();
        auto bar = pbr({0.08f, 0.08f, 0.08f}, 0.5f, 1.0f);
        for (int i = 0; i < 10; ++i)
        {
            const float a = float(i) * glm::two_pi<float>() / 10.0f;
            auto rod = Cylinder::create(0.012, 0.5, 6, 1);
            rod->setPos({0.3f * std::cos(a), 0.0f, 0.3f * std::sin(a)});
            cage->add(rod);
        }
        for (float y : {-0.25f, 0.25f})
        {
            auto ring = Cylinder::create(0.31, 0.02, 32, 1);
            ring->setPos({0.0f, y, 0.0f});
            cage->add(ring);
        }
        cage->setMaterial(bar);
        m_cage = cage;
        add(cage);

        updateLights();

        cameraManipulator()->setCameraPosition({0.0f, 4.5f, 10.5f});
        cameraManipulator()->setCameraTarget({0.0f, 1.0f, 0.0f});
        return 0;
    }

    void onUpdate() override
    {
        if (m_animate)
        {
            m_angle = 0.35f * static_cast<float>(elapsedTime());
            placeLights();
        }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({360, 260}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Point and spot light shadows");

        bool changed = false;
        const char *filters[ShadowFilterCount];
        for (int i = 0; i < ShadowFilterCount; ++i)
            filters[i] = shadowFilterName(static_cast<ShadowFilter>(i));
        changed |= ImGui::Combo("Filter", &m_filter, filters, ShadowFilterCount);
        if (m_filter == static_cast<int>(ShadowFilter::Pcf))
            changed |= ImGui::SliderInt("PCF radius", &m_filterRadius, 0, 4);
        if (m_filter == static_cast<int>(ShadowFilter::Pcss))
            changed |= ImGui::SliderFloat("Light radius", &m_lightRadius, 0.0f, 0.4f, "%.2f");
        changed |= ImGui::SliderFloat("Shadow strength", &m_strength, 0.0f, 1.0f, "%.2f");
        changed |= ImGui::Checkbox("Second light", &m_secondLight);
        changed |= ImGui::Checkbox("Spot light", &m_spotLight);
        ImGui::Checkbox("Animate", &m_animate);
        if (!m_animate)
            changed |= ImGui::SliderAngle("Angle", &m_angle, 0.0f, 360.0f);
        if (changed)
            updateLights();

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

    auto win = PointShadowWindow::create(1280, 720, "Point and spot light shadows");
    app->addWindow(win);
    return app->loop();
}
