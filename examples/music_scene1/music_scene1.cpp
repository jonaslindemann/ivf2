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
 *  - MipBloomEffect, GlowMaterial orbs and a GradientSkyMaterial sky dome
 *  - Live control: MIDI faders and pads mapped in assets/music_scene1_controls.json
 *    change the parameters on top of the automation; pad "burst" is a trigger read
 *    with triggerAge(). F6 records a take into music_scene1.take.json.
 *
 * Keys: P play/pause, Left/Right seek, PgUp/PgDn markers, M mute,
 * F1 overlay, F3 automation panel, F4 free camera, F5 live panel, F6 record.
 * Run with --help for options.
 */

#include <ivf/gl.h>
#include <ivf/mip_bloom_effect.h>
#include <ivf/nodes.h>
#include <ivfmusic/ivfmusic.h>

#include <cmath>

using namespace ivf;
using namespace ivfmusic;

namespace {

constexpr int kOrbCount = 32;

} // namespace

class MusicScene : public GLFWMusicSceneWindow {
private:
    std::vector<TransformNodePtr> m_orbs;
    GlowMaterialPtr m_orbMaterial;
    GradientSkyMaterialPtr m_sky;
    MipBloomEffectPtr m_bloom;

public:
    using GLFWMusicSceneWindow::GLFWMusicSceneWindow;

protected:
    int onSceneSetup() override
    {
        // Sky dome first; everything drawn later covers it
        m_sky = GradientSkyMaterial::create({0.0f, 0.0f, 0.02f}, {0.04f, 0.04f, 0.08f});
        if (!m_sky)
            return -1;
        m_sky->setGroundColor({0.01f, 0.01f, 0.03f});
        m_sky->setStarIntensity(0.7f);
        auto dome = Sphere::create(50.0, 64, 32);
        dome->setMaterial(m_sky);
        add(dome);

        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.06f, 0.06f, 0.09f, 1.0f});
        groundMaterial->setSpecularColor({0.0f, 0.0f, 0.0f, 1.0f}); // no highlight for the bloom to blow up
        auto ground = Plane::create(80.0, 80.0, 4, 4);
        ground->setMaterial(groundMaterial);
        add(ground);

        m_orbMaterial = GlowMaterial::create();
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

        auto light = renderContext()->lights()->addDirectionalLight();
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

        // A live "burst" pushes the orbs outwards; triggerAge() also covers recorded hits
        const float burst = triggerVelocity("burst") * float(std::exp(-3.0 * triggerAge("burst")));

        for (int i = 0; i < kOrbCount; ++i)
        {
            const float angle = float(i) / kOrbCount * 6.2831853f + spin * 6.2831853f;
            const float radius = 4.0f + 1.5f * float(i % 2) + 3.0f * burst;
            const float height = 1.0f + 0.8f * std::sin(float(time) * 0.7f + float(i) * 0.6f);
            m_orbs[i]->setPos({radius * std::cos(angle), height, radius * std::sin(angle)});
            m_orbs[i]->setScale(glm::vec3(1.0f + 0.5f * pulse * beatPulse));
        }

        // The orbs flash on each beat, more strongly when the pulse is high
        const glm::vec3 color = param3("color", {0.5f, 0.6f, 1.0f});
        m_orbMaterial->setColor(color * 0.7f);
        m_orbMaterial->setBrightness(1.0f + 0.6f * pulse * beatPulse);
        // The horizon glows faintly in the orb colour
        m_sky->setGlowColor(color * 0.06f);
        m_bloom->setIntensity(param("bloom", 0.8f));
    }
};

int main(int argc, char **argv)
{
    MusicSceneConfig config;
    config.title = "Music Scene";
    config.audioFile = "assets/sounds/pf.mp3";
    config.automationFile = "assets/music_scene1.json";
    config.controlsFile = "assets/music_scene1_controls.json"; // MIDI; only the devices named there are opened
    config.bpm = 120.0;       // No score file: give the tempo for beats and bars
    config.silentLength = 90; // Used if the audio cannot be played
    return run<MusicScene>(config, argc, argv);
}
