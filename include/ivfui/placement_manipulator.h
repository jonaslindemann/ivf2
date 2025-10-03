#pragma once

/**
 * @file placement_manipulator.h
 * @brief Declares the PlacementManipulator class for 3D object placement and manipulation.
 */

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <memory>
#include <vector>

#include <ivf/nodes.h>
#include <ivf/transform_node.h>
#include <ivfui/camera_manipulator.h>

namespace ivfui {

enum class PlacementPlane {
    XZ = 0,  // Default plane (Y is up)
    XY = 1,  // Z is depth
    YZ = 2   // X is sideways
};

enum class PlacementMode {
    CREATE,  // Creating new objects
    MOVE,    // Moving existing objects
    SELECT   // Just selecting objects
};

/**
 * @class PlacementManipulator
 * @brief Handles 3D object placement and manipulation in a scene.
 * 
 * This class provides functionality for:
 * - Placing nodes at 3D positions using mouse coordinates
 * - Moving existing selected nodes
 * - Switching between different placement planes (XZ, XY, YZ)
 * - Converting between screen coordinates and world coordinates
 * - Providing callbacks for cursor movement and clicks
 */
class PlacementManipulator {
public:
    // Callback function types
    using OnMoveCallback = std::function<void(float x, float y, float z)>;
    using OnClickCallback = std::function<void(float x, float y, float z, int button, int action, int mods)>;
    using OnDragCallback = std::function<void(float x, float y, float z, float dx, float dy, float dz)>;
    using OnSelectCallback = std::function<void(ivf::NodePtr node)>;

private:
    CameraManipulatorPtr m_cameraManip;
    PlacementPlane m_currentPlane;
    PlacementMode m_currentMode;
    float m_planeOffset;  // Offset from origin for current plane
    
    // Viewport information
    int m_viewportWidth;
    int m_viewportHeight;
    
    // Current cursor position in world coordinates
    glm::vec3 m_currentWorldPos;
    glm::vec3 m_lastWorldPos;
    
    // Selected nodes for manipulation
    std::vector<ivf::NodePtr> m_selectedNodes;
    
    // Dragging state
    bool m_isDragging;
    glm::vec3 m_dragStartPos;
    std::vector<glm::vec3> m_originalPositions;
    
    // Callbacks
    OnMoveCallback m_onMoveCallback;
    OnClickCallback m_onClickCallback;
    OnDragCallback m_onDragCallback;
    OnSelectCallback m_onSelectCallback;
    
    // Grid snapping
    bool m_snapToGrid;
    float m_gridSize;
    
public:
    PlacementManipulator(CameraManipulatorPtr cameraManip);
    virtual ~PlacementManipulator() = default;
    
    static std::shared_ptr<PlacementManipulator> create(CameraManipulatorPtr cameraManip);
    
    // Plane management
    void setPlane(PlacementPlane plane);
    PlacementPlane currentPlane() const { return m_currentPlane; }
    void setPlaneOffset(float offset) { m_planeOffset = offset; }
    float planeOffset() const { return m_planeOffset; }
    
    // Mode management
    void setMode(PlacementMode mode);
    PlacementMode currentMode() const { return m_currentMode; }
    
    // Viewport management
    void setViewport(int width, int height);
    
    // Main interaction methods
    void updateCursorPosition(double screenX, double screenY);
    void handleMouseButton(int button, int action, int mods, double screenX, double screenY);
    void handleMouseMove(double screenX, double screenY);
    
    // Node selection and manipulation
    void selectNode(ivf::NodePtr node);
    void selectNodes(const std::vector<ivf::NodePtr>& nodes);
    void clearSelection();
    const std::vector<ivf::NodePtr>& selectedNodes() const { return m_selectedNodes; }
    
    // Grid snapping
    void setGridSnapping(bool enabled, float gridSize = 1.0f);
    bool gridSnappingEnabled() const { return m_snapToGrid; }
    float gridSize() const { return m_gridSize; }
    
    // Current world position
    glm::vec3 currentWorldPosition() const { return m_currentWorldPos; }
    
    // Callback setters
    void setOnMoveCallback(OnMoveCallback callback) { m_onMoveCallback = callback; }
    void setOnClickCallback(OnClickCallback callback) { m_onClickCallback = callback; }
    void setOnDragCallback(OnDragCallback callback) { m_onDragCallback = callback; }
    void setOnSelectCallback(OnSelectCallback callback) { m_onSelectCallback = callback; }
    
private:
    // Coordinate conversion methods
    glm::vec3 screenToWorld(double screenX, double screenY);
    glm::vec3 projectToPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
    glm::vec3 snapToGrid(const glm::vec3& position);
    
    // Helper methods
    glm::vec3 getPlaneNormal() const;
    glm::vec3 getPlaneOrigin() const;
    
    // Ray casting for intersection
    void getRayFromScreen(double screenX, double screenY, glm::vec3& rayOrigin, glm::vec3& rayDirection);
};

using PlacementManipulatorPtr = std::shared_ptr<PlacementManipulator>;

} // namespace ivfui