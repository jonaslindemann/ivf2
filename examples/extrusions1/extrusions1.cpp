/**
 * @file extrusions1.cpp
 * @brief Extrusions example
 * @author Jonas Lindemann
 * @example extrusions1.cpp
 * @ingroup primitives_examples
 *
 * This example demonstrates the creation and rendering of extrusions
 * using solid lines and solid polylines. It sets up a basic scene with
 * an axis, grid, and two extrusion shapes: a solid line and a solid polyline.
 */

#include <iostream>
#include <memory>
#include <vector>

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
        // Enable depth testing for correct 3D rendering

        glEnable(GL_DEPTH_TEST);

        // Create and load font face for UI rendering

        auto fontMgr = FontManager::create();
        fontMgr->loadFace("fonts/Gidole-Regular.ttf", "gidole");

        // Create and load basic shader for rendering

        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        // Check for shader compilation/linking errors

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        // Create and enable lighting manager

        auto lightMgr = LightManager::create();
        lightMgr->enableLighting();

        // Add and configure a directional light

        auto dirLight = lightMgr->addDirectionalLight();
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
        dirLight->setEnabled(true);

        // Apply lighting settings

        lightMgr->apply();

        // Create the scene graph root node

        m_scene = CompositeNode::create();

        // Add axis and grid to the scene

        AxisPtr axis = Axis::create();
        GridPtr grid = Grid::create();

        m_scene->add(axis);
        m_scene->add(grid);

        // Create yellow and red materials

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        // Create a solid yellow line and add to the scene

        auto line = SolidLine::create(glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 1.0, 0.0), 0.1);
        line->setMaterial(yellowMat);

        // Create a solid yellow polyline extrusion and add to the scene

        auto extrusion = SolidPolyLine::create(0.1);
        extrusion->addPoint(gml::dvec3(0.5, -1.0, -1.0));
        extrusion->addPoint(gml::dvec3(0.5, 1.0, 1.0));
        extrusion->addPoint(gml::dvec3(0.5, 3.0, -1.0));
        extrusion->refresh();
        extrusion->setMaterial(yellowMat);

        m_scene->add(line);
        m_scene->add(extrusion);

        // Create camera manipulator for interactive view control

        m_camManip = CameraManipulator::create(this->ref());

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

    void onUpdateOtherUi()
    {
        // Update the camera manipulator state

        m_camManip->update();
    }

    void onResize(int width, int height)
    {
        // Update the camera manipulator with new window dimensions

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

    auto window = ExampleWindow::create(800, 800, "Extrusions");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
