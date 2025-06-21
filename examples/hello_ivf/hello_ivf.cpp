/**
 * @file hello_ivf.cpp
 * @brief Hello IVF example
 * @author Jonas Lindemann
 * @example hello_ivf.cpp
 * @ingroup general_examples
 *
 * This example demonstrates the basic usage of the IVF library to create a
 * simple 3D scene with various geometric shapes, lighting, and camera manipulation.
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

        auto shaderMgr = ShaderManager::create();

        // Load the default basic shader program

        shaderMgr->loadBasicShader();

        // Check for shader compilation and linking errors

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Setup lighting

        auto lightMgr = LightManager::instance();

        // Add a point light to the scene

        auto pointLight1 = lightMgr->addPointLight();

        // Enable the point light

        pointLight1->setEnabled(true);

        // Set the diffuse color of the point light

        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));

        // Set the specular color of the point light

        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));

        // Set attenuation factors for the point light

        pointLight1->setAttenuation(1.0, 0.0, 0.0);

        // Set the position of the point light

        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));

        // Apply the lighting setup

        lightMgr->apply();

        // Setup scene

        m_scene = CompositeNode::create();

        // Add an axis to the scene for reference

        auto axis = Axis::create();

        m_scene->add(axis);

        // Add 500 random geometric shapes to the scene

        for (auto i = 0; i < 500; i++)
        {
            // Generate a random value to select shape type

            double r = random(0.0, 9.0);

            TransformNodePtr node;

            // Create random shapes based on the random value

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

            // Create a random material for the shape

            auto material = Material::create();

            // Set a random diffuse color for the material

            material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));

            // Set a constant ambient color for the material

            material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

            // Set a random position for the shape in the scene

            node->setPos(glm::vec3(random(-20.0, 20.0), random(-20.0, 20.0), random(-20.0, 20.0)));

            // Assign the material to the shape

            node->setMaterial(material);

            // Add the shape to the scene

            m_scene->add(node);
        }

        // Setup camera manipulator for interactive camera control

        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onUpdate()
    {
        // Update camera manipulator

        m_camManip->update();
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
        // Update the camera manipulator with new window dimensions

        m_camManip->update();
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
