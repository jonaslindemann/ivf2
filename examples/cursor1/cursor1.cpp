/**
 * @file cursor1.cpp
 * @brief Cursor example
 * @author Jonas Lindemann
 * @example cursor1.cpp
 * @ingroup general_examples
 *
 * This example demonstrates the use of the Cursor node class in the ivf library.
 * It creates multiple cursors with different configurations to showcase:
 * - Basic cursor functionality with axis lines
 * - Configurable size and gap parameters  
 * - Optional ground projection line
 * - Custom color settings
 * - Integration with the scene graph system
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CursorPtr m_cursor1;
    CursorPtr m_cursor2;
    CursorPtr m_cursor3;
    CursorPtr m_cursor4;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Create basic cursor with default settings
        m_cursor1 = Cursor::create(1.0f, 0.1f, true);
        m_cursor1->setPos(glm::vec3(-3.0, 2.0, 0.0));
        this->add(m_cursor1);

        // Create larger cursor without ground projection
        m_cursor2 = Cursor::create(2.0f, 0.2f, false);
        m_cursor2->setPos(glm::vec3(0.0, 1.0, 0.0));
        this->add(m_cursor2);

        // Create cursor with custom colors
        m_cursor3 = Cursor::create(1.5f, 0.15f, true);
        m_cursor3->setPos(glm::vec3(3.0, 1.5, 0.0));
        
        // Set custom colors: bright magenta for X, cyan for Y, yellow for Z, orange for ground
        GLfloat xColor[4] = {1.0f, 0.0f, 1.0f, 1.0f}; // Magenta
        GLfloat yColor[4] = {0.0f, 1.0f, 1.0f, 1.0f}; // Cyan
        GLfloat zColor[4] = {1.0f, 1.0f, 0.0f, 1.0f}; // Yellow
        GLfloat groundColor[4] = {1.0f, 0.5f, 0.0f, 1.0f}; // Orange
        m_cursor3->setAxisColors(xColor, yColor, zColor, groundColor);
        
        this->add(m_cursor3);

        // Create small cursor with large gap
        m_cursor4 = Cursor::create(0.8f, 0.5f, true);
        m_cursor4->setPos(glm::vec3(6.0, 3.0, 0.0));
        this->add(m_cursor4);

        // Set camera position for good view of all cursors
        this->cameraManipulator()->setCameraPosition(glm::vec3(8.0, 5.0, 8.0));
        this->cameraManipulator()->setTargetPosition(glm::vec3(1.5, 1.5, 0.0));

        return 0;
    }

    void onUpdateUi() override
    {
        if (ImGui::Begin("Cursor Controls"))
        {
            ImGui::Text("Cursor Configuration Demo");
            ImGui::Separator();

            // Controls for cursor 1
            if (ImGui::CollapsingHeader("Basic Cursor (Red Position)", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static float size1 = 1.0f;
                static float gap1 = 0.1f;
                static bool showGround1 = true;

                if (ImGui::SliderFloat("Size##1", &size1, 0.1f, 3.0f))
                {
                    m_cursor1->setSize(size1);
                }

                if (ImGui::SliderFloat("Gap##1", &gap1, 0.0f, 1.0f))
                {
                    m_cursor1->setGap(gap1);
                }

                if (ImGui::Checkbox("Show Ground Projection##1", &showGround1))
                {
                    m_cursor1->setShowGroundProjection(showGround1);
                }
            }

            // Controls for cursor 2
            if (ImGui::CollapsingHeader("Large Cursor (Green Position)"))
            {
                static float size2 = 2.0f;
                static float gap2 = 0.2f;
                static bool showGround2 = false;

                if (ImGui::SliderFloat("Size##2", &size2, 0.1f, 4.0f))
                {
                    m_cursor2->setSize(size2);
                }

                if (ImGui::SliderFloat("Gap##2", &gap2, 0.0f, 1.0f))
                {
                    m_cursor2->setGap(gap2);
                }

                if (ImGui::Checkbox("Show Ground Projection##2", &showGround2))
                {
                    m_cursor2->setShowGroundProjection(showGround2);
                }
            }

            // Controls for cursor 3
            if (ImGui::CollapsingHeader("Custom Color Cursor (Blue Position)"))
            {
                static float size3 = 1.5f;
                static float gap3 = 0.15f;
                static bool showGround3 = true;
                static bool useCustomColors = true;

                if (ImGui::SliderFloat("Size##3", &size3, 0.1f, 3.0f))
                {
                    m_cursor3->setSize(size3);
                }

                if (ImGui::SliderFloat("Gap##3", &gap3, 0.0f, 1.0f))
                {
                    m_cursor3->setGap(gap3);
                }

                if (ImGui::Checkbox("Show Ground Projection##3", &showGround3))
                {
                    m_cursor3->setShowGroundProjection(showGround3);
                }

                if (ImGui::Checkbox("Use Custom Colors##3", &useCustomColors))
                {
                    m_cursor3->setUseCustomColors(useCustomColors);
                }

                if (ImGui::Button("Reset to Default Colors##3"))
                {
                    m_cursor3->resetToDefaultColors();
                }
            }

            // Controls for cursor 4
            if (ImGui::CollapsingHeader("Small Cursor with Large Gap"))
            {
                static float size4 = 0.8f;
                static float gap4 = 0.5f;
                static bool showGround4 = true;

                if (ImGui::SliderFloat("Size##4", &size4, 0.1f, 2.0f))
                {
                    m_cursor4->setSize(size4);
                }

                if (ImGui::SliderFloat("Gap##4", &gap4, 0.0f, 2.0f))
                {
                    m_cursor4->setGap(gap4);
                }

                if (ImGui::Checkbox("Show Ground Projection##4", &showGround4))
                {
                    m_cursor4->setShowGroundProjection(showGround4);
                }
            }

            ImGui::Separator();
            ImGui::Text("Use the camera controls to examine the cursors from different angles.");
            ImGui::Text("Each cursor demonstrates different configuration options:");
            ImGui::BulletText("Basic cursor with standard RGB axis colors");
            ImGui::BulletText("Large cursor without ground projection");
            ImGui::BulletText("Custom colored cursor with bright colors");
            ImGui::BulletText("Small cursor with large gap to ground projection");
        }
        ImGui::End();
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

    auto window = ExampleWindow::create(1280, 800, "Cursor Demo");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}