#pragma once

/**
 * @file placement_mixin.h
 * @brief Declares the PlacementMixin class for integrating 3D placement functionality with GLFWSceneWindow.
 */

#include <ivfui/placement_manipulator.h>
#include <ivfui/glfw_scene_window.h>
#include <ivf/nodes.h>
#include <memory>
#include <functional>

namespace ivfui {

/**
 * @enum InteractionMode
 * @brief Defines the current interaction mode to prevent conflicts between camera and placement systems.
 */
enum class InteractionMode {
    CAMERA,    ///< Camera manipulation mode (default)
    PLACEMENT, ///< Node placement/manipulation mode
    AUTO       ///< Automatic mode switching based on context
};

/**
 * @class PlacementMixin
 * @brief Mixin class that adds 3D object placement and manipulation capabilities to GLFWSceneWindow.
 *
 * This mixin provides a clean interface for integrating 3D cursor functionality
 * with existing GLFWSceneWindow implementations. It handles:
 * - Mouse event integration with conflict resolution
 * - Keyboard modifiers for plane switching
 * - Node creation and manipulation callbacks
 * - Visual feedback for placement operations
 * - Modal interaction to prevent camera/placement conflicts
 */
class PlacementMixin {
public:
    // Callback types for node operations
    using NodeCreationCallback = std::function<ivf::NodePtr(float x, float y, float z)>;
    using NodeSelectionCallback = std::function<void(ivf::NodePtr node)>;
    using NodeMovedCallback = std::function<void(ivf::NodePtr node, float x, float y, float z)>;

private:
    GLFWSceneWindow *m_window;
    PlacementManipulatorPtr m_placementManip;

    // Visual feedback
    ivf::SpherePtr m_cursor;
    ivf::GridPtr m_placementGrid;
    bool m_showCursor;
    bool m_showPlacementGrid;

    // Callbacks
    NodeCreationCallback m_nodeCreationCallback;
    NodeSelectionCallback m_nodeSelectionCallback;
    NodeMovedCallback m_nodeMovedCallback;

    // State
    bool m_placementActive;
    int m_lastMouseX, m_lastMouseY;

    // Interaction mode management
    InteractionMode m_interactionMode;
    bool m_shouldBlockCamera;

    // Plane locking state
    bool m_planeLocked;
    float m_planeConstraintOffset;  // Fixed position along the plane normal
    glm::vec3 m_lastUnconstrainedPosition;  // For smooth transitions
    glm::vec3 m_constrainedPosition;  // Current constrained position when plane locked
    bool m_hasConstrainedPosition;  // Whether we have an active constraint
    
    // Viewport information for ray calculations
    int m_viewportWidth;
    int m_viewportHeight;

public:
    PlacementMixin(GLFWSceneWindow *window);
    virtual ~PlacementMixin();

    // Setup and initialization
    void initialize();
    void cleanup();

    // Activation/deactivation
    void setPlacementActive(bool active);
    bool isPlacementActive() const
    {
        return m_placementActive;
    }

    // Interaction mode management
    void setInteractionMode(InteractionMode mode);
    InteractionMode currentInteractionMode() const;
    bool shouldBlockCameraManipulation() const;

    // Plane locking functionality
    void setPlaneLocked(bool locked);
    bool isPlaneLocked() const;
    void setPlaneConstraintOffset(float offset);
    float planeConstraintOffset() const;

    // Visual feedback
    void setCursorVisible(bool visible);
    bool isCursorVisible() const
    {
        return m_showCursor;
    }
    void setPlacementGridVisible(bool visible);
    bool isPlacementGridVisible() const
    {
        return m_showPlacementGrid;
    }

    // Cursor appearance
    void setCursorRadius(float radius);
    void setCursorMaterial(ivf::MaterialPtr material);

    // Grid settings
    void setGridSize(float size);
    void setGridTicks(int x, int y, int z);
    void setGridSpacing(float x, float y, float z);

    // Placement settings
    void setPlane(PlacementPlane plane);
    PlacementPlane currentPlane() const;
    void setMode(PlacementMode mode);
    PlacementMode currentMode() const;
    void setPlaneOffset(float offset);
    void setGridSnapping(bool enabled, float gridSize = 1.0f);
    bool gridSnappingEnabled() const;
    float gridSize() const;

    // Node management
    void selectNode(ivf::NodePtr node);
    void clearSelection();
    const std::vector<ivf::NodePtr> &selectedNodes() const;

    // Callback setters
    void setNodeCreationCallback(NodeCreationCallback callback)
    {
        m_nodeCreationCallback = callback;
    }
    void setNodeSelectionCallback(NodeSelectionCallback callback)
    {
        m_nodeSelectionCallback = callback;
    }
    void setNodeMovedCallback(NodeMovedCallback callback)
    {
        m_nodeMovedCallback = callback;
    }

    // Event handlers (to be called from GLFWSceneWindow)
    void handleMouseButton(int button, int action, int mods);
    void handleMouseMove(double x, double y);
    void handleKey(int key, int scancode, int action, int mods);
    void handleResize(int width, int height);

    // Utility methods
    glm::vec3 currentCursorPosition() const;

    // Grid access
    ivf::GridPtr placementGrid() const
    {
        return m_placementGrid;
    }

    // Grid plane control (for manual override if needed)
    void setGridPlane(ivf::GridPlane plane);
    ivf::GridPlane currentGridPlane() const;

    // UI interaction detection
    bool isImGuiCapturingMouse() const;
    bool isImGuiCapturingKeyboard() const;

private:
    void setupCursor();
    void setupPlacementGrid();
    void updateCursorPosition();
    void updatePlacementGrid();
    void updateInteractionMode(int button, int action, int mods);
    void updateCameraModeHint();
    bool shouldHandleInAutoMode(int button, int action, int mods) const;

    // Plane locking helpers
    void applyCursorPlaneLocking();
    std::string getPlaneDescription() const;
    void setCursorWorldPosition(const glm::vec3& position);
    void updatePlacementManipulatorPosition(const glm::vec3& position);
    glm::vec3 calculatePerpendicularMovement();
    glm::vec3 calculatePerpendicularMovementFromMouse();
    PlacementPlane getBestPerpendicularPlane(const glm::vec3& cameraDirection);
    void getRayFromScreen(double screenX, double screenY, glm::vec3& rayOrigin, glm::vec3& rayDirection);
    void createPerpendicularPlane(PlacementPlane plane, glm::vec3& planePoint, glm::vec3& planeNormal);
    glm::vec3 rayPlaneIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, 
                                   const glm::vec3& planePoint, const glm::vec3& planeNormal);
    glm::vec3 rayLineIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                                  const glm::vec3& linePoint, const glm::vec3& lineDirection);
    
    // Internal callbacks
    void onCursorMove(float x, float y, float z);
    void onCursorClick(float x, float y, float z, int button, int action, int mods);
    void onNodeDrag(float x, float y, float z, float dx, float dy, float dz);
    void onNodeSelect(ivf::NodePtr node);
};

using PlacementMixinPtr = std::shared_ptr<PlacementMixin>;

} // namespace ivfui
