#include <ivfui/glfw_scene_window.h>

using namespace std;
using namespace ivfui;

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/shader_manager.h>
#include <ivf/font_manager.h>

#include <ivf/stock_shaders.h>

#include <glm/gtx/intersect.hpp>

using namespace ivf;

GLFWSceneWindow::GLFWSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor,
                                 GLFWwindow *shared)
    : GLFWWindow(width, height, title, monitor, shared), m_selectionEnabled(false), m_lastNode(nullptr),
      m_currentNode(nullptr), m_renderToTexture(false), m_selectionRendering(false), m_showAxis(false),
      m_showGrid(false), m_currentIntersectionPoint(0.0f, 0.0f, 0.0f), m_snapToGrid(false), m_lockPosXZ(false),
      m_gridSnapValue(0.1f), m_cursor(nullptr)
{
    m_scene = ivf::CompositeNode::create();
    m_camManip = ivfui::CameraManipulator::create(this->ref());
    m_bufferSelection = ivf::BufferSelection::create(m_scene);
    m_frameBuffer = ivf::FrameBuffer::create(width, height);
    m_postProcessor = ivf::PostProcessor::create(width, height);
    m_mainMenu = ivfui::UiMainMenu::create();
    m_inputDialog = ivfui::UiInputDialog::create("Grid Snap Value", "Snap Value:");
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
    // Re-add essential scene elements after clearing
    if (m_cursor)
    {
        this->add(m_cursor);
    }
    if (m_axis)
    {
        this->add(m_axis);
    }
    if (m_grid)
    {
        this->add(m_grid);
    }
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

ivf::GridPtr ivfui::GLFWSceneWindow::grid()
{
    return m_grid;
}

ivf::CursorPtr ivfui::GLFWSceneWindow::cursor()
{
    return m_cursor;
}

void ivfui::GLFWSceneWindow::enableCursor()
{
    if (m_cursor)
    {
        m_cursor->setVisible(true);
    }
}

void ivfui::GLFWSceneWindow::disableCursor()
{
    if (m_cursor)
    {
        m_cursor->setVisible(false);
    }
}

bool ivfui::GLFWSceneWindow::cursorEnabled()
{
    return m_cursor ? m_cursor->visible() : false;
}

void ivfui::GLFWSceneWindow::setCursorVisible(bool visible)
{
    if (m_cursor)
    {
        m_cursor->setVisible(visible);
    }
}

bool ivfui::GLFWSceneWindow::cursorVisible()
{
    return m_cursor ? m_cursor->visible() : false;
}

void ivfui::GLFWSceneWindow::setCursorPosition(float x, float y, float z)
{
    if (m_cursor)
    {
        m_cursor->setPos(glm::vec3(x, y, z));
        m_currentIntersectionPoint = glm::vec3(x, y, z);
#ifdef IVF_DEBUG
        std::cout << "Cursor position manually set to: " << x << ", " << y << ", " << z << std::endl;
#endif
    }
}

void ivfui::GLFWSceneWindow::setGridSnapValue(float value)
{
    m_gridSnapValue = value;
}

float ivfui::GLFWSceneWindow::gridSnapValue()
{
    return m_gridSnapValue;
}

void ivfui::GLFWSceneWindow::setSnapToGrid(bool snap)
{
    m_snapToGrid = snap;
}

bool ivfui::GLFWSceneWindow::snapToGrid()
{
    return m_snapToGrid;
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

void ivfui::GLFWSceneWindow::showSceneInspector()
{
    m_sceneInspector->show();
}

void ivfui::GLFWSceneWindow::zoomToExtent(bool includeInvisible)
{
    m_camManip->zoomToExtent(m_scene, includeInvisible);
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
    m_grid->setPlane(ivf::GridPlane::XZ);
    m_cursor = ivf::Cursor::create(0.5f, 0.05f, true); // Make cursor larger: 0.5 size, 0.05 gap
    m_cursor->setPos(glm::vec3(0.0f, 0.0f, 0.0f));
    m_cursor->refresh();

    this->add(m_axis);
    this->add(m_grid);
    this->add(m_cursor);

    m_axis->setVisible(m_showAxis);
    m_grid->setVisible(m_showGrid);
    m_cursor->setVisible(false); // Start with cursor hidden

    // Make cursor visible by default and position it at origin

#ifdef IVF_DEBUG
    std::cout << "Cursor created and positioned at origin: " << m_cursor->pos().x << ", " << m_cursor->pos().y << ", "
              << m_cursor->pos().z << std::endl;
    std::cout << "Cursor visible: " << m_cursor->visible() << std::endl;
#endif
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
    m_cameraWindow->setScene(m_scene); // Set scene reference for extent calculation
    m_cameraWindow->hide();

    this->addUiWindow(m_cameraWindow);

    // Create EffectInspector with the new interface, passing 'this' as EffectListProvider
    m_effectInspector = ivfui::EffectInspector::create("Effect Inspector", this);
    m_effectInspector->hide();

    this->addUiWindow(m_effectInspector);

    m_sceneInspector = ivfui::SceneInspector::create("Scene Inspector");
    m_sceneInspector->hide();
    m_sceneInspector->setRootNode(m_scene);

    this->addUiWindow(m_sceneInspector);

    this->doPreSetup();
    auto retVal = onSetup();
    this->doPostSetup();
    this->doSetupMainMenu();

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

    viewMenu->addItem(UiMenuItem::create("Zoom to Extent", "F", [this]() { this->zoomToExtent(); }));
    viewMenu->addItem(UiMenuItem::create("Reset View", "CTRL+R", [this]() { this->resetView(); }));
    viewMenu->addItem(UiMenuItem::create("Save View", "", [this]() { this->saveView(); }));

    viewMenu->addSeparator();

    viewMenu->addItem(UiMenuItem::create(
        "Axis", "A",
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
        "Grid", "G",
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
        "Cursor", "C",
        [this]() {
            if (this->cursorVisible())
                this->disableCursor();
            else
                this->enableCursor();
        },
        [this]() {
            return this->cursorVisible(); // Enable/disable based on current state
        }));

    viewMenu->addItem(UiMenuItem::create(
        "Snap to Grid", "", [this]() { this->setSnapToGrid(!this->snapToGrid()); },
        [this]() { return this->snapToGrid(); }));

    viewMenu->addItem(UiMenuItem::create("Grid Snap Value...", "CTRL+G", [this]() { this->showGridSnapDialog(); }));

    viewMenu->addSeparator();

    viewMenu->addItem(UiMenuItem::create(
        "Headlight", "H",
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

    viewMenu->addSeparator();

    /*
    viewMenu->addItem(UiMenuItem::create(
        "Camera control", "V", [this]() { this->showCameraWindow(); },
        [this]() {
            return m_cameraWindow->visible(); // Enable/disable based on visibility
        }));

    viewMenu->addItem(UiMenuItem::create(
        "Scene Inspector", "S", [this]() { this->showSceneInspector(); },
        [this]() {
            return m_sceneInspector->visible(); // Enable/disable based on visibility
        }));
    viewMenu->addItem(UiMenuItem::create(
        "Effect Inspector", "E", [this]() { this->showEffectInspector(); },
        [this]() {
            return m_effectInspector->visible(); // Enable/disable based on visibility
        }));

    */

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

    // Draw the input dialog if open
    if (m_inputDialog)
        m_inputDialog->draw();

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

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_V)
        {
            this->showCameraWindow();
        }
        if (key == GLFW_KEY_S)
        {
            this->showSceneInspector();
        }
        if (key == GLFW_KEY_E)
        {
            this->showEffectInspector();
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
        if (key == GLFW_KEY_C)
        {
            if (this->cursorVisible())
                this->disableCursor();
            else
                this->enableCursor();
        }
        if (key == GLFW_KEY_T) // Test cursor position
        {
            static float testX = 0.0f;
            testX += 1.0f; // Move cursor 1 unit along X each time T is pressed
            this->setCursorPosition(testX, 0.0f, 0.0f);
            std::cout << "Test: Moving cursor to X=" << testX << std::endl;
        }
        if (key == GLFW_KEY_H)
        {
            if (m_camManip->headlight() != nullptr)
                this->disableHeadlight();
            else
                this->enableHeadlight();
        }

        // New camera controls
        if (key == GLFW_KEY_F) // F for "Fit" or "Frame"
        {
            this->zoomToExtent();
        }
        if (key == GLFW_KEY_R && (mods & GLFW_MOD_CONTROL)) // Ctrl+R for reset
        {
            this->resetView();
        }
        if (key == GLFW_KEY_G && (mods & GLFW_MOD_CONTROL)) // Ctrl+G for grid snap dialog
        {
            this->showGridSnapDialog();
        }

        // Quick save slots with number keys (Ctrl+0-9 to save, 0-9 to restore)
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
        {
            int slot = (key == GLFW_KEY_0) ? 0 : (key - GLFW_KEY_1 + 1);
            if (slot == 0)
                slot = 9; // Make 0 key map to slot 9 for easier access
            else
                slot = slot - 1; // Map 1-9 keys to slots 0-8

            if (mods & GLFW_MOD_CONTROL)
            {
                m_camManip->saveStateToSlot(slot);
            }
            else
            {
                if (m_camManip->hasSlotData(slot))
                    m_camManip->restoreStateFromSlot(slot);
            }
        }
    }

    if (mods & GLFW_MOD_SHIFT)
    {
        m_lockPosXZ = true;
    }
    else
    {
        m_lockPosXZ = false;
    }

    GLFWWindow::doKey(key, scancode, action, mods);
}

void ivfui::GLFWSceneWindow::doMousePosition(double x, double y)
{
    GLFWWindow::doMousePosition(x, y);

    // Only update cursor if it exists and is visible
    if (!m_cursor || !m_cursor->visible())
    {
        return;
    }

    auto vec3 = this->cameraManipulator()->computeMouseRay(x, y);

    // Compute intersection with XZ plane (ground plane at Y=0)

    if (!m_lockPosXZ)
    {
        glm::vec3 rayOrigin = this->cameraManipulator()->cameraPosition();
        glm::vec3 rayDirection = glm::normalize(vec3);
        glm::vec3 planeOrigin(0.0f, 0.0f, 0.0f); // A point on the XZ plane (Y=0)
        glm::vec3 planeNormal(0.0f, 1.0f, 0.0f); // Normal vector of the XZ plane (pointing up)

        float intersectionDistance;
        bool intersects = glm::intersectRayPlane(rayOrigin,           // Ray origin
                                                 rayDirection,        // Ray direction (should be normalized)
                                                 planeOrigin,         // Point on the plane
                                                 planeNormal,         // Plane normal (should be normalized)
                                                 intersectionDistance // Output: distance along ray to intersection
        );

        if (intersects && intersectionDistance > 0.0f) // Make sure intersection is in front of camera
        {
            // Calculate the actual intersection point
            glm::vec3 intersectionPoint = rayOrigin + rayDirection * intersectionDistance;

#ifdef IVF_DEBUG
            std::cout << "Intersection found!" << std::endl;
            std::cout << "Distance: " << intersectionDistance << std::endl;
            std::cout << "Point: (" << intersectionPoint.x << ", " << intersectionPoint.y << ", " << intersectionPoint.z
                      << ")" << std::endl;
#endif

            // Snap to grid
            if (m_snapToGrid && m_gridSnapValue > 0.0f)
            {
                intersectionPoint.x = round(intersectionPoint.x / m_gridSnapValue) * m_gridSnapValue;
                intersectionPoint.y = round(intersectionPoint.y / m_gridSnapValue) * m_gridSnapValue;
                intersectionPoint.z = round(intersectionPoint.z / m_gridSnapValue) * m_gridSnapValue;
            }

            m_currentIntersectionPoint = intersectionPoint;

            // Update cursor position
            if (m_cursor)
            {
                m_cursor->updatePosition(intersectionPoint);
            }

            this->onMousePosition3D(intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
        }
#ifdef IVF_DEBUG
        else
        {
            std::cout << "No intersection found (ray parallel to plane or behind camera)" << std::endl;
        }
#endif
    }
    else
    {
        // Lock position in XZ plane - constrain Y movement
        glm::vec3 rayOrigin = this->cameraManipulator()->cameraPosition();
        glm::vec3 rayDirection = glm::normalize(vec3);
        glm::vec3 planeOrigin = m_currentIntersectionPoint; // Use current intersection point as plane origin
        glm::vec3 planeNormal(0.0f, 0.0f, 1.0f);            // Normal vector of the YZ plane (constraining Z)

        float intersectionDistance;

        bool intersects = glm::intersectRayPlane(rayOrigin,           // Ray origin
                                                 rayDirection,        // Ray direction (should be normalized)
                                                 planeOrigin,         // Point on the plane
                                                 planeNormal,         // Plane normal (should be normalized)
                                                 intersectionDistance // Output: distance along ray to intersection
        );

        if (intersects && intersectionDistance > 0.0f)
        {
            // Calculate the actual intersection point
            glm::vec3 intersectionPoint = rayOrigin + rayDirection * intersectionDistance;

#ifdef IVF_DEBUG
            std::cout << "Locked intersection found!" << std::endl;
            std::cout << "Distance: " << intersectionDistance << std::endl;
            std::cout << "Point: (" << intersectionPoint.x << ", " << intersectionPoint.y << ", " << intersectionPoint.z
                      << ")" << std::endl;
#endif

            // Keep X and Z from previous position when locked
            intersectionPoint.x = m_currentIntersectionPoint.x;
            intersectionPoint.z = m_currentIntersectionPoint.z;

            if (m_snapToGrid && m_gridSnapValue > 0.0f)
            {
                intersectionPoint.x = round(intersectionPoint.x / m_gridSnapValue) * m_gridSnapValue;
                intersectionPoint.y = round(intersectionPoint.y / m_gridSnapValue) * m_gridSnapValue;
                intersectionPoint.z = round(intersectionPoint.z / m_gridSnapValue) * m_gridSnapValue;
            }

            m_currentIntersectionPoint = intersectionPoint;

            // Update cursor position
            if (m_cursor)
            {
                m_cursor->updatePosition(intersectionPoint);
            }

            this->onMousePosition3D(intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
        }
#ifdef IVF_DEBUG
        else
        {
            std::cout << "No locked intersection found (ray parallel to plane or behind camera)" << std::endl;
        }
#endif
    }
}

void ivfui::GLFWSceneWindow::doMouseButton(int button, int action, int mods)
{}

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

void ivfui::GLFWSceneWindow::onMousePosition3D(double x, double y, double z)
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

void ivfui::GLFWSceneWindow::showGridSnapDialog()
{
    m_inputDialog->openFloat(
        m_gridSnapValue,
        [this](bool accepted, const InputValue &value) {
            if (accepted)
            {
                float *floatValue = std::get<float *>(value);
                if (floatValue && *floatValue > 0.0f)
                {
                    m_gridSnapValue = *floatValue;
                }
            }
        },
        -FLT_MAX, FLT_MAX, 0.01f, 0.1f);
}
