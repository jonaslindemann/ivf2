/**
 * @file music_scene1.cpp
 * @brief A scene driven by a song with GLFWMusicSceneWindow, SongTransport and Automation.
 * @ingroup effect_examples
 *
 * Demonstrates the ivfmusic library:
 *  - GLFWMusicSceneWindow: the song position drives the scene; transport bar,
 *    scrubbing, markers and presentation mode (--present)
 *  - Automation: parameters keyed by bar in assets/music_scene1.json, reloaded
 *    when the file changes (F3 shows the current values)
 *  - Beat-synchronised animation from the global `beatPhase` value
 *  - The camera follows the "cameraPos" spline path; drag to look around
 *  - MipBloomEffect and additive BlendShaderMaterial orbs
 *
 * Keys: P play/pause, Left/Right seek, PgUp/PgDn markers, M mute,
 * F1 overlay, F3 automation panel, F4 free camera. Run with --help for options.
 */

#include <ivf/gl.h>
#include <ivf/mip_bloom_effect.h>
#include <ivf/nodes.h>
#include <ivfmusic/ivfmusic.h>

#include <cmath>

using namespace ivf;
using namespace ivfmusic;

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

// beatPhase is published to all shader materials by GLFWMusicSceneWindow
const std::string orb_frag = R"(
#version 330 core
in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 viewPos;
uniform vec3 glowColor;
uniform float pulse;
uniform float beatPhase;

out vec4 fragColor;

void main()
{
    vec3 v = normalize(viewPos - worldPos);
    float core = pow(max(dot(normalize(worldNormal), v), 0.0), 1.5);
    float flash = 1.0 + 0.6 * pulse * exp(-6.0 * beatPhase);
    fragColor = vec4(glowColor * core * flash, core);
})";

constexpr int kOrbCount = 32;

} // namespace

class MusicScene : public GLFWMusicSceneWindow {
private:
    std::vector<TransformNodePtr> m_orbs;
    BlendShaderMaterialPtr m_orbMaterial;
    MipBloomEffectPtr m_bloom;

public:
    using GLFWMusicSceneWindow::GLFWMusicSceneWindow;

protected:
    int onSceneSetup() override
    {
        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.06f, 0.06f, 0.09f, 1.0f});
        groundMaterial->setSpecularColor({0.0f, 0.0f, 0.0f, 1.0f}); // no highlight for the bloom to blow up
        auto ground = Plane::create(80.0, 80.0, 4, 4);
        ground->setMaterial(groundMaterial);
        add(ground);

        m_orbMaterial = BlendShaderMaterial::createFromStrings(orb_vert, orb_frag, "music_scene1_orb");
        if (!m_orbMaterial)
            return -1;

        for (int i = 0; i < kOrbCount; ++i)
        {
            auto orb = Sphere::create(0.35, 24, 12);
            orb->setMaterial(m_orbMaterial);
            add(orb);
            m_orbs.push_back(orb);
        }

        m_bloom = MipBloomEffect::create();
        m_bloom->load();
        addEffect(m_bloom);
        setRenderToTexture(true);

        auto light = LightManager::instance()->addDirectionalLight();
        light->setDiffuseColor({0.3f, 0.3f, 0.4f});
        light->setDirection({-0.3f, -1.0f, -0.2f});
        light->setEnabled(true);

        setClearColor({0.01f, 0.01f, 0.03f, 1.0f});
        return 0;
    }

    void onSceneUpdate(double time, double dt) override
    {
        const float pulse = param("pulse", 0.5f);
        const float beatPulse = std::exp(-6.0f * float(transport().beatPhase()));
        // Spin is a rate; its integral keeps the angle a pure function of the position
        const float spin = float(automation().integral("spin", 1.0, barPosition()));

        for (int i = 0; i < kOrbCount; ++i)
        {
            const float angle = float(i) / kOrbCount * 6.2831853f + spin * 6.2831853f;
            const float radius = 4.0f + 1.5f * float(i % 2);
            const float height = 1.0f + 0.8f * std::sin(float(time) * 0.7f + float(i) * 0.6f);
            m_orbs[i]->setPos({radius * std::cos(angle), height, radius * std::sin(angle)});
            m_orbs[i]->setScale(glm::vec3(1.0f + 0.5f * pulse * beatPulse));
        }

        m_orbMaterial->setUniform("glowColor", param3("color", {0.5f, 0.6f, 1.0f}) * 0.7f);
        m_orbMaterial->setUniform("pulse", pulse);
        m_bloom->setIntensity(param("bloom", 0.8f));
    }
};

int main(int argc, char **argv)
{
    MusicSceneConfig config;
    config.title = "Music Scene";
    config.audioFile = "assets/sounds/pf.mp3";
    config.automationFile = "assets/music_scene1.json";
    config.bpm = 120.0;       // No score file: give the tempo for beats and bars
    config.silentLength = 90; // Used if the audio cannot be played
    return run<MusicScene>(config, argc, argv);
}
