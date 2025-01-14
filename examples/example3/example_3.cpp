#include <iostream>
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
    TextureManagerPtr m_texMgr;
    LightManagerPtr m_lightMgr;

    bool m_showDemoWindow{false};
    float m_blendFactor{1.0};

public:
    ExampleWindow(int width, int height, std::string title)
        : GLFWWindow(width, height, title), m_showDemoWindow(false), m_scene(nullptr), m_camManip(nullptr),
          m_texMgr(nullptr), m_lightMgr(nullptr)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    int onSetup()
    {
        ShaderManagerPtr shaderMgr = ShaderManager::create();
        shaderMgr->loadBasicShader();

        if (shaderMgr->compileLinkErrors())
        {
            cout << "Couldn't compile shaders, exiting..." << endl;
            return -1;
        }

        m_lightMgr = LightManager::create();

        auto pointLight1 = m_lightMgr->addPointLight();
        pointLight1->setEnabled(true);
        pointLight1->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        pointLight1->setAttenuation(1.0, 0.0, 0.0);
        pointLight1->setPosition(glm::vec3(5.0, 5.0, 5.0));
        m_lightMgr->apply();

        m_texMgr = TextureManager::create();
        m_texMgr->setUseTexture(true);
        m_texMgr->apply();

        m_scene = CompositeNode::create();

        auto axis = Axis::create();
        auto grid = Grid::create();
        grid->setType(GridType::LinesAndMarkers);

        auto sphereMaterial = Material::create();
        sphereMaterial->setDiffuseColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        sphereMaterial->setUseTexture(true);
        sphereMaterial->setShininess(100.0);

        auto textureCat = Texture::create();
        textureCat->load("assets/pop_cat.png");

        auto textureBrick = Texture::create();
        textureBrick->load("assets/brick.png");

        auto sphere = Sphere::create();
        sphere->setTexture(textureCat);
        sphere->setMaterial(sphereMaterial);
        sphere->setPos(glm::vec3(0.0, 3.0, 0.0));

        auto box = Box::create();
        box->setTexture(textureBrick);
        box->setMaterial(sphereMaterial);
        box->setPos(glm::vec3(3.0, 0.0, 0.0));

        m_scene->add(box);
        m_scene->add(sphere);

        m_scene->add(axis);
        m_scene->add(grid);

        m_camManip = CameraManipulator::create(this->ref());
        m_camManip->setCameraPosition(glm::vec3(0.0, 0.0, 10.0));

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

    void onKey(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            this->close();
        if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
            m_showDemoWindow = !m_showDemoWindow;

        if (key == '1' && action == GLFW_PRESS)
            m_texMgr->setTextureBlendMode(TextureBlendMode::Normal);
        if (key == '2' && action == GLFW_PRESS)
            m_texMgr->setTextureBlendMode(TextureBlendMode::Multiply);
        if (key == '3' && action == GLFW_PRESS)
            m_texMgr->setTextureBlendMode(TextureBlendMode::Add);
        if (key == '4' && action == GLFW_PRESS)
            m_texMgr->setTextureBlendMode(TextureBlendMode::Screen);
        if (key == '5' && action == GLFW_PRESS)
            m_texMgr->setTextureBlendMode(TextureBlendMode::Overlay);
        if (key == '6' && action == GLFW_PRESS)
            m_texMgr->setTextureBlendMode(TextureBlendMode::Decal);

        if (key == GLFW_KEY_T && action == GLFW_PRESS)
        {
            if (m_texMgr->useTexture())
                m_texMgr->setUseTexture(false);
            else
                m_texMgr->setUseTexture(true);

            m_lightMgr->apply();
        }

        if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        {
            m_blendFactor += 0.1;
            if (m_blendFactor > 1.0)
                m_blendFactor = 1.0;
            m_texMgr->setBlendFactor(m_blendFactor);
        }

        if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        {
            m_blendFactor -= 0.1;
            if (m_blendFactor < 0.0)
                m_blendFactor = 0.0;
            m_texMgr->setBlendFactor(m_blendFactor);
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

    auto window = ExampleWindow::create(800, 800, "Example 3");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
