/**
 * @file mip_bloom1.cpp
 * @brief Wide glow with MipBloomEffect, blended shader materials and a guided camera path.
 * @ingroup effect_examples
 *
 * Demonstrates:
 *  - MipBloomEffect: a wide, soft halo from the mip chain with soft tone mapping
 *  - BlendShaderMaterial: additive glowing orbs and an alpha-blended mist layer
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

const std::string orb_vert = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;
out vec3 worldNormal;

void main()
{
    vec4 p = model * vec4(aPos, 1.0);
    worldPos = p.xyz;
    worldNormal = normalize(mat3(model) * aNormal);
    gl_Position = projection * view * p;
})";

// Bright core fading towards the silhouette, so additive orbs look like light
const std::string orb_frag = R"(
#version 330 core
in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 viewPos;
uniform vec3 glowColor;
uniform float brightness;

out vec4 fragColor;

void main()
{
    vec3 v = normalize(viewPos - worldPos);
    float facing = max(dot(normalize(worldNormal), v), 0.0);
    float core = pow(facing, 1.5);
    fragColor = vec4(glowColor * core * brightness, core);
})";

// Soft radial mist; alpha falls off towards the edge of the disk
const std::string mist_frag = R"(
#version 330 core
in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 mistColor;
uniform float mistOpacity;
uniform float mistRadius;

out vec4 fragColor;

void main()
{
    float r = length(worldPos.xz) / mistRadius;
    float alpha = mistOpacity * (1.0 - smoothstep(0.2, 1.0, r));
    fragColor = vec4(mistColor, alpha);
})";

const char *k_blendModeNames[] = {"Additive", "Alpha", "Background"};

} // namespace

class MipBloomWindow : public GLFWSceneWindow {
private:
    MipBloomEffectPtr m_bloom;
    std::vector<BlendShaderMaterialPtr> m_orbMaterials;
    BlendShaderMaterialPtr m_mist;
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
        auto light = LightManager::instance()->addDirectionalLight();
        light->setDiffuseColor({0.25f, 0.25f, 0.35f});
        light->setDirection({-0.3f, -1.0f, -0.2f});
        light->setEnabled(true);

        // Opaque geometry first
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
        // One program, one material per colour
        auto orbMaterial =
            BlendShaderMaterial::createFromStrings(orb_vert, orb_frag, "mip_bloom1_orb", BlendMode::Additive);
        if (!orbMaterial)
            return -1;

        const glm::vec3 colors[] = {{1.0f, 0.55f, 0.2f}, {0.3f, 0.7f, 1.0f}, {0.9f, 0.35f, 1.0f}};
        for (const auto &color : colors)
        {
            auto material = m_orbMaterials.empty() ? orbMaterial
                                                   : BlendShaderMaterial::create(orbMaterial->program());
            material->setUniform("glowColor", color);
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

        m_mist = BlendShaderMaterial::createFromStrings(orb_vert, mist_frag, "mip_bloom1_mist", BlendMode::Alpha);
        if (!m_mist)
            return -1;
        m_mist->setUniform("mistColor", glm::vec3(0.5f, 0.45f, 0.7f));
        m_mist->setUniform("mistRadius", 12.0f);
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
        for (size_t i = 0; i < m_orbMaterials.size(); ++i)
        {
            const float phase = float(std::fmod(t * 0.5 + double(i) / double(m_orbMaterials.size()), 1.0));
            const float tri = phase < 0.5f ? phase * 2.0f : 2.0f - phase * 2.0f;
            const float pulse = detail::applyEasing(Easing::SmoothStep, tri);
            m_orbMaterials[i]->setBlendMode(BlendMode(m_orbBlendMode));
            m_orbMaterials[i]->setUniform("brightness", m_orbBrightness * (0.4f + 0.6f * pulse));
        }
        m_mist->setUniform("mistOpacity", m_mistOpacity);

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
