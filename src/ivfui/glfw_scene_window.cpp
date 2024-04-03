#include <ivfui/glfw_scene_window.h>

using namespace std;
using namespace ivfui;

#include <ivf/gl.h>
#include <ivf/nodes.h>

GLFWSceneWindow::GLFWSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor,
                                 GLFWwindow *shared)
    : GLFWWindow(width, height, title, monitor, shared)
{
    m_scene = ivf::CompositeNode::create();
    m_camManip = ivfui::CameraManipulator::create(this->ref());
}

GLFWSceneWindow::~GLFWSceneWindow()
{}

std::shared_ptr<GLFWSceneWindow> GLFWSceneWindow::create(int width, int height, const std::string title,
                                                         GLFWmonitor *monitor, GLFWwindow *shared)
{
    return std::make_shared<GLFWSceneWindow>(width, height, title, monitor, shared);
}

void ivfui::GLFWSceneWindow::add(ivf::NodePtr node)
{
    m_scene->add(node);
}

void ivfui::GLFWSceneWindow::remove(ivf::NodePtr node)
{
    m_scene->remove(node);
}

void ivfui::GLFWSceneWindow::clear()
{
    m_scene->clear();
}

void ivfui::GLFWSceneWindow::addUiWindow(ivfui::UiWindowPtr uiWindow)
{
    m_uiWindows.push_back(uiWindow);
}

ivf::CompositeNodePtr ivfui::GLFWSceneWindow::scene()
{
    return m_scene;
}

ivfui::CameraManipulatorPtr ivfui::GLFWSceneWindow::cameraManipulator()
{
    return m_camManip;
}

int ivfui::GLFWSceneWindow::onSetup()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    auto fontMgr = ivf::FontManager::create();
    fontMgr->loadFace("fonts/Gidole-Regular.ttf", "gidole");

    auto shaderMgr = ivf::ShaderManager::create();
    shaderMgr->loadBasicShader();

    if (shaderMgr->compileLinkErrors())
    {
        cout << "Couldn't compile shaders, exiting..." << endl;
        return -1;
    }

    auto lightMgr = ivf::LightManager::create();
    lightMgr->enableLighting();

    auto dirLight = lightMgr->addDirectionalLight();
    dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
    dirLight->setEnabled(true);
    lightMgr->apply();

    this->onSceneSetup();

    return 0;
}

void GLFWSceneWindow::onResize(int width, int height)
{
    GLFWWindow::onResize(width, height);
    m_camManip->update();
}

void GLFWSceneWindow::onUpdateOtherUi()
{
    m_camManip->update();
}

void GLFWSceneWindow::onDraw()
{
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_scene->draw();
}

void ivfui::GLFWSceneWindow::onDrawUi()
{
    for (auto uiWindow : m_uiWindows)
        uiWindow->draw();

    this->onUpdateUi();
}

void ivfui::GLFWSceneWindow::onUpdateUi()
{}

void ivfui::GLFWSceneWindow::onSceneSetup()
{}
