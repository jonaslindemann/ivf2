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
        GridPtr grid = Grid::create();

        m_scene->add(axis);
        m_scene->add(grid);

        auto yellowMat = Material::create();
        yellowMat->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        auto redMat = Material::create();
        redMat->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        auto line = SolidLine::create(glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 1.0, 0.0), 0.1);
        line->setMaterial(yellowMat);

        auto extrusion = SolidPolyLine::create(0.1);
        extrusion->addPoint(gml::dvec3(0.5, -1.0, -1.0));
        extrusion->addPoint(gml::dvec3(0.5, 1.0, 1.0));
        extrusion->addPoint(gml::dvec3(0.5, 3.0, -1.0));
        extrusion->refresh();
        extrusion->setMaterial(yellowMat);

        auto p1 = Sphere::create();
        p1->setRadius(0.2);
        p1->setPos(glm::vec3(0.5, -1.0, -1.0));
        p1->refresh();
        p1->setMaterial(redMat);

        auto p2 = Sphere::create();
        p2->setRadius(0.2);
        p2->setPos(glm::vec3(0.5, 1.0, 1.0));
        p2->refresh();
        p2->setMaterial(redMat);

        auto p3 = Sphere::create();
        p3->setRadius(0.2);
        p3->setPos(glm::vec3(0.5, 3.0, -1.0));
        p3->refresh();
        p3->setMaterial(redMat);

        auto p4 = Sphere::create();
        p4->setRadius(0.2);
        p4->setPos(glm::vec3(7.0, 1.0, 0.0));
        p4->refresh();
        p4->setMaterial(redMat);

        // m_scene->add(p1);
        // m_scene->add(p2);
        // m_scene->add(p3);
        //  m_scene->add(p4);
        m_scene->add(line);
        m_scene->add(extrusion);

        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->draw();
    }

    void onUpdateOtherUi()
    {
        m_camManip->update();
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

    auto window = ExampleWindow::create(800, 800, "Example 6");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
