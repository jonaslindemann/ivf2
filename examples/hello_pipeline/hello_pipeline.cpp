/**
 * @file hello_pipeline.cpp
 * @brief Basic Ivf pipeline
 * @author Jonas Lindemann
 * @example hello_pipeline.cpp
 * @ingroup general_examples
 *
 * This example demonstrates the basic usage of the IVF library to create a
 * simple 3D scene with a red cube, lighting, and camera manipulation.
 * It initializes a GLFW application, sets up a window, and renders the scene
 * with a camera. The scene includes a red cube with basic material properties.
 */

#include <iostream>
#include <memory>
#include <vector>

#include <ivfui/glfw_application.h>
#include <ivfui/glfw_window.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWWindow {
private:
    CompositeNodePtr m_scene;
    CameraManipulatorPtr m_camManip;
    CameraPtr m_camera;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Load basic shaders

        auto shaderMgr = ShaderManager::instance();
        shaderMgr->loadBasicShader();

        // Check for shader compilation errors

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Setup lighting

        auto lightMgr = LightManager::instance();

        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 15.0));

        lightMgr->apply();

        // Setup scene

        m_scene = CompositeNode::create();

        auto node = Cube::create();

        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
        material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

        node->setPos(glm::vec3(0.0, 0.0, 0.0));
        node->setMaterial(material);

        m_scene->add(node);

        // Setup camera

        m_camera = Camera::create();
        m_camera->setPosition(glm::vec3(-3.0, 3.0, 3.0));
        m_camera->setTarget(glm::vec3(0.0, 0.0, 0.0));
        m_camera->setUp(glm::vec3(0.0, 1.0, 0.0));
        m_camera->setPerspective(45.0, 0.1f, 100.0f);

        m_camera->setViewport(this->width(), this->height());

        return 0;
    }

    void onDraw()
    {
        // Clear the screen and depth buffer

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the scene graph

        m_scene->draw();
    }

    void onResize(int width, int height)
    {
        // Update the camera viewport when the window is resized
        
        m_camera->setViewport(width, height);
    }
};

// This is a typedef for a shared pointer to the ExampleWindow class

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    // Create a FLFW application instance

    auto app = GLFWApplication::create();

    // Set GLFW hints for OpenGL version and profile

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    // Create our example window

    auto window = ExampleWindow::create(800, 800, "Hello, ivf2");
    window->maximize();

    // Add window to application instance

    app->addWindow(window);

    // Enter application loop. This will block until the application is closed.

    return app->loop();
}
