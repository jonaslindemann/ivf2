#include <ivfui/glfw_scene_window.h>

using namespace ivfui;
using namespace std;
using namespace ivfui;

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
