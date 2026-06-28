/**
 * @file particles1.cpp
 * @brief Interactive fire/spark particle system example.
 * @ingroup particle_examples
 *
 * Demonstrates the ParticleSystem node:
 *  - Continuous emission with gravity
 *  - Per-particle color and size interpolation
 *  - Camera-facing (billboard) quads
 *  - ImGui sliders to tweak emitter settings at runtime
 *
 * Call ps->update(dt) in onUpdate() — the scene window does NOT call it
 * automatically (letting you control simulation independently of rendering).
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

class ParticleWindow : public GLFWSceneWindow {
private:
    ParticleSystemPtr m_ps;
    float m_emitRate{200.0f};
    float m_gravity{-4.0f};
    float m_minLife{0.8f}, m_maxLife{2.5f};
    bool m_fog{true};
    float m_fogNear{8.0f}, m_fogFar{30.0f};

public:
    ParticleWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<ParticleWindow> create(int w, int h, std::string title)
    { return std::make_shared<ParticleWindow>(w, h, title); }

    int onSetup() override
    {
        auto texture = Texture::create();
        texture->load("assets/particle.jpeg");

        m_ps = ParticleSystem::create(10000);
        m_ps->setEmitRate(m_emitRate);
        m_ps->setLifetime(m_minLife, m_maxLife);
        m_ps->setInitialVelocity({-0.6f, 1.5f, -0.6f}, {0.6f, 4.0f, 0.6f});
        m_ps->setGravity({0, m_gravity, 0});
        m_ps->setStartColor({1.0f, 0.6f, 0.1f, 0.4f});
        m_ps->setEndColor({0.9f, 0.1f, 0.0f, 0.0f});
        m_ps->setStartSize(0.25f);
        m_ps->setEndSize(0.05f);
        m_ps->setBillboard(true);
        m_ps->setTexture(texture);
        m_ps->setDepthFog(m_fog);
        m_ps->setFogRange(m_fogNear, m_fogFar);
        m_ps->start();
        add(m_ps);

        enableAxis();
        cameraManipulator()->setCameraPosition({0, 3, 12});
        return 0;
    }

    void onUpdate() override
    {
        float dt = float(frameTime());
        m_ps->update(dt);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({320, 320}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Particle Settings");

        if (ImGui::SliderFloat("Emit rate", &m_emitRate, 10, 1000)) {
            m_ps->setEmitRate(m_emitRate);
        }
        if (ImGui::SliderFloat("Gravity", &m_gravity, -20, 5)) {
            m_ps->setGravity({0, m_gravity, 0});
        }
        if (ImGui::SliderFloat("Min life", &m_minLife, 0.1f, 5.0f) |
            ImGui::SliderFloat("Max life", &m_maxLife, 0.1f, 5.0f)) {
            m_ps->setLifetime(m_minLife, m_maxLife);
        }
        ImGui::Separator();
        if (ImGui::Checkbox("Depth fog", &m_fog)) {
            m_ps->setDepthFog(m_fog);
        }
        if (ImGui::SliderFloat("Fog near", &m_fogNear, 0.5f, 50.0f) |
            ImGui::SliderFloat("Fog far", &m_fogFar, 1.0f, 80.0f)) {
            m_ps->setFogRange(m_fogNear, m_fogFar);
        }
        ImGui::Separator();
        ImGui::Text("Alive: %d / %d", m_ps->aliveCount(), m_ps->maxParticles());

        if (ImGui::Button(m_ps->isPlaying() ? "Stop" : "Start"))
            m_ps->isPlaying() ? m_ps->stop() : m_ps->start();
        ImGui::SameLine();
        if (ImGui::Button("Burst 500")) m_ps->emit(500);

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

    auto win = ParticleWindow::create(1024, 768, "Particle System");
    win->maximize();
    app->addWindow(win);
    return app->loop();
}
