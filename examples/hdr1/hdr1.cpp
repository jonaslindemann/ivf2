/**
 * @file hdr1.cpp
 * @brief HDR rendering: bright lights, exposure, tone mapping and anti-aliasing.
 * @ingroup general_examples
 *
 * Demonstrates:
 *  - Lights brighter than 1: the scene is drawn into a floating-point image, so
 *    highlights keep their range instead of clipping
 *  - GLFWSceneWindow::setToneMapping() (None, Reinhard, ACES, AgX), setExposure() and
 *    setGamma() as the final output step
 *  - MipBloomEffect working on HDR values (its own tone mapping off), so only the truly
 *    bright parts glow
 *  - Anti-aliasing: FXAA (setFxaa()) and multisampling of the offscreen image
 *    (setMsaaSamples())
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/glow_material.h>
#include <ivf/mip_bloom_effect.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <array>

using namespace ivf;
using namespace ivfui;

class HdrWindow : public GLFWSceneWindow {
private:
    PointLightPtr m_warmLight;
    SpotLightPtr m_coolLight;
    MipBloomEffectPtr m_bloom;
    float m_lightIntensity{4.0f};
    int m_msaaIndex{0};
    bool m_bloomEnabled{true};

    static constexpr std::array<int, 4> k_msaaSamples{0, 2, 4, 8};

public:
    HdrWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title)
    {}

    static std::shared_ptr<HdrWindow> create(int w, int h, std::string title)
    {
        return std::make_shared<HdrWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();

        // A dim fill light and two lights far brighter than 1
        auto fill = lights->addDirectionalLight();
        fill->setAmbientColor({0.05f, 0.05f, 0.07f});
        fill->setDiffuseColor({0.15f, 0.15f, 0.2f});
        fill->setSpecularColor({0.0f, 0.0f, 0.0f});
        fill->setDirection({0.3f, -1.0f, -0.4f});

        m_warmLight = lights->addPointLight();
        m_warmLight->setPosition({-2.0f, 2.2f, 1.0f});
        m_warmLight->setAttenuation(1.0f, 0.14f, 0.07f);
        m_warmLight->setAmbientColor({0.0f, 0.0f, 0.0f});

        m_coolLight = lights->addSpotLight();
        m_coolLight->setPosition({3.5f, 4.0f, 2.0f});
        m_coolLight->setDirection({-0.6f, -1.0f, -0.35f});
        m_coolLight->setCutoff(18.0f, 26.0f);
        m_coolLight->setAttenuation(1.0f, 0.05f, 0.01f);
        m_coolLight->setAmbientColor({0.0f, 0.0f, 0.0f});
        updateLights();

        // Ground and a row of objects in different materials
        auto ground = Plane::create(30.0, 30.0, 8, 8);
        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.5f, 0.5f, 0.52f, 1.0f});
        groundMaterial->setSpecularColor({0.2f, 0.2f, 0.2f, 1.0f});
        ground->setMaterial(groundMaterial);
        add(ground);

        const std::array<glm::vec3, 5> colors{
            glm::vec3{0.95f, 0.95f, 0.95f}, glm::vec3{0.9f, 0.25f, 0.2f}, glm::vec3{0.95f, 0.75f, 0.3f},
            glm::vec3{0.25f, 0.45f, 0.95f}, glm::vec3{0.3f, 0.85f, 0.45f}};
        for (std::size_t i = 0; i < colors.size(); ++i)
        {
            auto material = Material::create();
            material->setDiffuseColor(glm::vec4(colors[i], 1.0f));
            material->setSpecularColor({1.0f, 1.0f, 1.0f, 1.0f});
            material->setShininess(60.0f);

            const float x = -4.0f + 2.0f * float(i);
            auto pedestal = Box::create({0.45f, 0.3f, 0.45f}); // half extents
            pedestal->setPos({x, 0.3f, 0.0f});
            pedestal->setMaterial(groundMaterial);
            add(pedestal);

            auto sphere = Sphere::create(0.5);
            sphere->setPos({x, 1.1f, 0.0f});
            sphere->setMaterial(material);
            add(sphere);
        }

        // Bright additive orbs mark the lights (values far above 1: they bloom)
        auto warmGlow = GlowMaterial::create({1.0f, 0.7f, 0.35f}, 6.0f);
        auto coolGlow = GlowMaterial::create({0.4f, 0.6f, 1.0f}, 6.0f);
        if (!warmGlow || !coolGlow)
            return -1;
        auto warmOrb = Sphere::create(0.18);
        warmOrb->setPos(m_warmLight->position());
        warmOrb->setMaterial(warmGlow);
        add(warmOrb);
        auto coolOrb = Sphere::create(0.18);
        coolOrb->setPos(m_coolLight->position());
        coolOrb->setMaterial(coolGlow);
        add(coolOrb);

        // Bloom on the HDR image; the window tone maps once, at the end
        m_bloom = MipBloomEffect::create();
        m_bloom->load();
        m_bloom->setThreshold(1.0f);
        m_bloom->setIntensity(1.2f);
        m_bloom->setSpread(0.85f);
        m_bloom->setToneMapping(false);
        addEffect(m_bloom);

        // HDR output: ACES tone mapping and FXAA, no multisampling (FXAA smooths the edges)
        setToneMapping(ToneMapping::Aces);
        setExposure(1.0f);
        setFxaa(true);
        setMsaaSamples(k_msaaSamples[std::size_t(m_msaaIndex)]);

        setClearColor({0.02f, 0.02f, 0.03f, 1.0f});
        cameraManipulator()->setCameraPosition({0.0f, 3.5f, 9.0f});
        cameraManipulator()->setCameraTarget({0.0f, 1.0f, 0.0f});
        return 0;
    }

    void updateLights()
    {
        const float s = m_lightIntensity;
        m_warmLight->setDiffuseColor(glm::vec3(1.0f, 0.75f, 0.45f) * s);
        m_warmLight->setSpecularColor(glm::vec3(1.0f, 0.8f, 0.6f) * s);
        m_coolLight->setDiffuseColor(glm::vec3(0.45f, 0.65f, 1.0f) * s);
        m_coolLight->setSpecularColor(glm::vec3(0.6f, 0.75f, 1.0f) * s);
        renderContext()->lights()->apply();
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({340, 260}, ImGuiCond_FirstUseEver);
        ImGui::Begin("HDR");

        int toneMapping = static_cast<int>(this->toneMapping());
        const char *names[ToneMappingCount];
        for (int i = 0; i < ToneMappingCount; ++i)
            names[i] = toneMappingName(static_cast<ToneMapping>(i));
        if (ImGui::Combo("Tone mapping", &toneMapping, names, ToneMappingCount))
            setToneMapping(static_cast<ToneMapping>(toneMapping));

        float exposure = this->exposure();
        if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 4.0f, "%.2f", ImGuiSliderFlags_Logarithmic))
            setExposure(exposure);

        float gamma = this->gamma();
        if (ImGui::SliderFloat("Gamma", &gamma, 1.0f, 2.4f, "%.2f"))
            setGamma(gamma);

        if (ImGui::SliderFloat("Light intensity", &m_lightIntensity, 0.5f, 10.0f, "%.1f"))
            updateLights();

        if (ImGui::Checkbox("Bloom", &m_bloomEnabled))
            m_bloom->program()->setEnabled(m_bloomEnabled);

        bool fxaa = this->fxaa();
        if (ImGui::Checkbox("FXAA", &fxaa))
            setFxaa(fxaa);

        const char *msaaNames[] = {"Off", "2x", "4x", "8x"};
        if (ImGui::Combo("MSAA", &m_msaaIndex, msaaNames, int(k_msaaSamples.size())))
            setMsaaSamples(k_msaaSamples[std::size_t(m_msaaIndex)]);

        ImGui::TextDisabled("Tone mapping None: values above 1 clip.");
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = HdrWindow::create(1280, 720, "HDR rendering");
    app->addWindow(win);
    return app->loop();
}
