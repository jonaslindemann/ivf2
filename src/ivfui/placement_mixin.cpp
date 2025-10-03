#include <ivfui/placement_mixin.h>
#include <ivf/material.h>
#include <ivf/sphere.h>
#include <ivf/grid.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <cmath>

using namespace ivfui;
using namespace ivf;

PlacementMixin::PlacementMixin(GLFWSceneWindow *window)
    : m_window(window), m_showCursor(true), m_showPlacementGrid(false), m_placementActive(false), m_lastMouseX(0),
      m_lastMouseY(0), m_interactionMode(InteractionMode::AUTO), m_shouldBlockCamera(false), m_planeLocked(true),
      m_planeConstraintOffset(0.0f), m_lastUnconstrainedPosition(0.0f), m_constrainedPosition(0.0f),
      m_hasConstrainedPosition(false), m_viewportWidth(800), m_viewportHeight(600)
{
    if (!m_window)
    {
        throw std::runtime_error("PlacementMixin: GLFWSceneWindow pointer cannot be null");
    }

    // Create placement manipulator
    m_placementManip = PlacementManipulator::create(m_window->cameraManipulator());
}

PlacementMixin::~PlacementMixin()
{
    cleanup();
}

void PlacementMixin::initialize()
{
    setupCursor();
    setupPlacementGrid();

    // Set up callbacks
    m_placementManip->setOnMoveCallback([this](float x, float y, float z) { this->onCursorMove(x, y, z); });

    m_placementManip->setOnClickCallback([this](float x, float y, float z, int button, int action, int mods) {
        this->onCursorClick(x, y, z, button, action, mods);
    });

    m_placementManip->setOnDragCallback(
        [this](float x, float y, float z, float dx, float dy, float dz) { this->onNodeDrag(x, y, z, dx, dy, dz); });

    m_placementManip->setOnSelectCallback([this](ivf::NodePtr node) { this->onNodeSelect(node); });

    // Set viewport size
    m_placementManip->setViewport(m_window->width(), m_window->height());
}

void PlacementMixin::cleanup()
{
    if (m_cursor && m_window)
    {
        m_window->remove(m_cursor);
    }
    if (m_placementGrid && m_window)
    {
        m_window->remove(m_placementGrid);
    }
}

void PlacementMixin::setPlacementActive(bool active)
{
    m_placementActive = active;

    if (m_cursor)
    {
        m_cursor->setVisible(active && m_showCursor);
    }
    if (m_placementGrid)
    {
        m_placementGrid->setVisible(active && m_showPlacementGrid);
    }

    // Update interaction mode hints
    updateCameraModeHint();
}

void PlacementMixin::setInteractionMode(InteractionMode mode)
{
    m_interactionMode = mode;
    updateCameraModeHint();

    std::string modeStr;
    switch (mode)
    {
    case InteractionMode::CAMERA:
        modeStr = "Camera manipulation";
        break;
    case InteractionMode::PLACEMENT:
        modeStr = "Node placement/manipulation";
        break;
    case InteractionMode::AUTO:
        modeStr = "Automatic (context-sensitive)";
        break;
    }

    std::cout << "Interaction mode: " << modeStr << std::endl;
}

InteractionMode PlacementMixin::currentInteractionMode() const
{
    return m_interactionMode;
}

bool PlacementMixin::shouldBlockCameraManipulation() const
{
    return m_shouldBlockCamera;
}

void PlacementMixin::setCursorVisible(bool visible)
{
    m_showCursor = visible;
    if (m_cursor)
    {
        m_cursor->setVisible(m_placementActive && visible);
    }
}

void PlacementMixin::setPlacementGridVisible(bool visible)
{
    m_showPlacementGrid = visible;
    if (m_placementGrid)
    {
        m_placementGrid->setVisible(m_placementActive && visible);
    }
}

void PlacementMixin::setCursorRadius(float radius)
{
    if (m_cursor)
    {
        m_cursor->setRadius(radius);
    }
}

void PlacementMixin::setCursorMaterial(ivf::MaterialPtr material)
{
    if (m_cursor)
    {
        m_cursor->setMaterial(material);
    }
}

void PlacementMixin::setGridSize(float size)
{
    if (m_placementManip)
    {
        m_placementManip->setGridSnapping(m_placementManip->gridSnappingEnabled(), size);
    }
}

void PlacementMixin::setGridTicks(int x, int y, int z)
{
    if (m_placementGrid)
    {
        m_placementGrid->setTicks(x, y, z);
    }
}

void PlacementMixin::setGridSpacing(float x, float y, float z)
{
    if (m_placementGrid)
    {
        m_placementGrid->setSpacing(x, y, z);
    }
}

void PlacementMixin::setPlane(PlacementPlane plane)
{
    if (m_placementManip)
    {
        m_placementManip->setPlane(plane);
    }
    updatePlacementGrid();
}

PlacementPlane PlacementMixin::currentPlane() const
{
    return m_placementManip ? m_placementManip->currentPlane() : PlacementPlane::XZ;
}

void PlacementMixin::setMode(PlacementMode mode)
{
    if (m_placementManip)
    {
        m_placementManip->setMode(mode);
    }
}

PlacementMode PlacementMixin::currentMode() const
{
    return m_placementManip ? m_placementManip->currentMode() : PlacementMode::CREATE;
}

void PlacementMixin::setPlaneOffset(float offset)
{
    if (m_placementManip)
    {
        m_placementManip->setPlaneOffset(offset);
    }
    updatePlacementGrid();
}

void PlacementMixin::setGridSnapping(bool enabled, float gridSize)
{
    if (m_placementManip)
    {
        m_placementManip->setGridSnapping(enabled, gridSize);
    }
}

bool PlacementMixin::gridSnappingEnabled() const
{
    return m_placementManip ? m_placementManip->gridSnappingEnabled() : false;
}

float PlacementMixin::gridSize() const
{
    return m_placementManip ? m_placementManip->gridSize() : 1.0f;
}

void PlacementMixin::selectNode(ivf::NodePtr node)
{
    if (m_placementManip)
    {
        m_placementManip->selectNode(node);
        m_placementManip->setMode(PlacementMode::MOVE);
    }
}

void PlacementMixin::clearSelection()
{
    if (m_placementManip)
    {
        m_placementManip->clearSelection();
    }
}

const std::vector<ivf::NodePtr> &PlacementMixin::selectedNodes() const
{
    static std::vector<ivf::NodePtr> empty;
    return m_placementManip ? m_placementManip->selectedNodes() : empty;
}

void PlacementMixin::handleMouseButton(int button, int action, int mods)
{
    if (!m_placementActive || !m_placementManip)
        return;

    // Don't handle mouse events if ImGui wants to capture them
    if (isImGuiCapturingMouse())
        return;

    // Update interaction mode based on context
    updateInteractionMode(button, action, mods);

    // Only handle placement if we're in placement mode or auto mode allows it
    bool shouldHandlePlacement = false;

    switch (m_interactionMode)
    {
    case InteractionMode::PLACEMENT:
        shouldHandlePlacement = true;
        break;
    case InteractionMode::CAMERA:
        shouldHandlePlacement = false;
        break;
    case InteractionMode::AUTO:
        // In auto mode, decide based on context
        shouldHandlePlacement = shouldHandleInAutoMode(button, action, mods);
        break;
    }

    if (shouldHandlePlacement)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(m_window->ref(), &mouseX, &mouseY);
        m_placementManip->handleMouseButton(button, action, mods, mouseX, mouseY);
    }

    // Update camera blocking hint
    updateCameraModeHint();
}

void PlacementMixin::handleMouseMove(double x, double y)
{
    if (!m_placementActive || !m_placementManip)
        return;

    // Always update cursor position for visual feedback
    m_lastMouseX = static_cast<int>(x);
    m_lastMouseY = static_cast<int>(y);

    // Always update cursor position regardless of interaction mode
    if (!isImGuiCapturingMouse())
    {
        // Update cursor position with plane locking consideration
        m_placementManip->updateCursorPosition(x, y);

        // Apply plane locking if enabled
        if (m_planeLocked)
        {
            applyCursorPlaneLocking();
        }
    }

    // Don't handle mouse move for placement logic if ImGui wants to capture mouse
    if (isImGuiCapturingMouse())
        return;

    // Only handle placement mouse move for interaction logic if appropriate
    bool shouldHandlePlacement = false;

    switch (m_interactionMode)
    {
    case InteractionMode::PLACEMENT:
        shouldHandlePlacement = true;
        break;
    case InteractionMode::CAMERA:
        shouldHandlePlacement = false;
        break;
    case InteractionMode::AUTO:
        // In auto mode, handle if we're currently blocking camera or have active placement
        shouldHandlePlacement = m_shouldBlockCamera || !selectedNodes().empty();
        break;
    }

    if (shouldHandlePlacement)
    {
        m_placementManip->handleMouseMove(x, y);

        // Apply plane locking after placement handling too
        if (m_planeLocked)
        {
            applyCursorPlaneLocking();
        }
    }
}

void PlacementMixin::handleKey(int key, int scancode, int action, int mods)
{
    if (!m_placementActive)
        return;

    // Don't handle placement keys if ImGui wants to capture keyboard input
    if (isImGuiCapturingKeyboard())
    {
        return;
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        // Switch interaction modes
        if (key == GLFW_KEY_TAB && !(mods & GLFW_MOD_SHIFT))
        {
            // Tab: cycle through interaction modes
            switch (m_interactionMode)
            {
            case InteractionMode::AUTO:
                setInteractionMode(InteractionMode::CAMERA);
                break;
            case InteractionMode::CAMERA:
                setInteractionMode(InteractionMode::PLACEMENT);
                break;
            case InteractionMode::PLACEMENT:
                setInteractionMode(InteractionMode::AUTO);
                break;
            }
        }
        // Quick mode switches
        else if (key == GLFW_KEY_1 && (mods & GLFW_MOD_ALT))
        {
            setInteractionMode(InteractionMode::AUTO);
        }
        else if (key == GLFW_KEY_2 && (mods & GLFW_MOD_ALT))
        {
            setInteractionMode(InteractionMode::CAMERA);
        }
        else if (key == GLFW_KEY_3 && (mods & GLFW_MOD_ALT))
        {
            setInteractionMode(InteractionMode::PLACEMENT);
        }

        // Switch planes with modifier keys
        else if (key == GLFW_KEY_X && (mods & GLFW_MOD_SHIFT))
        {
            setPlane(PlacementPlane::YZ); // X plane (YZ visible)
        }
        else if (key == GLFW_KEY_Y && (mods & GLFW_MOD_SHIFT))
        {
            setPlane(PlacementPlane::XZ); // Y plane (XZ visible) - default
        }
        else if (key == GLFW_KEY_Z && (mods & GLFW_MOD_SHIFT))
        {
            setPlane(PlacementPlane::XY); // Z plane (XY visible)
        }

        // Toggle grid snapping
        else if (key == GLFW_KEY_G && (mods & GLFW_MOD_CONTROL))
        {
            bool currentSnap = m_placementManip->gridSnappingEnabled();
            m_placementManip->setGridSnapping(!currentSnap, m_placementManip->gridSize());
        }

        // Toggle placement grid visibility
        else if (key == GLFW_KEY_G && !(mods & GLFW_MOD_CONTROL))
        {
            setPlacementGridVisible(!m_showPlacementGrid);
        }

        // Toggle cursor visibility
        else if (key == GLFW_KEY_C)
        {
            setCursorVisible(!m_showCursor);
        }

        // Toggle plane locking
        else if (key == GLFW_KEY_L)
        {
            setPlaneLocked(!m_planeLocked);
        }

        // Switch modes
        else if (key == GLFW_KEY_M)
        {
            PlacementMode current = currentMode();
            PlacementMode next = (current == PlacementMode::CREATE) ? PlacementMode::MOVE : PlacementMode::CREATE;
            setMode(next);
        }

        // Escape to deactivate or clear selection
        else if (key == GLFW_KEY_ESCAPE)
        {
            if (!selectedNodes().empty())
            {
                clearSelection();
                setMode(PlacementMode::CREATE);
            }
            else
            {
                setPlacementActive(false);
            }
        }
    }
}

void PlacementMixin::handleResize(int width, int height)
{
    // Store viewport size for ray calculations
    m_viewportWidth = width;
    m_viewportHeight = height;

    if (m_placementManip)
    {
        m_placementManip->setViewport(width, height);
    }
}

glm::vec3 PlacementMixin::currentCursorPosition() const
{
    // Return constrained position if plane locking is active and we have a constraint
    if (m_planeLocked && m_hasConstrainedPosition)
    {
        return m_constrainedPosition;
    }

    // Otherwise return the placement manipulator's position
    return m_placementManip ? m_placementManip->currentWorldPosition() : glm::vec3(0.0f);
}

void PlacementMixin::setupCursor()
{
    m_cursor = Sphere::create();
    m_cursor->setRadius(0.05f);
    m_cursor->refresh();

    // Create a semi-transparent material for the cursor
    auto cursorMaterial = Material::create();
    cursorMaterial->setDiffuseColor(glm::vec4(1.0f, 1.0f, 0.0f, 0.7f)); // Yellow
    cursorMaterial->setAmbientColor(glm::vec4(0.3f, 0.3f, 0.0f, 0.7f));
    m_cursor->setMaterial(cursorMaterial);

    m_cursor->setVisible(false);
    m_window->add(m_cursor);
}

void PlacementMixin::setupPlacementGrid()
{
    m_placementGrid = Grid::create();
    m_placementGrid->setTicks(20, 20, 20);
    m_placementGrid->setSpacing(1.0f, 1.0f, 1.0f);

    // Initialize with current placement plane
    updatePlacementGrid();

    m_placementGrid->setVisible(false);
    m_window->add(m_placementGrid);
}

void PlacementMixin::updateCursorPosition()
{
    if (m_cursor)
    {
        glm::vec3 pos = currentCursorPosition();
        m_cursor->setPos(pos);

        // Change cursor appearance based on state
        if (m_cursor->material())
        {
            auto material = m_cursor->material();
            glm::vec4 color;

            if (isImGuiCapturingMouse())
            {
                // ImGui capturing - semi-transparent to indicate disabled
                color = glm::vec4(1.0f, 1.0f, 0.0f, 0.3f);
            }
            else if (!m_planeLocked)
            {
                // Plane constraint disabled - yellow for free 3D movement
                color = glm::vec4(1.0f, 1.0f, 0.0f, 0.7f);
            }
            else
            {
                // Plane constraint enabled (default) - check if shift is pressed
                bool shiftPressed = (glfwGetKey(m_window->ref(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                                    (glfwGetKey(m_window->ref(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
                if (shiftPressed)
                {
                    // Shift pressed while constrained - orange for perpendicular movement
                    color = glm::vec4(1.0f, 0.5f, 0.0f, 0.8f);
                }
                else
                {
                    // Normal plane constraint - red for constrained movement
                    color = glm::vec4(1.0f, 0.0f, 0.0f, 0.8f);
                }
            }

            material->setDiffuseColor(color);
        }
    }
}

void PlacementMixin::updatePlacementGrid()
{
    if (!m_placementGrid)
        return;

    // Get current placement plane and offset
    PlacementPlane plane = currentPlane();
    float offset = m_placementManip ? m_placementManip->planeOffset() : 0.0f;

    // Map PlacementPlane to GridPlane and set grid plane
    switch (plane)
    {
    case PlacementPlane::XZ:
        m_placementGrid->setPlane(ivf::GridPlane::XZ);
        m_placementGrid->setPos(glm::vec3(0.0f, offset, 0.0f));
        std::cout << "Placement grid synchronized to XZ plane" << std::endl;
        break;
    case PlacementPlane::XY:
        m_placementGrid->setPlane(ivf::GridPlane::XY);
        m_placementGrid->setPos(glm::vec3(0.0f, 0.0f, offset));
        std::cout << "Placement grid synchronized to XY plane" << std::endl;
        break;
    case PlacementPlane::YZ:
        m_placementGrid->setPlane(ivf::GridPlane::YZ);
        m_placementGrid->setPos(glm::vec3(offset, 0.0f, 0.0f));
        std::cout << "Placement grid synchronized to YZ plane" << std::endl;
        break;
    }
}

void PlacementMixin::onCursorMove(float x, float y, float z)
{
    updateCursorPosition();
}

void PlacementMixin::onCursorClick(float x, float y, float z, int button, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    // Don't create nodes if ImGui is capturing mouse input
    if (isImGuiCapturingMouse())
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (currentMode() == PlacementMode::CREATE)
        {
            // ALWAYS use the current cursor position (which includes perpendicular movement)
            // This ensures that clicks work in both normal and perpendicular modes
            glm::vec3 nodePos = currentCursorPosition();

            // Create new node at the determined position
            if (m_nodeCreationCallback)
            {
                auto newNode = m_nodeCreationCallback(nodePos.x, nodePos.y, nodePos.z);
                if (newNode)
                {
                    m_window->add(newNode);
                    std::cout << "Node created at: (" << nodePos.x << ", " << nodePos.y << ", " << nodePos.z << ")"
                              << std::endl;
                }
            }
        }
    }
}

void PlacementMixin::onNodeDrag(float x, float y, float z, float dx, float dy, float dz)
{
    // Node positions are updated automatically by the placement manipulator
    // This callback can be used for additional feedback or validation

    // Call user callback for each moved node
    if (m_nodeMovedCallback)
    {
        for (auto &node : selectedNodes())
        {
            // Cast to TransformNode to access pos() method
            if (auto transformNode = std::dynamic_pointer_cast<TransformNode>(node))
            {
                glm::vec3 pos = transformNode->pos();
                m_nodeMovedCallback(node, pos.x, pos.y, pos.z);
            }
        }
    }
}

void PlacementMixin::onNodeSelect(ivf::NodePtr node)
{
    if (m_nodeSelectionCallback)
    {
        m_nodeSelectionCallback(node);
    }
}

bool PlacementMixin::isImGuiCapturingMouse() const
{
    // Access the UI renderer through the GLFWWindow to check if ImGui wants to capture mouse
    if (m_window && m_window->uiRenderer())
    {
        return m_window->uiRenderer()->wantCaptureMouse();
    }
    return false;
}

bool PlacementMixin::isImGuiCapturingKeyboard() const
{
    // Access the UI renderer through the GLFWWindow to check if ImGui wants to capture keyboard
    if (m_window && m_window->uiRenderer())
    {
        return m_window->uiRenderer()->wantCaptureKeyboard();
    }
    return false;
}

void PlacementMixin::setGridPlane(ivf::GridPlane plane)
{
    if (m_placementGrid)
    {
        m_placementGrid->setPlane(plane);
    }
}

ivf::GridPlane PlacementMixin::currentGridPlane() const
{
    return m_placementGrid ? m_placementGrid->plane() : ivf::GridPlane::Full3D;
}

void PlacementMixin::updateInteractionMode(int button, int action, int mods)
{
    if (m_interactionMode != InteractionMode::AUTO)
        return;

    // In AUTO mode, determine interaction based on context
    bool hasSelection = !selectedNodes().empty();
    bool isCreateMode = (currentMode() == PlacementMode::CREATE);
    bool isLeftClick = (button == GLFW_MOUSE_BUTTON_LEFT);
    bool isRightClick = (button == GLFW_MOUSE_BUTTON_RIGHT);
    bool hasModifiers = (mods & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT)) != 0;

    // Determine if this should be placement or camera operation
    m_shouldBlockCamera = false;

    if (action == GLFW_PRESS)
    {
        if (isLeftClick && isCreateMode && !hasModifiers)
        {
            // Left click in create mode without modifiers -> placement
            m_shouldBlockCamera = true;
        }
        else if (isLeftClick && hasSelection)
        {
            // Left click with selection -> might be node manipulation
            m_shouldBlockCamera = true;
        }
        else if (hasModifiers)
        {
            // Any click with modifiers might be placement-specific
            m_shouldBlockCamera = false; // Let camera handle modified clicks
        }
    }
    else if (action == GLFW_RELEASE)
    {
        // On release, check if we should continue blocking
        m_shouldBlockCamera = hasSelection && (currentMode() == PlacementMode::MOVE);
    }
}

bool PlacementMixin::shouldHandleInAutoMode(int button, int action, int mods) const
{
    bool hasSelection = !selectedNodes().empty();
    bool isCreateMode = (currentMode() == PlacementMode::CREATE);
    bool isLeftClick = (button == GLFW_MOUSE_BUTTON_LEFT);
    bool hasModifiers = (mods & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT)) != 0;

    // Handle placement for:
    // - Left clicks in create mode without modifiers
    // - Any interaction when nodes are selected
    // - Middle mouse is reserved for camera zoom, don't interfere
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        return false;
    }

    if (isLeftClick && isCreateMode && !hasModifiers)
    {
        return true; // Node creation
    }

    if (hasSelection)
    {
        return true; // Node manipulation
    }

    return false; // Let camera handle everything else
}

void PlacementMixin::updateCameraModeHint()
{
    // Update camera manipulation blocking based on current state
    auto cameraManip = m_window->cameraManipulator();
    if (cameraManip)
    {
        bool shouldBlock = false;

        switch (m_interactionMode)
        {
        case InteractionMode::CAMERA:
            shouldBlock = false;
            m_shouldBlockCamera = false;
            break;
        case InteractionMode::PLACEMENT:
            shouldBlock = m_placementActive;
            m_shouldBlockCamera = m_placementActive;
            break;
        case InteractionMode::AUTO:
            shouldBlock = m_shouldBlockCamera;
            break;
        }

        cameraManip->setManipulationBlocked(shouldBlock);

        // Debug output (can be removed later)
        static bool lastBlockState = false;
        if (shouldBlock != lastBlockState)
        {
            std::cout << "Camera manipulation " << (shouldBlock ? "BLOCKED" : "ENABLED") << std::endl;
            lastBlockState = shouldBlock;
        }
    }
}

void PlacementMixin::applyCursorPlaneLocking()
{
    if (!m_planeLocked || !m_placementManip)
        return;

    // Check if Shift key is pressed for perpendicular movement
    bool shiftPressed = (glfwGetKey(m_window->ref(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                        (glfwGetKey(m_window->ref(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if (shiftPressed)
    {
        // SHIFT pressed: Perpendicular movement mode
        glm::vec3 constrainedPos = calculatePerpendicularMovementFromMouse();
        setCursorWorldPosition(constrainedPos);
    }
    else
    {
        // Normal mode: Constrain to the current plane
        glm::vec3 currentPos = m_placementManip->currentWorldPosition();
        glm::vec3 constrainedPos = currentPos;

        PlacementPlane plane = currentPlane();
        switch (plane)
        {
        case PlacementPlane::XZ:
            constrainedPos.y = m_planeConstraintOffset;
            break;
        case PlacementPlane::XY:
            constrainedPos.z = m_planeConstraintOffset;
            break;
        case PlacementPlane::YZ:
            constrainedPos.x = m_planeConstraintOffset;
            break;
        }

        setCursorWorldPosition(constrainedPos);
        updatePlacementManipulatorPosition(constrainedPos);
    }
}

void PlacementMixin::getRayFromScreen(double screenX, double screenY, glm::vec3 &rayOrigin, glm::vec3 &rayDirection)
{
    // Convert screen coordinates to normalized device coordinates (NDC)
    float x = (2.0f * static_cast<float>(screenX)) / m_viewportWidth - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(screenY)) / m_viewportHeight;

    // Get camera position and matrices
    rayOrigin = m_window->cameraManipulator()->cameraPosition();
    glm::vec3 cameraTarget = m_window->cameraManipulator()->cameraTarget();

    // Calculate camera's coordinate system
    glm::vec3 forward = glm::normalize(cameraTarget - rayOrigin);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(right, forward));

    // Get field of view from camera manipulator
    double fovDegrees = m_window->cameraManipulator()->fov();
    float fov = static_cast<float>(fovDegrees) * glm::pi<float>() / 180.0f; // Convert to radians
    float aspect = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

    // Calculate the ray direction in world space
    float tanHalfFov = tan(fov * 0.5f);
    float u = x * tanHalfFov * aspect;
    float v = y * tanHalfFov;

    rayDirection = glm::normalize(forward + u * right + v * up);
}

glm::vec3 PlacementMixin::calculatePerpendicularMovement()
{
    // Get the current locked position (when shift was first pressed)
    static glm::vec3 lockedPosition = currentCursorPosition();

    // Get current mouse ray
    double mouseX, mouseY;
    glfwGetCursorPos(m_window->ref(), &mouseX, &mouseY);

    glm::vec3 rayOrigin, rayDirection;
    getRayFromScreen(mouseX, mouseY, rayOrigin, rayDirection);

    // Calculate perpendicular line based on current plane
    PlacementPlane plane = currentPlane();
    glm::vec3 lineDirection;

    switch (plane)
    {
    case PlacementPlane::XZ:
        // Movement perpendicular to XZ plane is along Y axis
        lineDirection = glm::vec3(0.0f, 1.0f, 0.0f);
        break;
    case PlacementPlane::XY:
        // Movement perpendicular to XY plane is along Z axis
        lineDirection = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case PlacementPlane::YZ:
        // Movement perpendicular to YZ plane is along X axis
        lineDirection = glm::vec3(1.0f, 0.0f, 0.0f);
        break;
    }

    // Find closest point on the perpendicular line
    glm::vec3 closestPoint = rayLineIntersection(rayOrigin, rayDirection, lockedPosition, lineDirection);

    return closestPoint;
}

glm::vec3 PlacementMixin::calculatePerpendicularMovementFromMouse()
{
    // Get current mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(m_window->ref(), &mouseX, &mouseY);

    // Get current mouse ray
    glm::vec3 rayOrigin, rayDirection;
    getRayFromScreen(mouseX, mouseY, rayOrigin, rayDirection);

    // Get the current placement plane and its properties
    PlacementPlane plane = currentPlane();
    glm::vec3 planeNormal;
    glm::vec3 planePoint;

    // Define the current placement plane
    switch (plane)
    {
    case PlacementPlane::XZ:
        planeNormal = glm::vec3(0.0f, 1.0f, 0.0f); // Y is up
        planePoint = glm::vec3(0.0f, m_planeConstraintOffset, 0.0f);
        break;
    case PlacementPlane::XY:
        planeNormal = glm::vec3(0.0f, 0.0f, 1.0f); // Z is forward
        planePoint = glm::vec3(0.0f, 0.0f, m_planeConstraintOffset);
        break;
    case PlacementPlane::YZ:
        planeNormal = glm::vec3(1.0f, 0.0f, 0.0f); // X is right
        planePoint = glm::vec3(m_planeConstraintOffset, 0.0f, 0.0f);
        break;
    }

    // Step 1: Get the free in-plane position from mouse ray intersecting placement plane
    glm::vec3 inPlanePosition = rayPlaneIntersection(rayOrigin, rayDirection, planePoint, planeNormal);

    // Step 2: Apply grid snapping to in-plane coordinates if enabled
    if (gridSnappingEnabled())
    {
        float gridSize = this->gridSize();
        inPlanePosition.x = std::round(inPlanePosition.x / gridSize) * gridSize;
        inPlanePosition.y = std::round(inPlanePosition.y / gridSize) * gridSize;
        inPlanePosition.z = std::round(inPlanePosition.z / gridSize) * gridSize;
    }

    // Step 3: Calculate perpendicular coordinate using SIMPLE linear mouse Y movement
    // Convert mouse Y to normalized coordinate [-1, 1] where:
    // - Top of screen (mouseY = 0) → normalizedY = 1
    // - Bottom of screen (mouseY = height) → normalizedY = -1
    float normalizedY = 1.0f - (2.0f * static_cast<float>(mouseY)) / m_viewportHeight;

    // Use a FIXED scale for predictable movement (not camera-distance dependent)
    float perpendicularRange = 10.0f; // Fixed range: ±10 units from plane
    float perpendicularOffset = normalizedY * perpendicularRange;

    // Step 4: Create final position by combining in-plane position with perpendicular offset
    glm::vec3 resultPosition = inPlanePosition;

    switch (plane)
    {
    case PlacementPlane::XZ:
        // For XZ plane: use in-plane X,Z and calculated Y
        resultPosition.y = m_planeConstraintOffset + perpendicularOffset;
        break;
    case PlacementPlane::XY:
        // For XY plane: use in-plane X,Y and calculated Z
        resultPosition.z = m_planeConstraintOffset + perpendicularOffset;
        break;
    case PlacementPlane::YZ:
        // For YZ plane: use in-plane Y,Z and calculated X
        resultPosition.x = m_planeConstraintOffset + perpendicularOffset;
        break;
    }

    // Step 5: Apply grid snapping to the perpendicular coordinate if enabled
    if (gridSnappingEnabled())
    {
        float gridSize = this->gridSize();
        switch (plane)
        {
        case PlacementPlane::XZ:
            resultPosition.y = std::round(resultPosition.y / gridSize) * gridSize;
            break;
        case PlacementPlane::XY:
            resultPosition.z = std::round(resultPosition.z / gridSize) * gridSize;
            break;
        case PlacementPlane::YZ:
            resultPosition.x = std::round(resultPosition.x / gridSize) * gridSize;
            break;
        }
    }

    return resultPosition;
}

glm::vec3 PlacementMixin::rayLineIntersection(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection,
                                              const glm::vec3 &linePoint, const glm::vec3 &lineDirection)
{
    // Find the closest point between a ray and a line using parametric equations
    // Ray: P = rayOrigin + t * rayDirection
    // Line: Q = linePoint + s * lineDirection
    // We want to minimize |P - Q|

    glm::vec3 w0 = rayOrigin - linePoint;
    float a = glm::dot(rayDirection, rayDirection);
    float b = glm::dot(rayDirection, lineDirection);
    float c = glm::dot(lineDirection, lineDirection);
    float d = glm::dot(rayDirection, w0);
    float e = glm::dot(lineDirection, w0);

    float denominator = a * c - b * b;

    // Check if lines are parallel
    if (std::abs(denominator) < 1e-6f)
    {
        // Lines are parallel, use the line point
        return linePoint;
    }

    float s = (b * d - a * e) / denominator;

    // Calculate the point on the line
    glm::vec3 closestPoint = linePoint + s * lineDirection;

    return closestPoint;
}

glm::vec3 PlacementMixin::rayPlaneIntersection(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection,
                                               const glm::vec3 &planePoint, const glm::vec3 &planeNormal)
{
    // Calculate intersection with plane
    // Ray: P = rayOrigin + t * rayDirection
    // Plane: dot(P - planePoint, planeNormal) = 0
    // Solving: t = dot(planePoint - rayOrigin, planeNormal) / dot(rayDirection, planeNormal)

    float denominator = glm::dot(rayDirection, planeNormal);
    if (std::abs(denominator) < 1e-6f)
    {
        // Ray is parallel to plane, return a default position
        return planePoint;
    }

    float t = glm::dot(planePoint - rayOrigin, planeNormal) / denominator;
    return rayOrigin + t * rayDirection;
}

void PlacementMixin::setCursorWorldPosition(const glm::vec3 &position)
{
    // Set the visual cursor position
    if (m_cursor)
    {
        m_cursor->setPos(position);
    }

    // Store the constrained position to override PlacementManipulator when needed
    m_constrainedPosition = position;
    m_hasConstrainedPosition = true;
}

void PlacementMixin::updatePlacementManipulatorPosition(const glm::vec3 &position)
{
    // Store the constrained position for use in callbacks
    // This ensures that when nodes are created or moved, they use the constrained position
    m_constrainedPosition = position;
    m_hasConstrainedPosition = true;
}

void PlacementMixin::setPlaneLocked(bool locked)
{
    // Note: This method now controls whether plane constraint is enabled at all
    // By default, cursor should be constrained to plane (locked = true by default)
    m_planeLocked = locked;

    if (locked)
    {
        // When enabling plane constraint, set offset to current cursor position
        glm::vec3 currentPos = currentCursorPosition();
        m_lastUnconstrainedPosition = currentPos;

        // Calculate offset along the plane normal
        PlacementPlane plane = currentPlane();
        switch (plane)
        {
        case PlacementPlane::XZ:
            m_planeConstraintOffset = currentPos.y;
            break;
        case PlacementPlane::XY:
            m_planeConstraintOffset = currentPos.z;
            break;
        case PlacementPlane::YZ:
            m_planeConstraintOffset = currentPos.x;
            break;
        }

        std::cout << "Plane constraint ENABLED - cursor constrained to " << getPlaneDescription() << " at offset "
                  << m_planeConstraintOffset << std::endl;
    }
    else
    {
        std::cout << "Plane constraint DISABLED - free 3D movement" << std::endl;
    }
}

bool PlacementMixin::isPlaneLocked() const
{
    return m_planeLocked;
}

void PlacementMixin::setPlaneConstraintOffset(float offset)
{
    m_planeConstraintOffset = offset;

    if (m_planeLocked)
    {
        std::cout << "Plane constraint offset updated to " << offset << std::endl;
    }
}

float PlacementMixin::planeConstraintOffset() const
{
    return m_planeConstraintOffset;
}

std::string PlacementMixin::getPlaneDescription() const
{
    PlacementPlane plane = currentPlane();
    switch (plane)
    {
    case PlacementPlane::XZ:
        return "XZ plane";
    case PlacementPlane::XY:
        return "XY plane";
    case PlacementPlane::YZ:
        return "YZ plane";
    default:
        return "Unknown plane";
    }
}
