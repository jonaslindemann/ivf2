#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

#include "text_window.h"

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWWindow {
private:
    CompositeNodePtr m_scene;
    CameraManipulatorPtr m_camManip;

    FpsWindowPtr m_fpsWindow;
    TextWindowPtr m_textWindow;

    bool m_showDemoWindow = false;

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

        auto material = Material::create();
        material->setDiffuseColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        material->setShininess(40.0);

        m_scene = CompositeNode::create();
        auto axis = Axis::create();

        auto text = TextNode::create();
        text->setText("Ivf++ 2.0");
        text->setAlignX(TextAlignX::CENTER);
        text->setAlignY(TextAlignY::MIDDLE);

        auto texture = Texture::create();
        texture->load("assets/planks.png");

        text->setMaterial(material);

        m_scene->add(axis);
        m_scene->add(text);

        m_fpsWindow = FpsWindow::create();
        m_textWindow = TextWindow::create(text);

        m_camManip = CameraManipulator::create(this->ref());

        return 0;
    }

    void onDraw()
    {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->draw();
    }

    void onDrawUi()
    {
        m_fpsWindow->draw();
        m_textWindow->draw();
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
#ifdef __APPLE__
    app->hint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    auto window = ExampleWindow::create(800, 800, "Example 6");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
