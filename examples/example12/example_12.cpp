#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include <ivf/node_visitor.h>
#include <ivf/stock_shaders.h>
#include <ivf/blur_effect.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    CubePtr m_cube;
    SpherePtr m_sphere;
    TransformPtr m_sphereXfm;
    MaterialPtr m_yellowMat;
    MaterialPtr m_selectionMaterial;

    BlurEffectPtr m_blurEffect;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        this->setSelectionEnabled(true);
        this->setRenderToTexture(true);

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

        m_blurEffect = BlurEffect::create();
        m_blurEffect->setBlurRadius(2.0);
        m_blurEffect->load();

        this->addEffect(m_blurEffect);

        this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, 0.0, 20.0));

        return 0;
    }

    virtual void onEnterNode(Node *node) override
    {
        std::cout << "Enter node" << std::endl;
        node->setMaterial(m_selectionMaterial);
    }

    virtual void onOverNode(Node *node) override
    {
        std::cout << "Over node" << std::endl;
    }

    virtual void onLeaveNode(Node *node) override
    {
        std::cout << "Leave node" << std::endl;
        node->setMaterial(m_yellowMat);
    }

    virtual void onUpdate()
    {}

    virtual void onUpdateEffects()
    {
        m_blurEffect->setBlurRadius(10.0 + 10.0 * sin(glfwGetTime()));
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
    // window->maximize();

    app->addWindow(window);
    return app->loop();
}
