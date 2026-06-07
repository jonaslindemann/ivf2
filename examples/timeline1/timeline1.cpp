/**
 * @file timeline1.cpp
 * @brief Timeline scene scripting example.
 *
 * Demonstrates SceneTimeline and SceneTimelinePlayer with three scripted clips.
 */

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/bloom_effect.h>
#include <ivf/chromatic_effect.h>
#include <ivf/tint_effect.h>
#include <ivf/vignette_effect.h>

#include <ivfui/ui.h>

#include <imgui.h>

#include <cmath>
#include <algorithm>
#include <memory>
#include <numbers>
#include <vector>

using namespace ivf;
using namespace ivfui;

namespace {

MaterialPtr makeMaterial(const glm::vec4& color, float shininess = 32.0f)
{
    auto material = Material::create();
    material->setDiffuseColor(color);
    material->setAmbientColor(glm::vec4(glm::vec3(color) * 0.25f, color.a));
    material->setSpecularColor(glm::vec4(1.0f));
    material->setShininess(shininess);
    return material;
}

CompositeNodePtr makeOpeningScene(SpherePtr& sphere)
{
    auto root = CompositeNode::create();

    auto plane = Plane::create(12.0, 12.0);
    plane->setPos(glm::vec3(0.0f, -0.05f, 0.0f));
    plane->setMaterial(makeMaterial(glm::vec4(0.18f, 0.20f, 0.24f, 1.0f)));
    root->add(plane);

    sphere = Sphere::create(1.25f);
    sphere->setPos(glm::vec3(0.0f, 1.25f, 0.0f));
    sphere->setMaterial(makeMaterial(glm::vec4(0.95f, 0.72f, 0.35f, 1.0f), 64.0f));
    sphere->refresh();
    root->add(sphere);

    return root;
}

CompositeNodePtr makeBuildScene(CompositeNodePtr& cubes)
{
    auto root = CompositeNode::create();
    cubes = CompositeNode::create();

    constexpr int gridSize = 4;
    for (int ix = -gridSize; ix <= gridSize; ++ix) {
        for (int iz = -gridSize; iz <= gridSize; ++iz) {
            auto cube = Box::create();
            float height = 0.35f + 1.5f * std::abs(std::sin(ix * 0.7f) * std::cos(iz * 0.6f));
            cube->setSize(glm::vec3(0.72f, height, 0.72f));
            cube->setPos(glm::vec3(ix * 1.25f, height * 0.5f, iz * 1.25f));
            cube->storePos();

            float hue = static_cast<float>(ix + gridSize) / static_cast<float>(gridSize * 2);
            cube->setMaterial(makeMaterial(glm::vec4(hsvToRgb(190.0f + hue * 130.0f, 0.70f, 0.92f), 1.0f)));
            cubes->add(cube);
        }
    }

    root->add(cubes);
    return root;
}

CompositeNodePtr makeFinaleScene(CompositeNodePtr& ring)
{
    auto root = CompositeNode::create();
    ring = CompositeNode::create();

    auto center = Sphere::create(0.9f);
    center->setPos(glm::vec3(0.0f, 1.2f, 0.0f));
    center->setMaterial(makeMaterial(glm::vec4(0.35f, 0.85f, 1.0f, 1.0f), 96.0f));
    center->refresh();
    root->add(center);

    constexpr int count = 24;
    for (int i = 0; i < count; ++i) {
        float a = static_cast<float>(i) / static_cast<float>(count) * std::numbers::pi_v<float> * 2.0f;
        auto box = Box::create();
        box->setSize(glm::vec3(0.35f, 1.0f, 0.35f));
        box->setPos(glm::vec3(std::cos(a) * 4.0f, 0.5f, std::sin(a) * 4.0f));
        box->setEulerAngles(0.0f, -a, 0.0f);
        box->setMaterial(makeMaterial(glm::vec4(hsvToRgb(i * 360.0f / count, 0.78f, 0.95f), 1.0f)));
        ring->add(box);
    }

    root->add(ring);
    return root;
}

} // namespace

class ExampleWindow : public GLFWSceneWindow {
private:
    SceneTimelinePtr m_timeline;
    SceneTimelinePlayerPtr m_player;

    SpherePtr m_openingSphere;
    CompositeNodePtr m_buildCubes;
    CompositeNodePtr m_finaleRing;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title) {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup() override
    {
        enableHeadlight();
        setClearColor(glm::vec4(0.055f, 0.065f, 0.078f, 1.0f));

        auto openingRoot = makeOpeningScene(m_openingSphere);
        auto buildRoot = makeBuildScene(m_buildCubes);
        auto finaleRoot = makeFinaleScene(m_finaleRing);

        auto openingTint = TintEffect::create();
        openingTint->setTintColor(glm::vec3(1.12f, 0.94f, 0.72f));
        openingTint->setTintStrength(0.22f);
        openingTint->load();

        auto openingVignette = VignetteEffect::create();
        openingVignette->setSize(0.95f);
        openingVignette->setSmoothness(0.55f);
        openingVignette->load();

        auto buildBloom = BloomEffect::create();
        buildBloom->setThreshold(0.85f);
        buildBloom->setIntensity(0.55f);
        buildBloom->load();

        auto finaleChromatic = ChromaticEffect::create();
        finaleChromatic->setOffset(0.006f);
        finaleChromatic->load();

        auto finaleVignette = VignetteEffect::create();
        finaleVignette->setSize(0.78f);
        finaleVignette->setSmoothness(0.38f);
        finaleVignette->load();

        m_timeline = SceneTimeline::create();
        m_timeline->setLoop(true);

        m_timeline->addScene("Opening", 5.0)
            .root(openingRoot)
            .camera(TimelineCamera::lookAt(glm::vec3(0.0f, 4.0f, 9.0f), glm::vec3(0.0f, 0.9f, 0.0f), 42.0))
            .effect(openingTint)
            .effect(openingVignette)
            .onUpdate([this](double localTime, double) {
                if (m_openingSphere)
                    m_openingSphere->setEulerAngles(0.0f, static_cast<float>(localTime * 0.45), 0.0f);
            });

        m_timeline->addScene("Build", 6.0)
            .root(buildRoot)
            .camera(TimelineCamera::lookAt(glm::vec3(7.5f, 5.0f, 8.0f), glm::vec3(0.0f, 0.9f, 0.0f), 48.0))
            .effect(buildBloom)
            .onUpdate([this](double localTime, double) {
                if (!m_buildCubes)
                    return;

                m_buildCubes->setEulerAngles(0.0f, static_cast<float>(localTime * 0.25), 0.0f);
                for (size_t i = 0; i < m_buildCubes->count(); ++i) {
                    auto node = std::dynamic_pointer_cast<TransformNode>(m_buildCubes->at(i));
                    if (!node)
                        continue;
                    auto pos = node->storedPos();
                    pos.y += 0.22f * std::sin(static_cast<float>(localTime) * 2.0f + static_cast<float>(i) * 0.35f);
                    node->setPos(pos);
                }
            });

        m_timeline->addScene("Finale", 5.0)
            .root(finaleRoot)
            .camera(TimelineCamera::lookAt(glm::vec3(-6.5f, 6.5f, 7.0f), glm::vec3(0.0f, 1.0f, 0.0f), 50.0))
            .effect(finaleChromatic)
            .effect(finaleVignette)
            .onUpdate([this](double localTime, double) {
                if (m_finaleRing)
                    m_finaleRing->setEulerAngles(0.0f, static_cast<float>(localTime * 0.65), 0.0f);
            });

        m_player = SceneTimelinePlayer::create(this, m_timeline);
        m_timeline->play();

        return 0;
    }

    void onUpdate() override
    {
        if (m_player)
            m_player->update(TimeController::instance()->delta());
    }

    void onKey(int key, int, int action, int) override
    {
        if (action != GLFW_PRESS || !m_timeline)
            return;

        if (key == GLFW_KEY_SPACE) {
            m_timeline->togglePlay();
        } else if (key == GLFW_KEY_RIGHT) {
            m_timeline->nextScene();
        } else if (key == GLFW_KEY_LEFT) {
            m_timeline->previousScene();
        } else if (key == GLFW_KEY_R) {
            m_timeline->restart();
        }
    }

    void onDrawUi() override
    {
        if (!m_timeline)
            return;

        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(12.0f, io.DisplaySize.y - 12.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.48f);
        ImGui::Begin("##timeline_status", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav);

        auto* scene = m_timeline->activeScene();
        ImGui::Text("Scene: %s", scene ? scene->name().c_str() : "none");
        ImGui::Text("Time : %.1f / %.1f", m_timeline->time(), m_timeline->duration());
        ImGui::TextDisabled("Space=play/pause  Left/Right=scene  R=restart");
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

    auto window = ExampleWindow::create(1280, 720, "Timeline scene scripting");
    app->addWindow(window);
    return app->loop();
}
