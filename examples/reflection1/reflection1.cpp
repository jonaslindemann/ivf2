/**
 * @file reflection1.cpp
 * @brief Planar reflections on a water surface with PlanarReflection and WaterMaterial.
 * @ingroup effect_examples
 *
 * Demonstrates:
 *  - PlanarReflection rendering the scene mirrored in the water plane
 *  - WaterMaterial with Fresnel reflection and animated ripple distortion
 *  - Clipping below the surface: the sunken sphere does not show up in the
 *    reflection, and only the part of the column above water is reflected
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <cmath>

using namespace ivf;
using namespace ivfui;

class ReflectionWindow : public GLFWSceneWindow {
private:
    PlanarReflectionPtr m_reflection;
    WaterMaterialPtr m_water;
    std::vector<TransformNodePtr> m_orbiters;

    glm::vec3 m_waterColor{0.02f, 0.05f, 0.08f};
    float m_rippleStrength{1.0f};
    float m_distortion{0.03f};
    float m_reflectivity{1.0f};
    float m_minReflectivity{0.3f};
    float m_resolutionScale{0.5f};
    bool m_reflectionEnabled{true};

public:
    ReflectionWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<ReflectionWindow> create(int w, int h, std::string title)
    { return std::make_shared<ReflectionWindow>(w, h, title); }

    int onSetup() override
    {
        auto light = LightManager::instance()->addDirectionalLight();
        light->setDiffuseColor({0.9f, 0.85f, 0.8f});
        light->setDirection({-0.4f, -1.0f, -0.6f});
        light->setEnabled(true);

        // Water surface in the plane y = 0
        auto lake = Plane::create(80.0, 80.0, 4, 4);
        add(lake);

        m_reflection = PlanarReflection::create({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
        m_reflection->addHiddenNode(lake);
        addReflection(m_reflection);

        m_water = WaterMaterial::create(m_reflection);
        if (!m_water)
            return -1;
        lake->setMaterial(m_water);

        auto pillarMaterial = Material::create();
        pillarMaterial->setDiffuseColor({0.8f, 0.6f, 0.4f, 1.0f});
        for (int i = 0; i < 8; ++i)
        {
            float a = 6.2831853f * i / 8.0f;
            auto pillar = Box::create({0.8f, 3.0f + i % 3, 0.8f});
            pillar->setPos({9.0f * std::cos(a), (3.0f + i % 3) * 0.5f, 9.0f * std::sin(a) - 4.0f});
            pillar->setMaterial(pillarMaterial);
            add(pillar);
        }

        // Crosses the surface: only the part above water may be reflected
        auto columnMaterial = Material::create();
        columnMaterial->setDiffuseColor({0.3f, 0.8f, 0.4f, 1.0f});
        auto column = Box::create({1.0f, 6.0f, 1.0f});
        column->setPos({-2.5f, 1.0f, -2.0f});
        column->setMaterial(columnMaterial);
        add(column);

        // Entirely below water: must not appear in the reflection
        auto sunkenMaterial = Material::create();
        sunkenMaterial->setDiffuseColor({1.0f, 0.1f, 0.1f, 1.0f});
        sunkenMaterial->setUseLighting(false);
        auto sunken = Sphere::create(1.0);
        sunken->setPos({2.5f, -1.5f, -2.0f});
        sunken->setMaterial(sunkenMaterial);
        add(sunken);

        // Glowing orbiters
        for (int i = 0; i < 12; ++i)
        {
            auto material = Material::create();
            float a = 6.2831853f * i / 12.0f;
            material->setDiffuseColor({0.5f + 0.5f * std::cos(a), 0.5f + 0.5f * std::sin(a), 1.0f, 1.0f});
            material->setUseLighting(false);
            auto orb = Sphere::create(0.25, 16, 8);
            orb->setMaterial(material);
            add(orb);
            m_orbiters.push_back(orb);
        }

        setClearColor({0.05f, 0.04f, 0.12f, 1.0f});
        cameraManipulator()->setCameraPosition({0.0f, 2.5f, 14.0f});
        return 0;
    }

    void onUpdate() override
    {
        float t = float(elapsedTime());
        ShaderMaterial::setGlobalUniform("time", t);

        for (size_t i = 0; i < m_orbiters.size(); ++i)
        {
            float a = 6.2831853f * i / m_orbiters.size() + t * 0.3f;
            m_orbiters[i]->setPos({5.0f * std::cos(a), 2.0f + 0.8f * std::sin(a * 2.0f), 5.0f * std::sin(a) - 4.0f});
        }

        m_water->setWaterColor(m_waterColor);
        m_water->setRippleStrength(m_rippleStrength);
        m_water->setDistortion(m_distortion);
        m_water->setReflectivity(m_reflectivity);
        m_water->setMinReflectivity(m_minReflectivity);
        m_reflection->setResolutionScale(m_resolutionScale);
        m_reflection->setEnabled(m_reflectionEnabled);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({320, 250}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Water");
        ImGui::Checkbox("Reflection", &m_reflectionEnabled);
        ImGui::ColorEdit3("Water color", &m_waterColor.x);
        ImGui::SliderFloat("Ripple strength", &m_rippleStrength, 0.0f, 5.0f);
        ImGui::SliderFloat("Distortion", &m_distortion, 0.0f, 0.15f);
        ImGui::SliderFloat("Reflectivity", &m_reflectivity, 0.0f, 1.0f);
        ImGui::SliderFloat("Min reflectivity", &m_minReflectivity, 0.0f, 1.0f);
        ImGui::SliderFloat("Resolution scale", &m_resolutionScale, 0.1f, 1.0f);
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

    auto win = ReflectionWindow::create(1280, 720, "Planar Reflection");
    app->addWindow(win);
    return app->loop();
}
