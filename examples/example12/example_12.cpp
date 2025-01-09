#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivf/node_visitor.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CubePtr m_cube;
    SpherePtr m_sphere;
    TransformPtr m_sphereXfm;
    BufferSelectionPtr m_bufferSelection;
    SelectionManagerPtr m_selectionMgr;
    MaterialPtr m_selectionMaterial;
    MaterialPtr m_yellowMat;
    Node *m_lastNode;
    Node *m_currentNode;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual void onSceneSetup() override
    {
        auto shaderMgr = ShaderManager::instance();
        m_selectionMgr = SelectionManager::instance();
        m_bufferSelection = BufferSelection::create(this->scene());

        m_lastNode = nullptr;

        // m_selectionMgr->setSelectionRendering(true);

        AxisPtr axis = Axis::create();

        this->add(axis);

        m_yellowMat = Material::create();
        m_yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        m_selectionMaterial = Material::create();
        m_selectionMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

        for (auto row = -3; row < 4; row++)
        {
            for (auto col = -3; col < 4; col++)
            {
                for (auto layer = -3; layer < 4; layer++)
                {
                    auto sph1 = Sphere::create();
                    sph1->setMaterial(m_yellowMat);
                    sph1->setRadius(0.5);
                    sph1->refresh();
                    sph1->setPos(glm::vec3(row * 3, col * 3, layer * 3));
                    this->add(sph1);
                }
            }
        }

        m_bufferSelection->initialize(width(), height());

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, 0.0, 20.0));
    }

    virtual void onDrawComplete() override
    {
        m_bufferSelection->begin();
        this->drawScene();

        auto m_currentNode = m_bufferSelection->nodeAtPixel(mouseX(), mouseY());

        if ((m_currentNode != m_lastNode))
        {
            if (m_lastNode != nullptr)
                m_lastNode->setMaterial(m_yellowMat);
            if (m_currentNode != nullptr)
            {
                m_currentNode->setMaterial(m_selectionMaterial);
                m_lastNode = m_currentNode;
            }
        }
        m_bufferSelection->end();
    }

    virtual void onUpdate()
    {}

    virtual void onResize(int width, int height) override
    {
        m_bufferSelection->resize(width, height);
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

    auto window = ExampleWindow::create(800, 800, "Example 6");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
