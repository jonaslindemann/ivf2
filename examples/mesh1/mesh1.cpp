#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
#include <numbers>

#include <ivf/gl.h>
#include <ivf/nodes.h>

#include <ivf/twist_deformer.h>
#include <ivf/bend_deformer.h>
#include <ivf/deformable_mesh_node.h>
#include <ivf/deformable_primitive.h>

#include <ivfui/ui.h>

#include "twist_window.h"

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    PlanePtr m_plane;
    RoundedBoxPtr m_box;
    VerticesPtr m_vertices;
    IndicesPtr m_indices;
    std::shared_ptr<DeformablePrimitive<RoundedBox>> m_deformableCube;
    TwistDeformerPtr m_twistDeformer;
    TwistWindowPtr m_twistWindow;

    float m_angle{0.0f};          // Twist angle in radians
    float m_falloff{1.0};         // Distance falloff factor
    float m_startDistance{0.0};   // Distance where twist starts
    float m_endDistance{1.0};     // Distance where twist ends
    bool m_wireframe{true};       // Wireframe mode
    float m_angleIncrement{0.5f}; // Angle increment for animation
public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        auto axis = Axis::create();

        this->add(axis);

        auto redMaterial = Material::create();
        redMaterial->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        auto greenMaterial = Material::create();
        greenMaterial->setDiffuseColor(glm::vec4(0.0, 1.0, 0.0, 1.0));

        auto blueMaterial = Material::create();
        blueMaterial->setDiffuseColor(glm::vec4(0.0, 0.0, 1.0, 1.0));

        auto yellowMaterial = Material::create();
        yellowMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto whiteMaterial = Material::create();
        whiteMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

        mmDefaultMeshUsage(GL_DYNAMIC_DRAW);

        m_deformableCube = DeformablePrimitive<RoundedBox>::create();
        m_deformableCube->primitive()->setSize(glm::vec3(1.0f, 4.0f, 1.0f));
        m_deformableCube->primitive()->setSegments(glm::uvec3(10, 40, 10));
        m_deformableCube->refresh(); // Update geometry and deformer setup
        m_deformableCube->setMaterial(whiteMaterial);
        m_deformableCube->setWireframe(true);

        // Add deformers
        m_twistDeformer = TwistDeformer::create(glm::vec3(0, 1, 0));
        m_twistDeformer->setAngle(glm::radians(m_angle));
        m_twistDeformer->setFalloff(m_falloff);
        m_twistDeformer->setDistanceRange(m_startDistance, m_endDistance);

        m_deformableCube->addDeformer(m_twistDeformer);

        // Use like any other MeshNode
        m_deformableCube->applyDeformers();

        this->add(m_deformableCube);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0, 5, 20));

        m_twistWindow = TwistWindow::create();
        this->addUiWindow(m_twistWindow);

        return 0;
    }

    virtual void onUpdate()
    {
        // Update twist parameters from the UI window

        m_angle = m_twistWindow->angle();
        m_falloff = m_twistWindow->falloff();
        m_startDistance = m_twistWindow->startDistance();
        m_endDistance = m_twistWindow->endDistance();
        m_wireframe = m_twistWindow->wireframe();
        m_deformableCube->setWireframe(m_wireframe);

        m_twistDeformer->setAngle(glm::radians(m_angle));
        m_twistDeformer->setFalloff(m_falloff);
        m_twistDeformer->setDistanceRange(m_startDistance, m_endDistance);
        m_twistDeformer->setCenter(
            glm::vec3(m_twistWindow->center()[0], m_twistWindow->center()[1], m_twistWindow->center()[2]));

        m_twistDeformer->setAxis(
            glm::vec3(m_twistWindow->axis()[0], m_twistWindow->axis()[1], m_twistWindow->axis()[2]));

        m_deformableCube->applyDeformers();

        if (std::abs(m_angle) >= 45.0f)
        {
            m_angleIncrement = -m_angleIncrement; // Reverse direction after a full twist
        }
    }

    virtual void onKey(int key, int scancode, int action, int mods) override
    {
        if (key == GLFW_KEY_W && action == GLFW_PRESS)
        {
            m_wireframe = !m_wireframe;
            m_deformableCube->setWireframe(m_wireframe);
        }
        else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            this->close();
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

    auto window = ExampleWindow::create(800, 800, "Animation");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
