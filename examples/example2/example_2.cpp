#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
    {
    }

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        glEnable(GL_DEPTH_TEST);

        auto shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        if (shaderMgr->compileLinkErrors()) {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        auto lightMgr = LightManager::create();

        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.0, 0.0);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));
        lightMgr->apply();

        m_scene = CompositeNode::create();

        auto axis = Axis::create();
        auto grid = Grid::create();

        m_scene->add(axis);
        m_scene->add(grid);

        for (auto i = 0; i < 500; i++) {
            double r = random(0.0, 9.0);

            TransformNodePtr node;

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

            auto material = Material::create();
            material->setDiffuseColor(glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0));
            material->setAmbientColor(glm::vec4(0.1, 0.1, 0.1, 1.0));

            node->setPos(glm::vec3(random(-20.0, 20.0), random(-20.0, 20.0), random(-20.0, 20.0)));
            node->setMaterial(material);

            m_scene->add(node);
        }

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
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = ExampleWindow::create(800, 800, "Example 2");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
