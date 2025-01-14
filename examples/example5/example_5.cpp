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

class ExampleWindow : public GLFWSceneWindow {
private:
    PointLightWindowPtr m_pointLightControl1;
    PointLightWindowPtr m_pointLightControl2;
    DirectionalLightWindowPtr m_dirLightControl;
    SpotLightWindowPtr m_spotLightControl;

    FpsWindowPtr m_fpsWindow;

    bool m_showDemoWindow = false;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        auto lightMgr = LightManager::instance();

        lightMgr->clearLights();

        auto pointLight1 = lightMgr->addPointLight();
        pointLight1->setEnabled(false);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.14, 0.07);
        pointLight1->setPosition(glm::vec3(2.0, 2.0, 2.0));

        auto pointLight2 = lightMgr->addPointLight();
        pointLight2->setEnabled(false);
        pointLight2->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight2->setAttenuation(1.0, 0.14, 0.07);
        pointLight2->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight2->setPosition(glm::vec3(-2.0, -2.0, -2.0));

        auto dirLight = lightMgr->addDirectionalLight();
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
        dirLight->setEnabled(false);

        auto spotLight = lightMgr->addSpotLight();
        spotLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        spotLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        spotLight->setDirection(glm::vec3(0.0, 0.0, -1.0));
        spotLight->setPosition(glm::vec3(0.0, 0.0, 10.0));
        spotLight->setCutoff(12.0f, 13.0f);
        spotLight->setEnabled(true);

        lightMgr->apply();

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
                        this->add(instSphere);
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
                        this->add(sphere);
                    }
                }

        this->add(axis);

        m_pointLightControl1 = PointLightWindow::create(pointLight1, "pointLight1");
        m_pointLightControl2 = PointLightWindow::create(pointLight2, "pointLight2");
        m_dirLightControl = DirectionalLightWindow::create(dirLight, "dirLight");
        m_spotLightControl = SpotLightWindow::create(spotLight, "spotLight");

        this->addUiWindow(m_pointLightControl1);
        this->addUiWindow(m_pointLightControl2);
        this->addUiWindow(m_dirLightControl);
        this->addUiWindow(m_spotLightControl);

        m_fpsWindow = FpsWindow::create();

        this->addUiWindow(m_fpsWindow);

        return 0;
    }

    void onUpdateUi()
    {
        if ((m_pointLightControl1->isDirty()) || (m_pointLightControl2->isDirty()) || (m_dirLightControl->isDirty()) ||
            (m_spotLightControl->isDirty()))
            LightManager::instance()->apply();
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
