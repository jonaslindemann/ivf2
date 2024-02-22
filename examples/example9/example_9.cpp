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

class ExampleWindow : public GLFWWindow {
private:
    CompositeNodePtr m_scene;
    CameraManipulatorPtr m_camManip;

    CubePtr m_cube;
    SpherePtr m_sphere;
    TransformPtr m_sphereXfm;

public:
    ExampleWindow(int width, int height, std::string title) 
        : GLFWWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        glEnable(GL_DEPTH_TEST);

        auto fontMgr = FontManager::create();
        fontMgr->loadFace("fonts/Gidole-Regular.ttf", "gidole");

        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        auto lightMgr = LightManager::create();
        lightMgr->enableLighting();

        auto dirLight = lightMgr->addDirectionalLight();
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
        dirLight->setEnabled(true);
        lightMgr->apply();

        m_scene = CompositeNode::create();

        AxisPtr axis = Axis::create();

        m_scene->add(axis);

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        m_cube = Cube::create();
        m_cube->setMaterial(redMat);
        
        m_sphere = Sphere::create();
        m_sphere->setMaterial(yellowMat);
        m_sphere->setRadius(0.25);
        m_sphere->refresh();
        m_sphere->setPos(glm::vec3(2.0, 0.0, 0.0));

        m_sphereXfm = Transform::create();
        m_sphereXfm->add(m_sphere); 

        m_scene->add(m_cube);
        m_scene->add(m_sphereXfm);



        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_cube->setRotAxis(glm::vec3(1.0, 1.0, 1.0));
        m_cube->setRotAngle(20.0*elapsedTime());

        m_sphereXfm->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
        m_sphereXfm->setRotAngle(20.0*elapsedTime());

        m_scene->draw();
    }

    void onUpdateOtherUi()
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

    auto window = ExampleWindow::create(800, 800, "Example 6");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
