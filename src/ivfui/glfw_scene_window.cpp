#include <ivfui/glfw_scene_window.h>

using namespace std;
using namespace ivfui;

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/shader_manager.h>
#include <ivf/font_manager.h>

#include <ivf/stock_shaders.h>

using namespace ivf;

GLFWSceneWindow::GLFWSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor,
                                 GLFWwindow *shared)
    : GLFWWindow(width, height, title, monitor, shared), m_selectionEnabled(false), m_lastNode(nullptr),
      m_currentNode(nullptr), m_renderToTexture(false), m_selectionRendering(false)
{
    m_scene = ivf::CompositeNode::create();
    m_camManip = ivfui::CameraManipulator::create(this->ref());
    m_bufferSelection = ivf::BufferSelection::create(m_scene);
    m_frameBuffer = ivf::FrameBuffer::create(width, height);
    m_postProcessor = ivf::PostProcessor::create(width, height);
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

void ivfui::GLFWSceneWindow::setRenderToTexture(bool renderToTexture)
{
    m_renderToTexture = renderToTexture;
}

bool ivfui::GLFWSceneWindow::renderToTexture()
{
    return m_renderToTexture;
}

void ivfui::GLFWSceneWindow::addUiWindow(ivfui::UiWindowPtr uiWindow)
{
    m_uiWindows.push_back(uiWindow);
}

void ivfui::GLFWSceneWindow::addEffect(ivf::EffectPtr effect)
{
    m_effects.push_back(effect);
    m_postProcessor->addEffect(effect->program());
}

void ivfui::GLFWSceneWindow::clearEffects()
{
    m_effects.clear();
    m_postProcessor->clearEffects();
}

void ivfui::GLFWSceneWindow::enableHeadlight()
{
    auto lightMgr = ivf::LightManager::instance();

    if (lightMgr->dirLightCount() == 0)
    {
        auto dirLight = lightMgr->addDirectionalLight();
        dirLight->setAmbientColor(glm::vec3(0.3, 0.3, 0.3));
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -2.0, -3.0));
        dirLight->setEnabled(true);
        lightMgr->apply();

        m_camManip->setHeadlight(dirLight);
    }
    else
    {
        auto dirLight = lightMgr->getDirectionalLight(0);
        m_camManip->setHeadlight(dirLight);
    }
}

void ivfui::GLFWSceneWindow::disableHeadlight()
{
    m_camManip->setHeadlight(nullptr);
}

void ivfui::GLFWSceneWindow::enableAxis()
{
    m_showAxis = true;
    m_axis->setVisible(true);
}

void ivfui::GLFWSceneWindow::disableAxis()
{
    m_showAxis = false;
    m_axis->setVisible(false);
}

bool ivfui::GLFWSceneWindow::axisEnabled()
{
    return m_showAxis;
}

void ivfui::GLFWSceneWindow::setAxisVisible(bool visible)
{
    m_showAxis = visible;
    m_axis->setVisible(visible);
}

bool ivfui::GLFWSceneWindow::axisVisible()
{
    return m_showAxis;
}

void ivfui::GLFWSceneWindow::enableGrid()
{
    m_showGrid = true;
    m_grid->setVisible(true);
}

void ivfui::GLFWSceneWindow::disableGrid()
{
    m_showGrid = false;
    m_grid->setVisible(false);
}

bool ivfui::GLFWSceneWindow::gridEnabled()
{
    return m_showGrid;
}

void ivfui::GLFWSceneWindow::setGridVisible(bool visible)
{
    m_showGrid = visible;
    m_grid->setVisible(visible);
}

bool ivfui::GLFWSceneWindow::gridVisible()
{
    return m_showGrid;
}

void ivfui::GLFWSceneWindow::setAxisLength(float length)
{
    m_axis->setSize(length);
}

void ivfui::GLFWSceneWindow::setGridTicks(int x, int y, int z)
{
    m_grid->setTicks(x, y, z);
}

void ivfui::GLFWSceneWindow::setGridSpacing(float x, float y, float z)
{
    m_grid->setSpacing(x, y, z);
}

void ivfui::GLFWSceneWindow::showControlPanel()
{
    m_sceneControlPanel->show();
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
    shaderMgr->loadRenderToTextureShader();
    shaderMgr->loadBasicShader();
    shaderMgr->apply();

    if (shaderMgr->compileLinkErrors())
    {
        cout << "Couldn't compile shaders, exiting..." << endl;
        return -1;
    }

    auto lightMgr = ivf::LightManager::create();
    lightMgr->enableLighting();

    auto dirLight = lightMgr->addDirectionalLight();
    dirLight->setAmbientColor(glm::vec3(0.3, 0.3, 0.3));
    dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    dirLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
    dirLight->setDirection(glm::vec3(-1.0, -2.0, -3.0));
    dirLight->setEnabled(true);
    lightMgr->apply();

    m_axis = ivf::Axis::create();
    m_grid = ivf::Grid::create();

    this->add(m_axis);
    this->add(m_grid);

    m_axis->setVisible(m_showAxis);
    m_grid->setVisible(m_showGrid);

    auto retVal = onSetup();

    if (m_selectionEnabled)
        m_bufferSelection->initialize(width(), height());

    m_frameBuffer->initialize();
    m_frameBuffer->setMultisample(true);

    m_postProcessor->initialize();

    smSetCurrentProgram("basic");

    m_sceneControlPanel = ivfui::SceneControlPanel::create("Control panel", this);
    m_sceneControlPanel->hide();

    this->addUiWindow(m_sceneControlPanel);

    return retVal;
}

void GLFWSceneWindow::doResize(int width, int height)
{
    m_camManip->update();
    m_bufferSelection->resize(width, height);
    m_frameBuffer->resize(width, height);
    m_postProcessor->resize(width, height);
    GLFWWindow::doResize(width, height);
}

void GLFWSceneWindow::doUpdateOtherUi()
{
    m_camManip->update();
    GLFWWindow::doUpdateOtherUi();
}

void GLFWSceneWindow::doDraw()
{
    if ((m_renderToTexture) && (!m_selectionRendering))
    {
        m_frameBuffer->resize(width(), height());

        // m_frameBuffer->reset();

        // Draw scene to texture

        m_frameBuffer->begin();

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLFWWindow::doDraw();
        m_scene->draw();

        m_frameBuffer->end();

        // Draw texture to screen

        smApplyProgram("render_to_texture");
        smCurrentProgram()->uniformInt("screenTexture", 0);

        m_frameBuffer->draw();

        this->doUpdateEffects();

        for (auto &effect : m_effects)
            effect->use();

        m_postProcessor->setTime(glfwGetTime());
        m_postProcessor->apply(m_frameBuffer->colorTexture());

        // Reset shader to basic

        smApplyProgram("basic");
    }
    else
    {
        GLFWWindow::doDraw();

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->draw();
    }

    for (auto &uiWindow : m_uiWindows)
        uiWindow->update();
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
        m_selectionRendering = true;

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
    m_selectionRendering = false;

    GLFWWindow::doDrawComplete();
}

void ivfui::GLFWSceneWindow::doKey(int key, int scancode, int action, int mods)
{
#ifdef IVF_DEBUG
    std::cout << "Key: " << key << " Scancode: " << scancode << " Action: " << action << " Mods: " << mods << std::endl;
#endif

    if (key == GLFW_KEY_KP_DIVIDE)
    {
        this->showControlPanel();
    }
    GLFWWindow::doKey(key, scancode, action, mods);
}

void ivfui::GLFWSceneWindow::onUpdateUi()
{}

void ivfui::GLFWSceneWindow::onUpdateEffects()
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

void ivfui::GLFWSceneWindow::doUpdateEffects()
{
    this->onUpdateEffects();
}
