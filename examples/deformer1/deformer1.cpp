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
#include <ivfui/object_inspector.h>

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
    BendDeformerPtr m_bendDeformer;

    ObjectInspectorPtr m_twistInspector;
    ObjectInspectorPtr m_bendInspector;

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

        m_bendDeformer = BendDeformer::create(glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));
        m_bendDeformer->setCurvature(2.5f);
        m_bendDeformer->setDistanceRange(-4.0f, 4.0f);
        m_bendDeformer->setAxis(glm::vec3(0, 1, 0));
        m_bendDeformer->setCenter(glm::vec3(0, 0, 0));

        m_deformableCube->addDeformer(m_twistDeformer);
        m_deformableCube->addDeformer(m_bendDeformer);

        // Use like any other MeshNode
        m_deformableCube->applyDeformers();

        this->add(m_deformableCube);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0, 5, 20));

        m_twistInspector = ObjectInspector::create("Twist");
        m_twistInspector->setObject(m_twistDeformer);
        m_twistInspector->setVisible(true);

        this->addUiWindow(m_twistInspector);

        m_bendInspector = ObjectInspector::create("Bend");
        m_bendInspector->setObject(m_bendDeformer);
        m_bendInspector->setVisible(true);

        this->addUiWindow(m_bendInspector);

        return 0;
    }

    virtual void onUpdate()
    {
        // Update twist parameters from the UI window

        m_deformableCube->applyDeformers();
    }

    virtual void onKey(int key, int scancode, int action, int mods) override
    {
        if (key == GLFW_KEY_W && action == GLFW_PRESS)
        {
            m_deformableCube->primitive()->mesh()->setWireframe(!m_deformableCube->primitive()->mesh()->wireframe());
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
