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
      m_currentNode(nullptr), m_renderToTexture(false), m_selectionRendering(false), m_showAxis(false),
      m_showGrid(false)
{
    m_scene = ivf::CompositeNode::create();
    m_camManip = ivfui::CameraManipulator::create(this->ref());
    m_bufferSelection = ivf::BufferSelection::create(m_scene);
    m_frameBuffer = ivf::FrameBuffer::create(width, height);
    m_postProcessor = ivf::PostProcessor::create(width, height);
    m_mainMenu = ivfui::UiMainMenu::create();
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

// EffectListProvider interface implementation

int GLFWSceneWindow::getEffectCount() const
{
    return static_cast<int>(m_effects.size());
}

std::shared_ptr<ivf::Effect> GLFWSceneWindow::getEffect(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_effects.size()))
    {
        return nullptr;
    }
    return m_effects[index];
}

void GLFWSceneWindow::enableEffect(int index)
{
    if (index < 0 || index >= static_cast<int>(m_effects.size()))
    {
        cout << "Invalid effect index: " << index << endl;
        return;
    }

    auto effect = m_effects[index];
    if (effect)
    {
        effect->program()->setEnabled(true);
    }
    else
    {
        cout << "Effect at index " << index << " is null." << endl;
    }
}

void GLFWSceneWindow::disableEffect(int index)
{
    if (index < 0 || index >= static_cast<int>(m_effects.size()))
    {
        cout << "Invalid effect index: " << index << endl;
        return;
    }

    auto effect = m_effects[index];
    if (effect)
    {
        effect->program()->setEnabled(false);
    }
    else
    {
        cout << "Effect at index " << index << " is null." << endl;
    }
}

bool GLFWSceneWindow::isEffectEnabled(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_effects.size()))
    {
        cout << "Invalid effect index: " << index << endl;
        return false;
    }
    auto effect = m_effects[index];
    if (effect)
    {
        return effect->program()->enabled();
    }
    else
    {
        cout << "Effect at index " << index << " is null." << endl;
        return false;
    }
}

void GLFWSceneWindow::disableAllEffects()
{
    for (auto &effect : m_effects)
    {
        if (effect)
            effect->program()->setEnabled(false);
        else
        {
            cout << "Effect is null." << endl;
        }
    }
}

void GLFWSceneWindow::clearEffects()
{
    m_effects.clear();
    m_postProcessor->clearEffects();
}

bool GLFWSceneWindow::reorderEffect(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= static_cast<int>(m_effects.size()) || toIndex < 0 ||
        toIndex >= static_cast<int>(m_effects.size()) || fromIndex == toIndex)
    {
        return false;
    }

    // Reorder in the effects vector
    auto effect = m_effects[fromIndex];
    m_effects.erase(m_effects.begin() + fromIndex);
    m_effects.insert(m_effects.begin() + toIndex, effect);

    // Rebuild the post-processor with the new order
    m_postProcessor->clearEffects();
    for (const auto &eff : m_effects)
    {
        if (eff)
        {
            m_postProcessor->addEffect(eff->program());
        }
    }

    return true;
}

// Legacy effect interface methods (for backward compatibility)

ivf::EffectPtr GLFWSceneWindow::effect(int index)
{
    return getEffect(index);
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
        m_camManip->setHeadlight(dirLight);
        lightMgr->apply();
    }
    else
    {
        auto dirLight = lightMgr->getDirectionalLight(0);
        dirLight->setAmbientColor(glm::vec3(0.3, 0.3, 0.3));
        dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
        dirLight->setDirection(glm::vec3(-1.0, -2.0, -3.0));
        dirLight->setEnabled(true);
        m_camManip->setHeadlight(dirLight);
        lightMgr->apply();
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

void ivfui::GLFWSceneWindow::resetView()
{
    m_camManip->reset();
}

void ivfui::GLFWSceneWindow::saveView()
{
    m_camManip->saveState();
}

void ivfui::GLFWSceneWindow::showControlPanel()
{
    m_sceneControlPanel->show();
}

void ivfui::GLFWSceneWindow::showCameraWindow()
{
    m_cameraWindow->show();
}

void ivfui::GLFWSceneWindow::showEffectInspector()
{
    m_effectInspector->show();
}

void ivfui::GLFWSceneWindow::showMainMenu()
{
    m_showMainMenu = true;
}

void ivfui::GLFWSceneWindow::hideMainMenu()
{
    m_showMainMenu = false;
}

bool ivfui::GLFWSceneWindow::isMainMenuVisible() const
{
    return m_showMainMenu;
}

ivf::CompositeNodePtr ivfui::GLFWSceneWindow::scene()
{
    return m_scene;
}

ivfui::CameraManipulatorPtr ivfui::GLFWSceneWindow::cameraManipulator()
{
    return m_camManip;
}

ivfui::UiMainMenu *ivfui::GLFWSceneWindow::mainMenu()
{
    return m_mainMenu.get();
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

    if (m_selectionEnabled)
        m_bufferSelection->initialize(width(), height());

    m_frameBuffer->initialize();
    m_frameBuffer->setMultisample(true);

    m_postProcessor->initialize();

    smSetCurrentProgram("basic");

    m_sceneControlPanel = ivfui::SceneControlPanel::create("Control panel", this);
    m_sceneControlPanel->hide();

    this->addUiWindow(m_sceneControlPanel);

    m_cameraWindow = ivfui::CameraWindow::create(m_camManip, "Camera");
    m_cameraWindow->hide();

    this->addUiWindow(m_cameraWindow);

    // Create EffectInspector with the new interface, passing 'this' as EffectListProvider
    m_effectInspector = ivfui::EffectInspector::create("Effect Inspector", this);
    m_effectInspector->hide();

    this->addUiWindow(m_effectInspector);

    auto retVal = onSetup();
    doSetupMainMenu();

    return retVal;
}

void ivfui::GLFWSceneWindow::doSetupMainMenu()
{
    if (!m_mainMenu)
        return;

    // Clear existing menus if any

    m_mainMenu->clear();

    // Create the File menu

    auto fileMenu = UiMenu::create("File");

    this->onAddMenuItems(fileMenu.get());

    fileMenu->addItem(UiMenuItem::create("Exit", "ALT+F4", [this]() { this->close(); }));
    m_mainMenu->addMenu(fileMenu);

    // Create the View menu

    auto viewMenu = UiMenu::create("View");
    viewMenu->addItem(UiMenuItem::create(
        "Control panel", "/", [this]() { this->showControlPanel(); },
        [this]() {
            return m_sceneControlPanel->visible(); // Enable/disable based on visibility
        }));
    viewMenu->addItem(UiMenuItem::create(
        "Camera control", "v", [this]() { this->showCameraWindow(); },
        [this]() {
            return m_cameraWindow->visible(); // Enable/disable based on visibility
        }));
    viewMenu->addItem(UiMenuItem::create(
        "Effect Inspector", "e", [this]() { this->showEffectInspector(); },
        [this]() {
            return m_effectInspector->visible(); // Enable/disable based on visibility
        }));
    viewMenu->addItem(UiMenuItem::create(
        "Axis", "a",
        [this]() {
            if (m_showAxis)
                this->disableAxis();
            else
                this->enableAxis();
        },
        [this]() {
            return m_showAxis; // Enable/disable based on current state
        }));

    viewMenu->addItem(UiMenuItem::create(
        "Grid", "g",
        [this]() {
            if (m_showGrid)
                this->disableGrid();
            else
                this->enableGrid();
        },
        [this]() {
            return m_showGrid; // Enable/disable based on current state
        }));

    viewMenu->addItem(UiMenuItem::create(
        "Headlight", "h",
        [this]() {
            if (m_camManip->headlight() != nullptr)
                this->disableHeadlight();
            else
                this->enableHeadlight();
        },
        [this]() {
            return (m_camManip->headlight() != nullptr); // Enable/disable based on headlight state
        }));

    viewMenu->addItem(UiMenuItem::create(
        "Main menu", "CTRL+M",
        [this]() {
            if (m_showMainMenu)
                this->hideMainMenu();
            else
                this->showMainMenu();
        },
        [this]() {
            return m_showMainMenu; // Enable/disable based on current state
        }));

    this->onAddMenuItems(viewMenu.get());

    m_mainMenu->addMenu(viewMenu);

    if (!m_uiWindows.empty())
    {
        auto windowMenu = UiMenu::create("Windows");

        for (const auto &uiWindow : m_uiWindows)
        {
            if (uiWindow)
            {
                windowMenu->addItem(UiMenuItem::create(
                    uiWindow->name(), "", [uiWindow]() { uiWindow->toggleVisibility(); },
                    [uiWindow]() { return uiWindow->visible(); }));
            }
        }

        m_mainMenu->addMenu(windowMenu);
    }
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

        LightManager::instance()->renderShadowMaps(m_scene);
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

        LightManager::instance()->renderShadowMaps(m_scene);
        m_scene->draw();
    }

    for (auto &uiWindow : m_uiWindows)
        uiWindow->update();
}

void ivfui::GLFWSceneWindow::doDrawUi()
{
    if (m_showMainMenu)
        m_mainMenu->draw();

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
    if (key == GLFW_KEY_V)
    {
        this->showCameraWindow();
    }
    if (key == GLFW_KEY_A)
    {
        if (m_showAxis)
            this->disableAxis();
        else
            this->enableAxis();
    }
    if (key == GLFW_KEY_G)
    {
        if (m_showGrid)
            this->disableGrid();
        else
            this->enableGrid();
    }
    if (key == GLFW_KEY_H)
    {
        if (m_camManip->headlight() != nullptr)
            this->disableHeadlight();
        else
            this->enableHeadlight();
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

void ivfui::GLFWSceneWindow::onAddMenuItems(ivfui::UiMenu *menu)
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
