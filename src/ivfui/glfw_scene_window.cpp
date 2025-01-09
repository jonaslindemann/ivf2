#include <ivfui/glfw_scene_window.h>

using namespace std;
using namespace ivfui;

#include <ivf/gl.h>
#include <ivf/nodes.h>

GLFWSceneWindow::GLFWSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor,
                                 GLFWwindow *shared)
    : GLFWWindow(width, height, title, monitor, shared), m_selectionEnabled(false), m_lastNode(nullptr),
      m_currentNode(nullptr)
{
    m_scene = ivf::CompositeNode::create();
    m_camManip = ivfui::CameraManipulator::create(this->ref());
    m_bufferSelection = ivf::BufferSelection::create(m_scene);
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

void ivfui::GLFWSceneWindow::setSelectionEnabled(bool enabled)
{
    m_selectionEnabled = enabled;
}

bool ivfui::GLFWSceneWindow::selectionEnabled()
{
    return m_selectionEnabled;
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

int ivfui::GLFWSceneWindow::doSetup()
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

    auto retVal = onSetup();

    if (m_selectionEnabled)
        m_bufferSelection->initialize(width(), height());

    return retVal;
}

void GLFWSceneWindow::doResize(int width, int height)
{
    m_camManip->update();
    m_bufferSelection->resize(width, height);
    GLFWWindow::doResize(width, height);
}

void GLFWSceneWindow::doUpdateOtherUi()
{
    m_camManip->update();
    GLFWWindow::doUpdateOtherUi();
}

void GLFWSceneWindow::doDraw()
{
    GLFWWindow::doDraw();

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_scene->draw();
}

void ivfui::GLFWSceneWindow::doDrawUi()
{
    for (auto uiWindow : m_uiWindows)
        uiWindow->draw();

    GLFWWindow::doDrawUi();
    this->doUpdateUi();
}

void ivfui::GLFWSceneWindow::doDrawComplete()
{
    if (m_selectionEnabled)
    {
        m_bufferSelection->begin();
        this->drawScene();
        auto m_currentNode = m_bufferSelection->nodeAtPixel(mouseX(), mouseY());

        if (m_currentNode != nullptr)
        {
            if (m_currentNode != m_lastNode)
            {
                if (m_lastNode != nullptr)
                    this->doLeaveNode(m_lastNode);
                this->doEnterNode(m_currentNode);
                m_lastNode = m_currentNode;
            }
            else
            {
                this->doOverNode(m_currentNode);
            }
        }
        else
        {
            if (m_lastNode != nullptr)
            {
                this->doLeaveNode(m_lastNode);
                m_lastNode = nullptr;
            }
        }

        m_bufferSelection->end();
    }
    GLFWWindow::doDrawComplete();
}

void ivfui::GLFWSceneWindow::onUpdateUi()
{}

void ivfui::GLFWSceneWindow::onEnterNode(ivf::Node *node)
{}

void ivfui::GLFWSceneWindow::onOverNode(ivf::Node *node)
{}

void ivfui::GLFWSceneWindow::onLeaveNode(ivf::Node *node)
{}

void ivfui::GLFWSceneWindow::doEnterNode(ivf::Node *node)
{
    this->onEnterNode(node);
}

void ivfui::GLFWSceneWindow::doOverNode(ivf::Node *node)
{
    this->onOverNode(node);
}

void ivfui::GLFWSceneWindow::doLeaveNode(ivf::Node *node)
{
    this->onLeaveNode(node);
}

void ivfui::GLFWSceneWindow::doUpdateUi()
{
    this->onUpdateUi();
}
