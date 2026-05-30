/**
 * @file behaviors1.cpp
 * @brief Node behavior/component system example.
 * @ingroup behavior_examples
 *
 * Demonstrates all built-in Behavior types:
 *  - SpinBehavior     — continuous rotation
 *  - PulseBehavior    — oscillating scale
 *  - HoverHighlightBehavior — material swap on mouse over
 *  - ClickCallbackBehavior  — lambda fired on click
 *  - LambdaBehavior   — arbitrary per-frame logic
 *
 * Selection must be enabled for hover/click behaviors to fire.
 */

#include <iostream>
#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;

class BehaviorsWindow : public GLFWSceneWindow {
private:
    std::string m_log{"Click a shape..."};

public:
    BehaviorsWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<BehaviorsWindow> create(int w, int h, std::string title)
    { return std::make_shared<BehaviorsWindow>(w, h, title); }

    int onSetup() override
    {
        setSelectionEnabled(true);

        /*
        auto lightMgr = LightManager::instance();
        auto light = lightMgr->addPointLight();
        light->setEnabled(true);
        light->setDiffuseColor({1.0f, 1.0f, 1.0f});
        light->setSpecularColor({1.0f, 1.0f, 1.0f});
        light->setAttenuation(1.0f, 0.0f, 0.0f);
        light->setPosition({5.0f, 8.0f, 5.0f});
        lightMgr->apply();
        */

        // --- SpinBehavior ---
        auto spin = Cube::create();
        spin->setPos({-4, 0, 0});
        auto mat1 = Material::create();
        mat1->setDiffuseColor({0.2f, 0.6f, 1.0f, 1.0f});
        mat1->setAmbientColor({0.05f, 0.1f, 0.2f, 1.0f});
        spin->setMaterial(mat1);
        spin->addBehavior(std::make_shared<SpinBehavior>(glm::vec3(0,1,0.3f), 60.0f));
        add(spin);

        // --- PulseBehavior ---
        auto pulse = Cube::create();
        pulse->setPos({-1.5f, 0, 0});
        auto mat2 = Material::create();
        mat2->setDiffuseColor({1.0f, 0.6f, 0.1f, 1.0f});
        mat2->setAmbientColor({0.2f, 0.1f, 0.02f, 1.0f});
        pulse->setMaterial(mat2);
        pulse->addBehavior(std::make_shared<PulseBehavior>(0.6f, 1.4f, 1.5f));
        add(pulse);

        // --- HoverHighlightBehavior + ClickCallbackBehavior ---
        auto btn = Box::create({1.2f, 1.2f, 1.2f});
        btn->setPos({1.5f, 0, 0});
        auto normalMat = Material::create();
        normalMat->setDiffuseColor({0.3f, 0.8f, 0.3f, 1.0f});
        normalMat->setAmbientColor({0.05f, 0.15f, 0.05f, 1.0f});
        auto hoverMat = Material::create();
        hoverMat->setDiffuseColor({1.0f, 0.9f, 0.0f, 1.0f});
        hoverMat->setAmbientColor({0.2f, 0.18f, 0.0f, 1.0f});
        btn->setMaterial(normalMat);
        btn->addBehavior(std::make_shared<HoverHighlightBehavior>(normalMat, hoverMat));
        btn->addBehavior(std::make_shared<ClickCallbackBehavior>([this]{
            m_log = "Box clicked!";
        }));
        add(btn);

        // --- LambdaBehavior (custom float bounce) ---
        auto bounce = CappedCylinder::create(0.5f, 1.0f);
        bounce->setPos({4, 0, 0});
        auto mat4 = Material::create();
        mat4->setDiffuseColor({0.9f, 0.2f, 0.6f, 1.0f});
        mat4->setAmbientColor({0.18f, 0.04f, 0.12f, 1.0f});
        bounce->setMaterial(mat4);
        float t = 0;
        bounce->addBehavior(LambdaBehavior::create([t](Node* n, float dt) mutable {
            t += dt;
            auto* tn = dynamic_cast<TransformNode*>(n);
            if (tn) tn->setPos({4.0f, std::sin(t * 2.5f) * 1.5f, 0.0f});
        }));
        add(bounce);

        enableAxis();
        enableGrid();
        cameraManipulator()->setCameraPosition({0, 4, 10});
        return 0;
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({10, 10}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Behaviors", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Blue sphere  — SpinBehavior");
        ImGui::Text("Orange cube  — PulseBehavior");
        ImGui::Text("Green box    — HoverHighlight + Click");
        ImGui::Text("Pink cyl     — LambdaBehavior (bounce)");
        ImGui::Separator();
        ImGui::TextColored({1,1,0,1}, "%s", m_log.c_str());
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

    auto win = BehaviorsWindow::create(1024, 768, "Behaviors");
    win->maximize();
    app->addWindow(win);
    return app->loop();
}
