/**
 * @file debug_draw1.cpp
 * @brief Immediate-mode debug drawing example.
 * @ingroup debug_examples
 *
 * Demonstrates the DebugDraw singleton for ephemeral geometry:
 *  - Wireframe bounding boxes around objects
 *  - Direction arrows (axes, normals, velocity vectors)
 *  - Wireframe spheres as range/radius indicators
 *  - World-space text labels projected to screen via ImGui
 *  - Persistent primitives with a user-set duration
 *
 * DebugDraw calls are placed in onUpdate() — they last exactly one frame
 * unless a duration > 0 is provided.  GLFWSceneWindow flushes and draws
 * them automatically after the scene render.
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <cmath>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

class DebugDrawWindow : public GLFWSceneWindow {
private:
    // Scene objects
    std::vector<TransformNodePtr> m_objects;
    std::vector<std::string>      m_labels;

    // Debug draw toggles
    bool m_showBoxes{true};
    bool m_showAxes{true};
    bool m_showSpheres{false};
    bool m_showArrows{true};
    bool m_showLabels{true};

    // Persistent draw test
    float m_persistDuration{2.0f};

    // Animated arrow direction
    float m_time{0.0f};

public:
    DebugDrawWindow(int w, int h, std::string title) : GLFWSceneWindow(w, h, title) {}

    static std::shared_ptr<DebugDrawWindow> create(int w, int h, std::string title)
    { return std::make_shared<DebugDrawWindow>(w, h, title); }

    int onSetup() override
    {
        // --- Objects --------------------------------------------------------
        struct ObjDef { glm::vec3 pos; glm::vec3 color; std::string label; };
        const ObjDef defs[] = {
            {{ -4.5f, 0, 0}, {0.2f, 0.5f, 1.0f}, "Cube"},
            {{ -1.5f, 0, 0}, {1.0f, 0.5f, 0.1f}, "Sphere"},
            {{  1.5f, 0, 0}, {0.3f, 0.9f, 0.3f}, "Cylinder"},
            {{  4.5f, 0, 0}, {0.9f, 0.2f, 0.5f}, "Cone"},
        };

        // Cube
        auto cube = Cube::create();
        cube->setSize(1.4f);
        cube->setPos(defs[0].pos);
        auto m0 = Material::create(); m0->setDiffuseColor({defs[0].color, 1}); cube->setMaterial(m0);
        add(cube); m_objects.push_back(cube); m_labels.push_back(defs[0].label);

        // Sphere
        auto sphere = Sphere::create(0.8f, 24, 24);
        sphere->setPos(defs[1].pos);
        auto m1 = Material::create(); m1->setDiffuseColor({defs[1].color, 1}); sphere->setMaterial(m1);
        add(sphere); m_objects.push_back(sphere); m_labels.push_back(defs[1].label);

        // Cylinder
        auto cyl = Cylinder::create(0.5f, 1.4f);
        cyl->setPos(defs[2].pos);
        auto m2 = Material::create(); m2->setDiffuseColor({defs[2].color, 1}); cyl->setMaterial(m2);
        add(cyl); m_objects.push_back(cyl); m_labels.push_back(defs[2].label);

        // Cone
        auto cone = Cone::create(0.6f, 1.4f);
        cone->setPos(defs[3].pos);
        auto m3 = Material::create(); m3->setDiffuseColor({defs[3].color, 1}); cone->setMaterial(m3);
        add(cone); m_objects.push_back(cone); m_labels.push_back(defs[3].label);

        // Ground plane for spatial reference
        auto plane = Plane::create(14.0f, 4.0f);
        plane->setPos({0, -1.5f, 0});
        auto mp = Material::create();
        mp->setDiffuseColor({0.25f, 0.25f, 0.28f, 1.0f});
        mp->setAmbientColor({0.05f, 0.05f, 0.06f, 1.0f});
        plane->setMaterial(mp);
        add(plane);

        cameraManipulator()->setCameraPosition({0, 4, 14});
        return 0;
    }

    void onUpdate() override
    {
        m_time += float(frameTime());
        auto* dd = DebugDraw::instance();

        // ---- Bounding boxes ------------------------------------------------
        if (m_showBoxes) {
            for (auto& obj : m_objects) {
                auto bbox = obj->worldBoundingBox();
                if (bbox.isValid())
                    dd->box(bbox.center(), bbox.extent(), {1.0f, 1.0f, 0.0f, 1.0f});
            }
        }

        // ---- Per-object local axes -----------------------------------------
        if (m_showAxes) {
            for (auto& obj : m_objects) {
                glm::vec3 p = obj->pos();
                dd->axes(p, 1.1f);
            }
        }

        // ---- Bounding spheres (radius indicators) --------------------------
        if (m_showSpheres) {
            const float radii[] = { 1.2f, 0.9f, 0.9f, 1.0f };
            for (int i = 0; i < (int)m_objects.size(); ++i) {
                glm::vec3 c = m_objects[i]->pos();
                dd->sphere(c, radii[i], {0.0f, 1.0f, 1.0f, 0.7f});
            }
        }

        // ---- Animated direction arrow (e.g. wind/force vector) -------------
        if (m_showArrows) {
            float a = m_time * 0.8f;
            glm::vec3 wind = glm::normalize(glm::vec3(std::cos(a), 0.3f, std::sin(a))) * 2.0f;
            dd->arrow({0, 2.5f, 0}, wind, {1.0f, 0.4f, 0.0f, 1.0f});

            // Gravity indicator on each object
            for (auto& obj : m_objects)
                dd->arrow(obj->pos() + glm::vec3(0, 1.2f, 0), {0, -0.8f, 0}, {0.5f, 0.5f, 1.0f, 1.0f});
        }

        // ---- World-space text labels ----------------------------------------
        if (m_showLabels) {
            for (int i = 0; i < (int)m_objects.size(); ++i) {
                glm::vec3 above = m_objects[i]->pos() + glm::vec3(0, 1.6f, 0);
                dd->text(above, m_labels[i], {1.0f, 1.0f, 1.0f, 1.0f});
            }
            dd->text({0, -2.2f, 0}, "Ground plane", {0.6f, 0.6f, 0.6f, 1.0f});
        }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowSize({280, 310}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({16, 16}, ImGuiCond_FirstUseEver);
        ImGui::Begin("DebugDraw Controls");

        ImGui::SeparatorText("Overlay types");
        ImGui::Checkbox("Bounding boxes",    &m_showBoxes);
        ImGui::Checkbox("Local axes",        &m_showAxes);
        ImGui::Checkbox("Bounding spheres",  &m_showSpheres);
        ImGui::Checkbox("Direction arrows",  &m_showArrows);
        ImGui::Checkbox("Text labels",       &m_showLabels);

        ImGui::Spacing();
        ImGui::SeparatorText("Persistent draw");
        ImGui::SliderFloat("Duration (s)", &m_persistDuration, 0.5f, 5.0f);

        if (ImGui::Button("Draw persistent box (origin)")) {
            DebugDraw::instance()->box({0, 0, 0}, {6.0f, 3.0f, 1.0f},
                                       {1.0f, 0.0f, 1.0f, 1.0f}, m_persistDuration);
        }
        if (ImGui::Button("Draw persistent sphere (origin)")) {
            DebugDraw::instance()->sphere({0, 0, 0}, 4.0f,
                                          {0.0f, 1.0f, 0.5f, 1.0f}, m_persistDuration);
        }
        if (ImGui::Button("Draw persistent arrow")) {
            DebugDraw::instance()->arrow({-5, 0, 0}, {2, 2, 0},
                                         {1.0f, 0.8f, 0.0f, 1.0f}, m_persistDuration);
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Help");
        ImGui::TextWrapped("All one-frame calls go in onUpdate().\n"
                           "Persistent draws use duration > 0 and\n"
                           "count down automatically each frame.\n"
                           "Text labels are projected to screen via\n"
                           "ImGui ForegroundDrawList.");
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

    auto win = DebugDrawWindow::create(1280, 800, "Debug Draw");
    win->maximize();
    app->addWindow(win);
    return app->loop();
}
