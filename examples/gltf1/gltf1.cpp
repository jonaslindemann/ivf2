/**
 * @file gltf1.cpp
 * @brief Loading and animating glTF 2.0 models.
 * @ingroup general_examples
 *
 * Demonstrates GltfModel:
 *  - loading a .glb/.gltf file (the generated showcase model, or a file given on the
 *    command line) with its node hierarchy and PBR materials (textures, alpha blend and
 *    mask, vertex colours)
 *  - playing its animations: node transforms, a skinned tentacle and morph targets
 *  - lighting the model's materials with an EnvironmentMap, and shadows from the sun
 *
 * Usage: gltf1 [model.glb]   (glTF sample models: https://github.com/KhronosGroup/glTF-Sample-Assets)
 */

#include <imgui.h>
#include <ivf/environment_map.h>
#include <ivf/gl.h>
#include <ivf/gltf_model.h>
#include <ivf/nodes.h>
#include <ivf/pbr_material.h>
#include <ivf/render_queue.h>
#include <ivf/skybox.h>
#include <ivfui/ui.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

class GltfWindow : public GLFWSceneWindow {
private:
    std::string m_file;
    GltfModelPtr m_model;
    EnvironmentMapPtr m_environment;
    bool m_playing{true};
    float m_speed{1.0f};
    float m_time{0.0f};
    double m_lastTime{0.0};

public:
    GltfWindow(int w, int h, const std::string &title, std::string file)
        : GLFWSceneWindow(w, h, title), m_file(std::move(file))
    {}

    [[nodiscard]] static std::shared_ptr<GltfWindow> create(int w, int h, const std::string &title, std::string file)
    {
        return std::make_shared<GltfWindow>(w, h, title, std::move(file));
    }

    int onSetup() override
    {
        // Image-based lighting from a procedural sky, and a sun that casts shadows
        SkySettings sky;
        sky.sunDirection = glm::normalize(glm::vec3(0.6f, 0.55f, 0.5f));
        sky.sunIntensity = 40.0f;
        m_environment = EnvironmentMap::create();
        if (!m_environment->loadSky(sky))
            return -1;
        add(Skybox::create(m_environment->cubemap()));

        auto lights = renderContext()->lights();
        lights->clearLights();
        lights->setUseShadows(true);
        auto sun = lights->addDirectionalLight();
        sun->setDirection(-sky.sunDirection);
        sun->setDiffuseColor({2.2f, 2.0f, 1.8f});
        sun->setAmbientColor({0.0f, 0.0f, 0.0f});
        sun->setCastShadows(true);
        sun->setShadowFilter(ShadowFilter::Pcss);
        sun->setShadowSoftness(0.01f);
        sun->setShadowNormalBias(1.0f);
        lights->apply();

        auto model = GltfModel::load(m_file);
        if (!model)
            return -1; // the reason is logged
        m_model = *model;
        for (auto &material : m_model->materials())
        {
            material->setEnvironment(m_environment);
            material->setToneMapping(false); // the window tone maps
        }
        if (m_model->animationCount() > 0)
            m_model->setAnimation(0);
        add(m_model);

        // A floor under the model and a camera that frames it
        RenderQueue queue;
        m_model->collect(queue, CollectState{});
        BoundingBox bounds = queue.bounds();
        if (!bounds.isValid())
            bounds = BoundingBox({-1.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f});
        const glm::vec3 center = bounds.center();
        const float radius = std::max(glm::length(bounds.size()) * 0.5f, 0.1f);

        auto floorMaterial = PBRMaterial::create();
        floorMaterial->setAlbedo({0.55f, 0.55f, 0.57f, 1.0f});
        floorMaterial->setRoughness(0.85f);
        floorMaterial->setEnvironment(m_environment);
        floorMaterial->setToneMapping(false);
        auto floor = Box::create({radius * 2.0f, 0.02f, radius * 2.0f});
        floor->setPos({center.x, bounds.min().y - 0.021f, center.z});
        floor->setMaterial(floorMaterial);
        add(floor);

        setToneMapping(ToneMapping::Aces);
        setGamma(2.2f);

        cameraManipulator()->setCameraPosition(center + glm::vec3(0.55f, 0.5f, 1.0f) * radius * 1.25f);
        cameraManipulator()->setCameraTarget(center);
        cameraManipulator()->setNearZ(radius * 0.01);
        cameraManipulator()->setFarZ(radius * 50.0);
        return 0;
    }

    void onUpdate() override
    {
        const double now = elapsedTime();
        if (m_playing)
            m_time += float(now - m_lastTime) * m_speed;
        m_lastTime = now;
        if (m_model && m_model->animation() >= 0)
            m_model->setTime(m_time);
    }

    void onDrawUi() override
    {
        if (!m_model)
            return;
        ImGui::SetNextWindowSize({360, 260}, ImGuiCond_FirstUseEver);
        ImGui::Begin("glTF model");
        ImGui::Text("%s", m_model->name().c_str());
        ImGui::TextDisabled("%d nodes, %zu meshes, %zu materials, %d animations", m_model->nodeCount(),
                            m_model->meshNodes().size(), m_model->materials().size(), m_model->animationCount());

        if (m_model->animationCount() > 0)
        {
            ImGui::SeparatorText("Animation");
            int current = m_model->animation();
            const std::string preview = current >= 0 ? m_model->animationName(current) : "(none)";
            if (ImGui::BeginCombo("Animation", preview.c_str()))
            {
                if (ImGui::Selectable("(none)", current < 0))
                    m_model->setAnimation(-1);
                for (int i = 0; i < m_model->animationCount(); ++i)
                    if (ImGui::Selectable(m_model->animationName(i).c_str(), i == current))
                    {
                        m_model->setAnimation(i);
                        m_time = 0.0f;
                    }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("Play", &m_playing);
            ImGui::SameLine();
            bool loop = m_model->looping();
            if (ImGui::Checkbox("Loop", &loop))
                m_model->setLooping(loop);
            ImGui::SliderFloat("Speed", &m_speed, 0.0f, 3.0f, "%.2f");
            if (current >= 0)
            {
                const float duration = m_model->animationDuration(current);
                float t = duration > 0.0f ? std::fmod(m_time, duration) : 0.0f;
                if (ImGui::SliderFloat("Time", &t, 0.0f, duration, "%.2f s"))
                    m_time = t;
            }
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

    const std::string file = argc > 1 ? argv[1] : "assets/models/showcase.glb";
    auto win = GltfWindow::create(1280, 720, "glTF models", file);
    app->addWindow(win);
    return app->loop();
}
