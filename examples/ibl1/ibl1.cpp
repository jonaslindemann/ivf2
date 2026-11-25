/**
 * @file ibl1.cpp
 * @brief Image-based lighting: PBR spheres lit by an HDR environment.
 * @ingroup general_examples
 *
 * Demonstrates:
 *  - EnvironmentMap: a procedural HDR sky (or an equirectangular .hdr file given on the
 *    command line), precomputed into irradiance, prefiltered reflections and a BRDF table
 *  - PBRMaterial::setEnvironment(): diffuse and specular light from the environment, with
 *    reflections that blur as roughness increases
 *  - The same environment as the Skybox background (setLod() blurs it)
 *  - Linear HDR output: the materials skip their own tone mapping, the window applies ACES
 *    and gamma 2.2
 *
 * Usage: ibl1 [environment.hdr]   (free HDRIs: https://polyhaven.com/hdris)
 */

#include <imgui.h>
#include <ivf/environment_map.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/pbr_material.h>
#include <ivf/skybox.h>
#include <ivfui/ui.h>

#include <array>
#include <cmath>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

class IblWindow : public GLFWSceneWindow {
private:
    std::string m_environmentFile;
    EnvironmentMapPtr m_environment;
    SkyboxPtr m_skybox;
    DirectionalLightPtr m_sunLight;
    std::vector<PBRMaterialPtr> m_materials;

    SkySettings m_sky;
    float m_sunElevation{35.0f}; ///< Degrees above the horizon.
    float m_sunAzimuth{-50.0f};  ///< Degrees around the vertical axis.
    float m_environmentIntensity{1.0f};
    float m_backgroundBlur{0.0f};
    bool m_directSun{true};

public:
    IblWindow(int w, int h, std::string title, std::string environmentFile)
        : GLFWSceneWindow(w, h, title), m_environmentFile(std::move(environmentFile))
    {}

    static std::shared_ptr<IblWindow> create(int w, int h, std::string title, std::string environmentFile)
    {
        return std::make_shared<IblWindow>(w, h, title, std::move(environmentFile));
    }

    glm::vec3 sunDirection() const
    {
        const float e = glm::radians(m_sunElevation);
        const float a = glm::radians(m_sunAzimuth);
        return {std::cos(e) * std::cos(a), std::sin(e), std::cos(e) * std::sin(a)};
    }

    /** @brief (Re)create the environment and hand it to the materials and the skybox. */
    bool loadEnvironment()
    {
        auto environment = EnvironmentMap::create();
        if (!m_environmentFile.empty())
        {
            if (!environment->load(m_environmentFile))
                return false; // the reason is already logged
        }
        else
        {
            m_sky.sunDirection = sunDirection();
            if (!environment->loadSky(m_sky))
                return false;
        }
        m_environment = environment;

        for (auto &material : m_materials)
            material->setEnvironment(m_environment);
        if (m_skybox)
            m_skybox->setCubemap(m_environment->cubemap());
        return true;
    }

    void updateSunLight()
    {
        // Direct light from the sun adds sharp highlights; the environment has the rest
        m_sunLight->setDirection(-sunDirection());
        m_sunLight->setDiffuseColor(m_directSun && m_environmentFile.empty() ? m_sky.sunColor * 2.5f
                                                                              : glm::vec3(0.0f));
        renderContext()->lights()->apply();
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        m_sunLight = lights->addDirectionalLight();
        m_sunLight->setAmbientColor({0.0f, 0.0f, 0.0f});

        // Spheres: rows of materials, roughness increasing to the right
        struct Row {
            glm::vec3 albedo;
            float metallic;
        };
        const std::array<Row, 3> rows{Row{{1.0f, 0.78f, 0.34f}, 1.0f},  // gold
                                      Row{{0.85f, 0.12f, 0.08f}, 0.0f}, // red plastic
                                      Row{{0.9f, 0.9f, 0.88f}, 0.0f}};  // white ceramic
        constexpr int columns = 7;
        for (std::size_t r = 0; r < rows.size(); ++r)
            for (int c = 0; c < columns; ++c)
            {
                auto material = PBRMaterial::create();
                material->setAlbedo(glm::vec4(rows[r].albedo, 1.0f));
                material->setMetallic(rows[r].metallic);
                material->setRoughness(0.05f + 0.95f * float(c) / float(columns - 1));
                material->setToneMapping(false); // linear HDR: the window tone maps
                m_materials.push_back(material);

                auto sphere = Sphere::create(0.45, 48, 24);
                sphere->setPos({-3.3f + 1.1f * float(c), 1.1f - 1.1f * float(r), 0.0f});
                sphere->setMaterial(material);
                add(sphere);
            }

        if (!loadEnvironment())
            return -1;
        m_skybox = Skybox::create(m_environment->cubemap());
        add(m_skybox);
        updateSunLight();

        // HDR output: ACES, and gamma 2.2 because the PBR materials light in linear space
        setToneMapping(ToneMapping::Aces);
        setGamma(2.2f);
        setExposure(1.0f);

        cameraManipulator()->setCameraPosition({0.0f, 0.4f, 7.5f});
        cameraManipulator()->setCameraTarget({0.0f, 0.0f, 0.0f});
        return 0;
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({360, 280}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Image-based lighting");

        if (ImGui::SliderFloat("Environment", &m_environmentIntensity, 0.0f, 3.0f, "%.2f"))
            for (auto &material : m_materials)
                material->setEnvironmentIntensity(m_environmentIntensity);

        if (ImGui::SliderFloat("Background blur", &m_backgroundBlur, 0.0f, 6.0f, "%.1f"))
            m_skybox->setLod(m_backgroundBlur);

        float exposure = this->exposure();
        if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 4.0f, "%.2f", ImGuiSliderFlags_Logarithmic))
            setExposure(exposure);

        int toneMapping = static_cast<int>(this->toneMapping());
        const char *names[ToneMappingCount];
        for (int i = 0; i < ToneMappingCount; ++i)
            names[i] = toneMappingName(static_cast<ToneMapping>(i));
        if (ImGui::Combo("Tone mapping", &toneMapping, names, ToneMappingCount))
            setToneMapping(static_cast<ToneMapping>(toneMapping));

        if (m_environmentFile.empty())
        {
            ImGui::SeparatorText("Procedural sky");
            bool changed = ImGui::SliderFloat("Sun elevation", &m_sunElevation, -5.0f, 90.0f, "%.0f");
            changed |= ImGui::SliderFloat("Sun azimuth", &m_sunAzimuth, -180.0f, 180.0f, "%.0f");
            changed |= ImGui::SliderFloat("Sun intensity", &m_sky.sunIntensity, 0.0f, 200.0f, "%.0f");
            if (changed)
            {
                static_cast<void>(loadEnvironment()); // failures are logged
                updateSunLight();
            }
            if (ImGui::Checkbox("Direct sunlight", &m_directSun))
                updateSunLight();
        }
        else
        {
            ImGui::TextDisabled("Environment: %s", m_environmentFile.c_str());
        }
        ImGui::End();
    }
};

int main(int argc, char **argv)
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = IblWindow::create(1280, 720, "Image-based lighting", argc > 1 ? argv[1] : "");
    app->addWindow(win);
    return app->loop();
}
