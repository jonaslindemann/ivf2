/**
 * @file ui1.cpp
 * @brief UI example with multiple transforms
 * @author Jonas Lindemann
 * @example ui1.cpp
 * @ingroup ui_examples
 *
 * This example demonstrates the use of the IVF library to create a scene with
 * various geometric shapes, a camera manipulator, and multiple UI windows.
 * It includes a main window with a scene containing randomly positioned and colored shapes,
 * two example windows for additional UI functionality,
 * and a camera manipulator for navigating the scene.
 */


#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include "example_window_1.h"
#include "example_window_2.h"

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWWindow {
private:
    CompositeNodePtr m_scene;
    CameraManipulatorPtr m_camManip;

    ExampleWindow1Ptr m_exampleWindow1 = ExampleWindow1::create();
    ExampleWindow2Ptr m_exampleWindow2 = ExampleWindow2::create();

    bool m_showDemoWindow = false;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        // Create and initialize the shader manager, then load the basic shader.
        
        auto shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        // Check for shader compilation or linking errors.
        
        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Create the light manager.
        
        auto lightMgr = LightManager::create();

        // Add and configure a point light.
        
        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(true);

        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));

        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));

        pointLight1->setAttenuation(1.0, 0.0, 0.0);

        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));

        lightMgr->apply();

        // Create the main scene composite node.
        
        m_scene = CompositeNode::create();

        // Add an axis to the scene.
        
        auto axis = Axis::create();

        m_scene->add(axis);

        // Add 500 randomly generated shapes to the scene.
        
        for (auto i = 0; i < 500; i++)
        {
            // Generate a random number to select the shape type.
            
            double r = random(0.0, 9.0);

            TransformNodePtr node;

            // Create a random geometric shape based on the value of r.
            
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

            // Create and set a random material for the shape.
            
            auto material = Material::create();
            material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));

            material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

            // Set a random position for the shape.
            
            node->setPos(glm::vec3(random(-20.0, 20.0), random(-20.0, 20.0), random(-20.0, 20.0)));

            node->setMaterial(material);

            // Add the shape to the scene.
            
            m_scene->add(node);
        }

        // Create the camera manipulator for scene navigation.
        
        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->draw();
    }

    void onUpdate()
    {
        m_exampleWindow1->update();
        m_exampleWindow2->update();
    }

    void onUpdateOtherUi()
    {
        m_camManip->update();
    }

    void onDrawUi()
    {
        m_exampleWindow1->draw();
        m_exampleWindow2->draw();

        if (m_showDemoWindow)
            ImGui::ShowDemoWindow(&m_showDemoWindow);

        if (m_exampleWindow1->demoWindowButton())
            m_showDemoWindow = true;

        if (m_exampleWindow1->anotherWindowButton())
            m_exampleWindow2->show();

        if (m_exampleWindow2->closeMeButton())
            m_exampleWindow2->hide();
    }

    void onResize(int width, int height)
    {
        m_camManip->update();
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

    auto window = ExampleWindow::create(800, 800, "User interface");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
