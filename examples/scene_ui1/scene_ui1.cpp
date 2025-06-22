/**
 * @file scene_ui1.cpp
 * @brief Hello IVF example
 * @author Jonas Lindemann
 * @example scene_ui1.cpp
 * @ingroup ui_examples
 *
 * This example demonstrates the use of the IVF library to create a scene with
 * various geometric shapes, a main menu, and multiple UI windows.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include "example_window_1.h"
#include "example_window_2.h"

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    ExampleWindow1Ptr m_exampleWindow1 = ExampleWindow1::create();
    ExampleWindow2Ptr m_exampleWindow2 = ExampleWindow2::create();

    UiMainMenuPtr m_mainMenu;

    bool m_showDemoWindow = false;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Add main menu

        // Create the File menu

        auto fileMenu = UiMenu::create("File");

        // Add Exit item to the File menu

        fileMenu->addItem(UiMenuItem::create("Exit", "ALT+F4", [this]() { this->onExit(); }));

        // Add the File menu to the main menu

        this->mainMenu()->addMenu(fileMenu);

        // Enable rendering to texture

        this->setRenderToTexture(true);

        // Add example UI windows

        this->addUiWindow(m_exampleWindow1);
        this->addUiWindow(m_exampleWindow2);

        // Create and add an axis to the scene

        auto axis = Axis::create();

        this->add(axis);

        // Add 500 random geometric shapes to the scene

        for (auto i = 0; i < 500; i++)
        {
            // Generate a random number to select shape type

            double r = random(0.0, 9.0);

            TransformNodePtr node;

            // Create a random shape based on r

            if (r < 1.0)
                node = Cube::create();
            else if ((r >= 1) && (r < 2.0))
                node = Sphere::create(random(0.2, 1.0));
            else if ((r >= 2.0) && (r < 3.0))
                node = Box::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
            else if ((r >= 3.0) && (r < 4.0))
                node = RoundedBox::create(glm::vec3(random(0.2, 2.0), random(0.2, 2.0), random(0.2, 2.0)));
            else if ((r >= 4.0) && (r < 5.0))
                node = CappedCylinder::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 5.0) && (r < 6.0))
                node = Cylinder::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 6.0) && (r < 7.0))
                node = CappedCone::create(random(0.2, 1.0), random(0.2, 1.0));
            else if ((r >= 7.0) && (r < 8.0))
                node = Cone::create(random(0.2, 1.0), random(0.2, 1.0));
            else
                node = Dodecahedron::create(random(0.2, 1.0));

            // Create and set a random material for the shape

            auto material = Material::create();
            material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));
            material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

            // Set a random position for the shape

            node->setPos(glm::vec3(random(-20.0, 20.0), random(-20.0, 20.0), random(-20.0, 20.0)));
            node->setMaterial(material);

            // Add the shape to the scene

            this->add(node);
        }

        return 0;
    }

    void onExit()
    {
        // Handle exit action, e.g., close the window or application

        this->close();
    }

    void onUpdateUi()
    {
        // Update UI elements

        if (m_showDemoWindow)
            ImGui::ShowDemoWindow(&m_showDemoWindow);

        if (m_exampleWindow1->demoWindowButton())
            m_showDemoWindow = true;

        if (m_exampleWindow1->anotherWindowButton())
            m_exampleWindow2->show();

        if (m_exampleWindow2->closeMeButton())
            m_exampleWindow2->hide();
    }

    void onKey(int key, int scancode, int action, int mods)
    {
        // Handle key events

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            this->close();
        }

        if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        {
            m_showDemoWindow = !m_showDemoWindow;
        }
    }
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = ExampleWindow::create(1280, 800, "Example 2");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
