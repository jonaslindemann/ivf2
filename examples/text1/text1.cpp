/**
 * @file text1.cpp
 * @brief Text example
 * @author Jonas Lindemann
 * @example text1.cpp
 * @ingroup primitive_examples
 *
 * This example demonstrates how to create a simple scene with text rendering
 * using the IVF library. It sets up a window, initializes a text node, and
 * renders the text "Ivf++ 2.0" in the center of the window.
 * The example also includes a FPS window and a text window for displaying
 * additional information.
 * The text is rendered using a custom font loaded from a TTF file.
 */

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include "text_window.h"

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    FpsWindowPtr m_fpsWindow;
    TextWindowPtr m_textWindow;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Create a font manager and load a custom font

        auto fontMgr = FontManager::create();
        fontMgr->loadFace("fonts/Gidole-Regular.ttf", "gidole");

        // Create a material for the text node

        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        material->setShininess(40.0);

        // Create an axis node for reference in the scene

        auto axis = Axis::create();

        // Create and configure the text node

        auto text = TextNode::create();
        text->setText("Ivf++ 2.0");
        text->setAlignX(TextAlignX::CENTER);
        text->setAlignY(TextAlignY::MIDDLE);

        // Load a texture (not used directly here, but could be applied)

        auto texture = Texture::create();
        texture->load("assets/planks.png");

        // Assign the material to the text node

        text->setMaterial(material);

        // Add the axis and text nodes to the scene

        this->add(axis);
        this->add(text);

        // Create FPS and text info UI windows
        
        m_fpsWindow = FpsWindow::create();
        m_textWindow = TextWindow::create(text);

        // Add the UI windows to the main window
        
        this->addUiWindow(m_fpsWindow);
        this->addUiWindow(m_textWindow);

        return 0;
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
#ifdef __APPLE__
    app->hint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    auto window = ExampleWindow::create(800, 800, "Text");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
