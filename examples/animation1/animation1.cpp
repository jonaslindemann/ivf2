/**
 * @file animation1.cpp
 * @brief Simple animation example
 * @author Jonas Lindemann
 * @example animation1.cpp
 * @ingroup animation_examples
 *
 * This example demonstrates basic animation techniques with the Interactive Visual Framework 2.
 * It creates a rotating cube and a sphere orbiting around the origin.
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CubePtr m_cube;            
    SpherePtr m_sphere;        
    TransformPtr m_sphereXfm;  

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        // Create and add coordinate system axis

        AxisPtr axis = Axis::create();
        this->add(axis);

        // Setup materials

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));  // Yellow material

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));  // Red material

        // Create a static yellow sphere at the origin (visual reference)

        auto sph1 = Sphere::create();
        sph1->setMaterial(yellowMat);
        sph1->setRadius(0.5);
        sph1->refresh();
        sph1->setPos(glm::vec3(0.0, 0.0, 0.0));
        
        // Setup rotating cube

        m_cube = Cube::create();
        m_cube->setMaterial(redMat);

        // Setup orbiting sphere

        m_sphere = Sphere::create();
        m_sphere->setMaterial(yellowMat);
        m_sphere->setRadius(0.25);
        m_sphere->refresh();
        m_sphere->setPos(glm::vec3(2.0, 0.0, 0.0));  // Position 2 units from origin

        // Transform node to control the sphere's orbit

        m_sphereXfm = Transform::create();
        m_sphereXfm->add(m_sphere);

        // Add objects to the scene
        
        this->add(m_cube);
        this->add(m_sphereXfm);

        return 0;
    }

    virtual void onUpdate()
    {
        // Updates the animation for each frame

        // Rotate the cube around a skewed axis (1,1,1)
        
        m_cube->setRotAxis(glm::vec3(1.0, 1.0, 1.0));
        m_cube->setRotAngle(20.0 * elapsedTime());  // 20 degrees per second

        // Orbit the sphere around the Y-axis
        
        m_sphereXfm->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
        m_sphereXfm->setRotAngle(20.0 * elapsedTime());  // 20 degrees per second
    }
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    // Create the GLFW application

    auto app = GLFWApplication::create();

    // Configure OpenGL context

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);  // Enable multisampling for anti-aliasing

    // Create and configure the window

    auto window = ExampleWindow::create(800, 800, "Animation");
    window->maximize();

    // Add window to application and start the main loop

    app->addWindow(window);
    return app->loop();
}
