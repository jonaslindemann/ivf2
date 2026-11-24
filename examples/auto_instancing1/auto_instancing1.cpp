/**
 * @file auto_instancing1.cpp
 * @brief Automatic instancing: InstanceNode copies drawn with one call per batch.
 * @ingroup general_examples
 *
 * Demonstrates:
 *  - A field of InstanceNode copies of a few shapes. Copies of the same shape with the
 *    same material are merged into one instanced draw call by the renderer - no special
 *    node type needed (compare InstancedMeshNode in instancing1, which also gives every
 *    copy its own colour)
 *  - Materials set on groups: the copies inherit them, so each group is one batch
 *  - Shadows: the shadow pass draws the same batches
 *  - A ShaderMaterial without instancing support: its copies are drawn one by one
 *  - The Render Statistics window (draw calls, instanced items) and a switch to compare
 *    with automatic instancing off
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/shader_material.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

class AutoInstancingWindow : public GLFWSceneWindow {
private:
    bool m_autoInstancing{true};

public:
    AutoInstancingWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title)
    {}

    static std::shared_ptr<AutoInstancingWindow> create(int w, int h, std::string title)
    {
        return std::make_shared<AutoInstancingWindow>(w, h, title);
    }

    int onSetup() override
    {
        m_autoInstancing = renderContext()->autoInstancing();

        auto lights = renderContext()->lights();
        lights->clearLights();
        lights->setUseShadows(true);
        auto sun = lights->addDirectionalLight();
        sun->setAmbientColor({0.3f, 0.3f, 0.3f});
        sun->setDiffuseColor({1.0f, 0.97f, 0.9f});
        sun->setDirection({-0.5f, -1.0f, -0.3f});
        sun->setEnabled(true);
        sun->setCastShadows(true);
        sun->setShadowStrength(0.4f);
        lights->apply();

        auto ground = Plane::create(40.0, 40.0, 4, 4);
        auto groundMaterial = Material::create();
        groundMaterial->setDiffuseColor({0.45f, 0.47f, 0.5f, 1.0f});
        ground->setMaterial(groundMaterial);
        add(ground);

        // The shapes that are copied. InstanceNode::setNode() turns off their own transform:
        // each copy places the shape with its own position.
        auto sphere = Sphere::create(0.35);
        auto cone = Cone::create(0.35, 0.9);
        auto box = Box::create({0.3f, 0.3f, 0.3f});

        // One group (and material) per shape and colour: every group is one batch
        auto addGroup = [&](const TransformNodePtr &shape, glm::vec3 color, int row, MaterialPtr material = nullptr) {
            if (!material)
            {
                material = Material::create();
                material->setDiffuseColor(glm::vec4(color, 1.0f));
                material->setShininess(40.0f);
            }
            auto group = CompositeNode::create();
            group->setMaterial(material);
            for (int i = 0; i < 24; ++i)
                for (int j = 0; j < 3; ++j)
                {
                    auto copy = InstanceNode::create();
                    copy->setNode(shape);
                    copy->setPos({-11.5f + float(i), 0.45f, -9.0f + float(row * 3 + j)});
                    copy->setEulerAngles(0.0f, float(i * 15 + j * 40), 0.0f);
                    group->add(copy);
                }
            add(group);
        };
        addGroup(sphere, {0.9f, 0.3f, 0.25f}, 0);
        addGroup(cone, {0.3f, 0.75f, 0.35f}, 1);
        addGroup(box, {0.3f, 0.45f, 0.9f}, 2);
        addGroup(sphere, {0.95f, 0.8f, 0.3f}, 3);

        // A shader without the instance attributes: drawn copy by copy (same image)
        auto glow = ShaderMaterial::createFromFiles("shaders/glow.vert", "shaders/glow.frag", "glow");
        if (!glow)
            return -1;
        glow->setUniform("glowColor", glm::vec3(0.4f, 1.0f, 0.8f));
        glow->setUniform("pulse", 0.0f);
        glow->setUniform("time", 0.0f);
        addGroup(box, {}, 4, glow);

        cameraManipulator()->setCameraPosition({0.0f, 9.0f, 16.0f});
        cameraManipulator()->setCameraTarget({0.0f, 0.0f, 0.0f});
        return 0;
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({330, 150}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Automatic instancing");
        if (ImGui::Checkbox("Automatic instancing", &m_autoInstancing))
            renderContext()->setAutoInstancing(m_autoInstancing);
        const auto &main = renderStats().main;
        ImGui::Text("Main pass: %llu items, %llu draw calls", static_cast<unsigned long long>(main.drawn),
                    static_cast<unsigned long long>(main.drawCalls));
        ImGui::Text("Drawn as instances: %llu", static_cast<unsigned long long>(main.instances));
        ImGui::TextDisabled("Windows > Render Statistics shows all passes.");
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

    auto win = AutoInstancingWindow::create(1280, 720, "Automatic instancing");
    app->addWindow(win);
    return app->loop();
}
