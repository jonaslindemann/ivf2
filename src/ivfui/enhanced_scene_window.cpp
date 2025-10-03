#include <ivfui/enhanced_scene_window.h>
#include <ivf/sphere.h>
#include <ivf/cube.h>
#include <ivf/material.h>
#include <ivf/transform_node.h>
#include <iostream>

using namespace ivfui;
using namespace ivf;

EnhancedSceneWindow::EnhancedSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor,
                                         GLFWwindow *shared)
    : GLFWSceneWindow(width, height, title, monitor, shared), m_placementEnabled(false)
{}

EnhancedSceneWindow::~EnhancedSceneWindow()
{
    // Cleanup is handled by unique_ptr
}

std::shared_ptr<EnhancedSceneWindow> EnhancedSceneWindow::create(int width, int height, const std::string title,
                                                                 GLFWmonitor *monitor, GLFWwindow *shared)
{
    return std::make_shared<EnhancedSceneWindow>(width, height, title, monitor, shared);
}

void EnhancedSceneWindow::enablePlacement(bool enable)
{
    m_placementEnabled = enable;
    if (m_placementMixin)
    {
        m_placementMixin->setPlacementActive(enable);
    }
}

void EnhancedSceneWindow::setPlacementPlane(PlacementPlane plane)
{
    if (m_placementMixin)
    {
        m_placementMixin->setPlane(plane);
    }
}

void EnhancedSceneWindow::setPlacementMode(PlacementMode mode)
{
    if (m_placementMixin)
    {
        m_placementMixin->setMode(mode);
    }
}

void EnhancedSceneWindow::setGridSnapping(bool enabled, float gridSize)
{
    if (m_placementMixin)
    {
        m_placementMixin->setGridSnapping(enabled, gridSize);
    }
}

void EnhancedSceneWindow::setCursorVisible(bool visible)
{
    if (m_placementMixin)
    {
        m_placementMixin->setCursorVisible(visible);
    }
}

void EnhancedSceneWindow::setPlacementGridVisible(bool visible)
{
    if (m_placementMixin)
    {
        m_placementMixin->setPlacementGridVisible(visible);
    }
}

void EnhancedSceneWindow::setNodeCreationCallback(PlacementMixin::NodeCreationCallback callback)
{
    if (m_placementMixin)
    {
        m_placementMixin->setNodeCreationCallback(callback);
    }
}

void EnhancedSceneWindow::setNodeSelectionCallback(PlacementMixin::NodeSelectionCallback callback)
{
    if (m_placementMixin)
    {
        m_placementMixin->setNodeSelectionCallback(callback);
    }
}

void EnhancedSceneWindow::setNodeMovedCallback(PlacementMixin::NodeMovedCallback callback)
{
    if (m_placementMixin)
    {
        m_placementMixin->setNodeMovedCallback(callback);
    }
}

void EnhancedSceneWindow::onMouseButton(int button, int action, int mods)
{
    // First let the placement mixin handle the event (it will check ImGui capture internally)
    if (m_placementMixin && m_placementEnabled)
    {
        m_placementMixin->handleMouseButton(button, action, mods);
    }

    // Then call the base class implementation
    GLFWSceneWindow::onMouseButton(button, action, mods);
}

void EnhancedSceneWindow::onMousePosition(double x, double y)
{
    // First let the placement mixin handle the event (it will check ImGui capture internally)
    if (m_placementMixin && m_placementEnabled)
    {
        m_placementMixin->handleMouseMove(x, y);
    }

    // Then call the base class implementation
    GLFWSceneWindow::onMousePosition(x, y);
}

void EnhancedSceneWindow::onKey(int key, int scancode, int action, int mods)
{
    // Handle placement-specific keys first
    if (m_placementMixin && m_placementEnabled)
    {
        m_placementMixin->handleKey(key, scancode, action, mods);
    }

    // Handle toggle placement mode
    if (key == GLFW_KEY_P && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL))
    {
        enablePlacement(!m_placementEnabled);
        std::cout << "Placement mode " << (m_placementEnabled ? "enabled" : "disabled") << std::endl;
    }

    // Then call the base class implementation
    GLFWSceneWindow::onKey(key, scancode, action, mods);
}

void EnhancedSceneWindow::doResize(int width, int height)
{
    // Handle resize for placement mixin
    if (m_placementMixin)
    {
        m_placementMixin->handleResize(width, height);
    }

    // Call base class implementation
    GLFWSceneWindow::doResize(width, height);
}

void EnhancedSceneWindow::doPreSetup()
{
    // Initialize placement mixin
    m_placementMixin = std::make_unique<PlacementMixin>(this);
    m_placementMixin->initialize();

    // Set up default node creation callback (creates spheres)
    m_placementMixin->setNodeCreationCallback([this](float x, float y, float z) -> ivf::NodePtr {
        auto sphere = Sphere::create();
        sphere->setRadius(0.2f);
        sphere->setPos(glm::vec3(x, y, z));

        // Create a random colored material
        auto material = Material::create();
        float r = static_cast<float>(rand()) / RAND_MAX;
        float g = static_cast<float>(rand()) / RAND_MAX;
        float b = static_cast<float>(rand()) / RAND_MAX;
        material->setDiffuseColor(glm::vec4(r, g, b, 1.0f));
        sphere->setMaterial(material);

        std::cout << "Created sphere at (" << x << ", " << y << ", " << z << ")" << std::endl;
        return sphere;
    });

    // Set up selection callback
    m_placementMixin->setNodeSelectionCallback([this](ivf::NodePtr node) {
        if (auto transformNode = std::dynamic_pointer_cast<TransformNode>(node))
        {
            std::cout << "Selected node at (" << transformNode->pos().x << ", " << transformNode->pos().y << ", "
                      << transformNode->pos().z << ")" << std::endl;
        }
    });

    // Set up moved callback
    m_placementMixin->setNodeMovedCallback([this](ivf::NodePtr node, float x, float y, float z) {
        std::cout << "Moved node to (" << x << ", " << y << ", " << z << ")" << std::endl;
    });
}

void EnhancedSceneWindow::onAddMenuItems(UiMenu *menu)
{
    if (!menu)
        return;

    // Add placement-related menu items
    menu->addSeparator();

    menu->addItem(UiMenuItem::create(
        "Toggle Placement Mode", "CTRL+P", [this]() { enablePlacement(!m_placementEnabled); },
        [this]() { return m_placementEnabled; }));

    if (m_placementMixin)
    {
        menu->addItem(UiMenuItem::create(
            "Toggle Cursor", "C",
            [this]() {
                bool visible = m_placementMixin->isCursorVisible();
                m_placementMixin->setCursorVisible(!visible);
            },
            [this]() { return m_placementMixin->isCursorVisible(); }));

        menu->addItem(UiMenuItem::create(
            "Toggle Placement Grid", "G",
            [this]() {
                bool visible = m_placementMixin->isPlacementGridVisible();
                m_placementMixin->setPlacementGridVisible(!visible);
            },
            [this]() { return m_placementMixin->isPlacementGridVisible(); }));

        menu->addItem(UiMenuItem::create(
            "XZ Plane", "SHIFT+Y", [this]() { m_placementMixin->setPlane(PlacementPlane::XZ); },
            [this]() { return m_placementMixin->currentPlane() == PlacementPlane::XZ; }));

        menu->addItem(UiMenuItem::create(
            "XY Plane", "SHIFT+Z", [this]() { m_placementMixin->setPlane(PlacementPlane::XY); },
            [this]() { return m_placementMixin->currentPlane() == PlacementPlane::XY; }));

        menu->addItem(UiMenuItem::create(
            "YZ Plane", "SHIFT+X", [this]() { m_placementMixin->setPlane(PlacementPlane::YZ); },
            [this]() { return m_placementMixin->currentPlane() == PlacementPlane::YZ; }));

        menu->addItem(UiMenuItem::create("Toggle Grid Snapping", "CTRL+G", [this]() {
            bool enabled = m_placementMixin->gridSnappingEnabled();
            m_placementMixin->setGridSnapping(!enabled, m_placementMixin->gridSize());
        }));
    }
}
