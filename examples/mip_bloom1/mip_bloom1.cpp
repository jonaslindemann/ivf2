/**
 * @file mip_bloom1.cpp
 * @brief Wide glow with MipBloomEffect, blended shader materials and a guided camera path.
 * @ingroup effect_examples
 *
 * Demonstrates:
 *  - MipBloomEffect: a wide, soft halo from the mip chain with soft tone mapping
 *  - GlowMaterial: additive glowing orbs, MistMaterial: an alpha-blended mist layer
 *  - GradientSkyMaterial: a night sky dome with a horizon glow and stars
 *  - PathCameraManipulator: the camera follows an authored orbit; drag with the
 *    left button to look around, with the right button or the wheel to float
 *    away from the path. The camera drifts back when left alone.
 *  - Easing::SmoothStep for the pulse of the orbs
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/mip_bloom_effect.h>
#include <ivf/nodes.h>
#include <ivf/tween.h>
#include <ivfui/ui.h>

#include <cmath>

using namespace ivf;
using namespace ivfui;

namespace {

const char *k_blendModeNames[] = {"Additive", "Alpha", "Background"};

} // namespace

class MipBloomWindow : public GLFWSceneWindow {
private:
    MipBloomEffectPtr m_bloom;
    std::vector<GlowMaterialPtr> m_orbMaterials;
    MistMaterialPtr m_mist;
    PathCameraManipulatorPtr m_pathCamera;

    bool m_bloomEnabled{true};
    float m_threshold{0.6f};
    float m_intensity{0.8f};
    float m_spread{0.75f};
    float m_exposure{1.0f};

    int m_orbBlendMode{0};
    float m_orbBrightness{1.6f};
    float m_mistOpacity{0.35f};

    bool m_guided{true};
    float m_orbitSpeed{0.08f};

public:
    MipBloomWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<MipBloomWindow> create(int w, int h, std::string title)
    {
        return std::make_shared<MipBloomWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto light = renderContext()->lights()->addDirectionalLight();
        light->setDiffuseColor({0.25f, 0.25f, 0.35f});
        light->setDirection({-0.3f, -1.0f, -0.2f});
        light->setEnabled(true);

        // Sky dome first; everything drawn later covers it
        auto sky = GradientSkyMaterial::create({0.0f, 0.0f, 0.02f}, {0.05f, 0.04f, 0.1f});
        if (!sky)
            return -1;
        sky->setGroundColor({0.01f, 0.01f, 0.03f});
        sky->setGlowColor({0.08f, 0.05f, 0.14f});
        sky->setStarIntensity(0.8f);
        auto dome = Sphere::create(50.0, 64, 32);
        dome->setMaterial(sky);
        add(dome);

        // Opaque geometry next
        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.08f, 0.08f, 0.12f, 1.0f});
        groundMaterial->setSpecularColor({0.0f, 0.0f, 0.0f, 1.0f}); // no highlight for the bloom to blow up
        auto ground = Plane::create(60.0, 60.0, 4, 4);
        ground->setMaterial(groundMaterial);
        add(ground);

        auto pillarMaterial = Material::create();
        pillarMaterial->setDiffuseColor({0.25f, 0.22f, 0.3f, 1.0f});
        for (int i = 0; i < 6; ++i)
        {
            const float angle = float(i) / 6.0f * 6.2831853f;
            auto pillar = Box::create({0.6f, 3.0f, 0.6f});
            pillar->setPos({7.0f * std::cos(angle), 1.5f, 7.0f * std::sin(angle)});
            pillar->setMaterial(pillarMaterial);
            add(pillar);
        }

        // Blended geometry after the opaque geometry
        // One material per colour; they share one shader program
        for (const glm::vec3 color : {glm::vec3(1.0f, 0.55f, 0.2f), glm::vec3(0.3f, 0.7f, 1.0f),
                                      glm::vec3(0.9f, 0.35f, 1.0f)})
        {
            auto material = GlowMaterial::create(color);
            if (!material)
                return -1;
            m_orbMaterials.push_back(material);
        }

        for (int i = 0; i < 24; ++i)
        {
            const float angle = float(i) / 24.0f * 6.2831853f;
            const float radius = 3.0f + 1.5f * float(i % 3);
            auto orb = Sphere::create(0.25 + 0.1 * (i % 4), 24, 12);
            orb->setPos({radius * std::cos(angle), 0.6f + 0.4f * float(i % 5), radius * std::sin(angle)});
            orb->setMaterial(m_orbMaterials[i % m_orbMaterials.size()]);
            add(orb);
        }

        m_mist = MistMaterial::create({0.5f, 0.45f, 0.7f}, m_mistOpacity, 12.0f);
        if (!m_mist)
            return -1;
        auto mist = Plane::create(24.0, 24.0, 1, 1);
        mist->setPos({0.0f, 0.3f, 0.0f});
        mist->setMaterial(m_mist);
        add(mist);

        m_bloom = MipBloomEffect::create();
        m_bloom->load();
        addEffect(m_bloom);
        setRenderToTexture(true);

        m_pathCamera = PathCameraManipulator::create();

        setClearColor({0.01f, 0.01f, 0.03f, 1.0f});
        cameraManipulator()->setCameraPosition({0.0f, 3.0f, 12.0f});
        return 0;
    }

    /** @brief The authored camera path: a slow orbit that rises and falls. */
    TimelineCamera cameraPath(double t) const
    {
        const float angle = float(t) * m_orbitSpeed * 6.2831853f;
        const glm::vec3 position{11.0f * std::cos(angle), 2.5f + 1.5f * std::sin(angle * 2.0f),
                                 11.0f * std::sin(angle)};
        return TimelineCamera::lookAt(position, {0.0f, 1.0f, 0.0f});
    }

    void onUpdate() override
    {
        m_bloom->setThreshold(m_threshold);
        m_bloom->setIntensity(m_intensity);
        m_bloom->setSpread(m_spread);
        m_bloom->setExposure(m_exposure);

        // Each orb colour pulses in turn, shaped by SmoothStep
        const double t = elapsedTime();
        ShaderMaterial::setGlobalUniform("time", float(t)); // twinkling stars
        for (size_t i = 0; i < m_orbMaterials.size(); ++i)
        {
            const float phase = float(std::fmod(t * 0.5 + double(i) / double(m_orbMaterials.size()), 1.0));
            const float tri = phase < 0.5f ? phase * 2.0f : 2.0f - phase * 2.0f;
            const float pulse = detail::applyEasing(Easing::SmoothStep, tri);
            m_orbMaterials[i]->setBlendMode(BlendMode(m_orbBlendMode));
            m_orbMaterials[i]->setBrightness(m_orbBrightness * (0.4f + 0.6f * pulse));
        }
        m_mist->setOpacity(m_mistOpacity);

        if (m_guided)
        {
            m_pathCamera->update(frameTime());
            m_pathCamera->applyTo(cameraManipulator(), cameraPath(t));
        }
    }

    void onMouseDrag(double x, double y, double dx, double dy, int button) override
    {
        GLFWSceneWindow::onMouseDrag(x, y, dx, dy, button);
        if (m_guided && !ImGui::GetIO().WantCaptureMouse)
            m_pathCamera->handleMouseDrag(dx, dy, button);
    }

    void onScroll(double xoffset, double yoffset) override
    {
        GLFWSceneWindow::onScroll(xoffset, yoffset);
        if (m_guided && !ImGui::GetIO().WantCaptureMouse)
            m_pathCamera->handleScroll(yoffset);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({340, 360}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Mip Bloom");
        if (ImGui::Checkbox("Bloom enabled", &m_bloomEnabled))
            m_bloomEnabled ? enableEffect(0) : disableEffect(0);
        ImGui::SliderFloat("Threshold", &m_threshold, 0.0f, 2.0f);
        ImGui::SliderFloat("Intensity", &m_intensity, 0.0f, 4.0f);
        ImGui::SliderFloat("Spread", &m_spread, 0.0f, 1.0f);
        ImGui::SliderFloat("Exposure", &m_exposure, 0.0f, 4.0f);

        ImGui::SeparatorText("Materials");
        ImGui::Combo("Orb blend mode", &m_orbBlendMode, k_blendModeNames, IM_ARRAYSIZE(k_blendModeNames));
        ImGui::SliderFloat("Orb brightness", &m_orbBrightness, 0.0f, 4.0f);
        ImGui::SliderFloat("Mist opacity", &m_mistOpacity, 0.0f, 1.0f);

        ImGui::SeparatorText("Camera");
        if (ImGui::Checkbox("Guided camera path", &m_guided) && m_guided)
            m_pathCamera->reset();
        ImGui::SliderFloat("Orbit speed", &m_orbitSpeed, 0.0f, 0.3f);
        ImGui::TextDisabled("LMB look, RMB/wheel float; returns to the path");
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

    auto win = MipBloomWindow::create(1280, 720, "Mip Bloom");
    app->addWindow(win);
    return app->loop();
}
