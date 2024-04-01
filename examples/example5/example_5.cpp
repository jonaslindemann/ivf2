#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

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

    PointLightWindowPtr m_pointLightControl1;
    PointLightWindowPtr m_pointLightControl2;
    DirectionalLightWindowPtr m_dirLightControl;
    SpotLightWindowPtr m_spotLightControl;

    LightManagerPtr m_lightMgr;

    FpsWindowPtr m_fpsWindow;

    bool m_showDemoWindow = false;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWWindow(width, height, title), m_lightMgr(nullptr)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        glEnable(GL_DEPTH_TEST);

        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        m_lightMgr = LightManager::create();
        m_lightMgr->enableLighting();

        auto pointLight1 = m_lightMgr->addPointLight();
        pointLight1->setEnabled(false);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.14, 0.07);
        pointLight1->setPosition(glm::vec3(2.0, 2.0, 2.0));

        auto pointLight2 = m_lightMgr->addPointLight();
        pointLight2->setEnabled(false);
        pointLight2->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight2->setAttenuation(1.0, 0.14, 0.07);
        pointLight2->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight2->setPosition(glm::vec3(-2.0, -2.0, -2.0));

        auto dirLight = m_lightMgr->addDirectionalLight();
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
        dirLight->setEnabled(false);

        auto spotLight = m_lightMgr->addSpotLight();
        spotLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        spotLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        spotLight->setDirection(glm::vec3(0.0, 0.0, -1.0));
        spotLight->setPosition(glm::vec3(0.0, 0.0, 10.0));
        spotLight->setCutoff(12.0f, 13.0f);
        spotLight->setEnabled(true);

        m_lightMgr->apply();

        m_scene = CompositeNode::create();
        auto axis = Axis::create();

        auto texture = Texture::create();
        texture->load("assets/planks.png");

        auto sphere = Sphere::create(0.15);

        for (auto i = 0; i < 11; i++)
            for (auto j = 0; j < 11; j++)
                for (auto k = 0; k < 11; k++)
                {
                    if (true)
                    {
                        auto instSphere = InstanceNode::create();
                        instSphere->setNode(sphere);
                        instSphere->setPos(glm::vec3(-5.0 + i, -5.0 + j, -5.0 + k));

                        auto material = Material::create();
                        material->setDiffuseColor(
                            glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0f));
                        material->setShininess(40.0);

                        instSphere->setMaterial(material);
                        // instSphere->setTexture(texture);
                        m_scene->add(instSphere);
                    }
                    else
                    {
                        auto sphere = Sphere::create(0.15);
                        sphere->setPos(glm::vec3(-5.0 + i, -5.0 + j, -5.0 + k));

                        auto material = Material::create();
                        material->setDiffuseColor(
                            glm::vec4(random(0.0, 1.0), random(0.0, 1.0), random(0.0, 1.0), 1.0f));
                        material->setShininess(40.0);

                        sphere->setMaterial(material);
                        // instSphere->setTexture(texture);
                        m_scene->add(sphere);
                    }
                }

        m_scene->add(axis);

        m_pointLightControl1 = PointLightWindow::create(pointLight1, "pointLight1");
        m_pointLightControl2 = PointLightWindow::create(pointLight2, "pointLight2");
        m_dirLightControl = DirectionalLightWindow::create(dirLight, "dirLight");
        m_spotLightControl = SpotLightWindow::create(spotLight, "spotLight");

        m_fpsWindow = FpsWindow::create();

        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if ((m_pointLightControl1->isDirty()) || (m_pointLightControl2->isDirty()) || (m_dirLightControl->isDirty()) ||
            (m_spotLightControl->isDirty()))
            m_lightMgr->apply();

        m_scene->draw();
    }

    void onDrawUi()
    {
        m_pointLightControl1->draw();
        m_pointLightControl2->draw();
        m_dirLightControl->draw();
        m_spotLightControl->draw();
        m_fpsWindow->draw();
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

    auto window = ExampleWindow::create(800, 800, "Example 5");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
