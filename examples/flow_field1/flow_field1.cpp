/**
 * @file flow_field1.cpp
 * @brief FlowField example — curl-noise particle steering.
 *
 * Demonstrates ivf::FlowField and ivf::curlNoise3D:
 *  - 5 000 particles emitted from a sphere, steered by a curl-noise field
 *  - The field evolves slowly over time — particles form organic, fluid-like streams
 *  - ImGui sliders to tweak field scale, strength, octaves, and time speed live
 *  - Space pauses / resumes the TimeController (particles freeze in place)
 *
 * Controls:
 *   Space  — pause / resume
 *   ESC    — quit
 */

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfui/time_control_panel.h>

#include <imgui.h>

using namespace ivf;
using namespace ivfui;

class ExampleWindow : public GLFWSceneWindow {
private:
    ParticleSystemPtr m_ps;
    FlowFieldPtr      m_field;

    // Live-tweak params
    float m_scale{0.25f};
    float m_strength{3.5f};
    int   m_octaves{2};
    float m_timeScale{0.06f};

public:
    ExampleWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<ExampleWindow> create(int w, int h, std::string title)
    {
        return std::make_shared<ExampleWindow>(w, h, title);
    }

    int onSetup() override
    {
        cameraManipulator()->setCameraPosition(glm::vec3(0.0f, 28.0f, 8.0f));
        cameraManipulator()->setCameraTarget(glm::vec3(0.0f, 0.0f, 0.0f));

        enableHeadlight();

        // Small reference sphere at the origin so the scene isn't empty without particles
        auto sphere = Sphere::create(0.15f);
        auto mat    = Material::create();
        mat->setDiffuseColor(glm::vec4(1.0f, 0.5f, 0.1f, 1.0f));
        sphere->setMaterial(mat);
        sphere->refresh();
        add(sphere);

        auto texture = Texture::create();
        texture->load("assets/particle.jpeg");

        // Flow field — offset into a well-varied noise region (away from origin)
        m_field = FlowField::create();
        m_field->setScale(m_scale);       // 0.25 — features ~4 world units wide
        m_field->setStrength(m_strength); // 3.5 units/sec peak velocity
        m_field->setOctaves(m_octaves);
        m_field->setTimeScale(m_timeScale);
        m_field->setOffset(glm::vec3(31.4f, 17.3f, 57.1f)); // avoid near-zero region at noise origin

        // Particle system — spawn from a large flat box so particles
        // cover diverse noise coordinates and see varied flow directions
        m_ps = ParticleSystem::create(8000);
        m_ps->setEmitRate(400.0f);
        m_ps->setLifetime(6.0f, 12.0f);
        m_ps->setGravity(glm::vec3(0.0f));
        m_ps->setInitialVelocity(glm::vec3(-0.2f, -0.1f, -0.2f), glm::vec3(0.2f, 0.1f, 0.2f));
        m_ps->setEmitFromBox(glm::vec3(-8.0f, -0.5f, -8.0f), glm::vec3(8.0f, 0.5f, 8.0f));
        m_ps->setStartColor(glm::vec4(0.2f, 0.6f, 1.0f, 0.85f));
        m_ps->setEndColor(glm::vec4(0.8f, 0.2f, 0.9f, 0.0f));
        m_ps->setStartSize(0.12f);
        m_ps->setEndSize(0.03f);
        m_ps->setBillboard(true);
        m_ps->setTexture(texture);
        m_ps->start();
        add(m_ps);

        // blendRate 3 — particles align with flow within ~0.3 s but keep some inertia
        m_field->applyToParticleSystem(m_ps, 3.0f);

        addUiWindow(TimeControlPanel::create());

        TimeController::instance()->setScale(1.0f);
        return 0;
    }

    void onUpdate() override
    {
        double t  = TimeController::instance()->elapsed();
        double dt = TimeController::instance()->delta();

        m_field->setTime(static_cast<float>(t));
        m_ps->update(static_cast<float>(dt));
    }

    void onKey(int key, int /*sc*/, int action, int /*mods*/) override
    {
        if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
            TimeController::instance()->togglePause();
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({280, 0}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({10, 10}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Flow Field");

        bool changed = false;
        changed |= ImGui::SliderFloat("Scale",      &m_scale,     0.05f, 2.0f);
        changed |= ImGui::SliderFloat("Strength",   &m_strength,  0.1f, 10.0f);
        changed |= ImGui::SliderInt  ("Octaves",    &m_octaves,   1, 4);
        changed |= ImGui::SliderFloat("Time speed", &m_timeScale, 0.0f, 0.5f);

        if (changed) {
            m_field->setScale(m_scale);
            m_field->setStrength(m_strength);
            m_field->setOctaves(m_octaves);
            m_field->setTimeScale(m_timeScale);
        }

        ImGui::Separator();
        ImGui::Text("Alive : %d / %d", m_ps->aliveCount(), m_ps->maxParticles());
        ImGui::Text("Time  : %.2f s",  TimeController::instance()->elapsed());

        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_SAMPLES, 4);
    auto win = ExampleWindow::create(1280, 720, "Flow Field — curl noise particles");
    app->addWindow(win);
    return app->loop();
}
