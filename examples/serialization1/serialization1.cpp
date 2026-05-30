/**
 * @file serialization1.cpp
 * @brief Scene serialisation demo — save, load, and clear a scene graph.
 *
 * Builds a small mixed scene (Spheres, Boxes, nested CompositeNode), then:
 *   - "Save"  → writes "scene.json" to the working directory
 *   - "Load"  → reads "scene.json" and replaces the current scene
 *   - "Clear" → empties the scene
 *   - "Rebuild" → reconstructs the original scene from code
 *
 * The JSON is printed in the ImGui panel so you can inspect the schema.
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <fstream>
#include <sstream>

using namespace ivf;
using namespace ivfui;

static const char* SAVE_PATH = "scene.json";

class Serialization1Window : public GLFWSceneWindow {
    std::string m_jsonPreview;
    bool        m_loaded{false};

public:
    Serialization1Window(int w, int h, const std::string& title)
        : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<Serialization1Window> create(int w, int h, const std::string& t)
    { return std::make_shared<Serialization1Window>(w, h, t); }

    int onSetup() override
    {
        SceneSerializer::registerBuiltinTypes();

        enableAxis();
        enableGrid();
        cameraManipulator()->setCameraPosition({0, 6, 16});

        auto lightMgr = LightManager::instance();
        lightMgr->clearLights();
        auto light = lightMgr->addDirectionalLight();
        light->setDirection({-1, -1.5f, -1});
        light->setDiffuseColor({1.2f, 1.2f, 1.2f});
        light->setAmbientColor({0.2f, 0.2f, 0.2f});
        light->setEnabled(true);
        lightMgr->setUseLighting(true);

        rebuildScene();
        return 0;
    }

    void rebuildScene()
    {
        scene()->clear();

        // A few spheres with different materials
        const glm::vec4 colours[] = {
            {1, 0.3f, 0.3f, 1}, {0.3f, 1, 0.4f, 1}, {0.3f, 0.5f, 1, 1},
        };
        for (int i = 0; i < 3; ++i) {
            auto s = Sphere::create(0.7f);
            s->setPos({float(i * 2 - 2), 0.7f, 0});
            auto mat = Material::create();
            mat->setDiffuseColor(colours[i]);
            s->setMaterial(mat);
            s->setName("Sphere_" + std::to_string(i));
            add(s);
        }

        // A nested group with boxes
        auto group = CompositeNode::create();
        group->setName("BoxGroup");
        group->setPos({0, 0, -3});
        for (int i = 0; i < 3; ++i) {
            auto b = Box::create({0.8f, 0.8f, 0.8f});
            b->setPos({float(i * 2 - 2), 0.4f, 0});
            auto mat = Material::create();
            mat->setDiffuseColor({0.9f, 0.7f, 0.2f, 1});
            mat->setShininess(80);
            b->setMaterial(mat);
            b->setName("Box_" + std::to_string(i));
            group->add(b);
        }
        add(group);
    }

    void doSave()
    {
        if (SceneSerializer::save(scene(), SAVE_PATH)) {
            // Read back for preview
            std::ifstream f(SAVE_PATH);
            std::ostringstream ss;
            ss << f.rdbuf();
            m_jsonPreview = ss.str();
            if (m_jsonPreview.size() > 3000)
                m_jsonPreview = m_jsonPreview.substr(0, 3000) + "\n... (truncated)";
        }
    }

    void doLoad()
    {
        auto loaded = SceneSerializer::load(SAVE_PATH);
        if (loaded) {
            scene()->clear();
            for (auto& child : loaded->nodes())
                add(child);
            m_loaded = true;
        }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Once);
        ImGui::SetNextWindowSize({380, 600}, ImGuiCond_Once);
        ImGui::Begin("Scene Serialization");

        if (ImGui::Button("Rebuild Scene")) rebuildScene();
        ImGui::SameLine();
        if (ImGui::Button("Save"))  doSave();
        ImGui::SameLine();
        if (ImGui::Button("Load"))  doLoad();
        ImGui::SameLine();
        if (ImGui::Button("Clear")) scene()->clear();

        if (m_loaded)
            ImGui::TextColored({0.3f, 1, 0.3f, 1}, "Scene loaded from %s", SAVE_PATH);

        ImGui::Separator();
        ImGui::Text("JSON preview (after Save):");
        ImGui::BeginChild("json", {0, 400}, true);
        ImGui::TextUnformatted(m_jsonPreview.c_str());
        ImGui::EndChild();

        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_SAMPLES, 4);

    auto win = Serialization1Window::create(1280, 800, "Scene Serialization — serialization1");
    app->addWindow(win);
    return app->loop();
}
