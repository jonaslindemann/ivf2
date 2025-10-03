#include <ivfui/placement_manipulator.h>
#include <ivf/transform_manager.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

using namespace ivfui;
using namespace ivf;

PlacementManipulator::PlacementManipulator(CameraManipulatorPtr cameraManip)
    : m_cameraManip(cameraManip)
    , m_currentPlane(PlacementPlane::XZ)
    , m_currentMode(PlacementMode::CREATE)
    , m_planeOffset(0.0f)
    , m_viewportWidth(800)
    , m_viewportHeight(600)
    , m_currentWorldPos(0.0f)
    , m_lastWorldPos(0.0f)
    , m_isDragging(false)
    , m_dragStartPos(0.0f)
    , m_snapToGrid(false)
    , m_gridSize(1.0f)
{
}

std::shared_ptr<PlacementManipulator> PlacementManipulator::create(CameraManipulatorPtr cameraManip)
{
    return std::make_shared<PlacementManipulator>(cameraManip);
}

void PlacementManipulator::setPlane(PlacementPlane plane)
{
    m_currentPlane = plane;
}

void PlacementManipulator::setMode(PlacementMode mode)
{
    m_currentMode = mode;
    if (mode != PlacementMode::MOVE) {
        clearSelection();
    }
}

void PlacementManipulator::setViewport(int width, int height)
{
    m_viewportWidth = width;
    m_viewportHeight = height;
}

void PlacementManipulator::updateCursorPosition(double screenX, double screenY)
{
    m_lastWorldPos = m_currentWorldPos;
    m_currentWorldPos = screenToWorld(screenX, screenY);
    
    if (m_snapToGrid) {
        m_currentWorldPos = snapToGrid(m_currentWorldPos);
    }
    
    // Call the move callback if set
    if (m_onMoveCallback) {
        m_onMoveCallback(m_currentWorldPos.x, m_currentWorldPos.y, m_currentWorldPos.z);
    }
}

void PlacementManipulator::handleMouseButton(int button, int action, int mods, double screenX, double screenY)
{
    updateCursorPosition(screenX, screenY);
    
    if (action == GLFW_PRESS) {
        if (m_currentMode == PlacementMode::MOVE && !m_selectedNodes.empty()) {
            // Start dragging selected nodes
            m_isDragging = true;
            m_dragStartPos = m_currentWorldPos;
            
            // Store original positions
            m_originalPositions.clear();
            for (auto& node : m_selectedNodes) {
                // Cast to TransformNode to access pos() method
                if (auto transformNode = std::dynamic_pointer_cast<TransformNode>(node)) {
                    m_originalPositions.push_back(transformNode->pos());
                } else {
                    m_originalPositions.push_back(glm::vec3(0.0f));
                }
            }
        }
    }
    else if (action == GLFW_RELEASE) {
        if (m_isDragging) {
            // Finish dragging
            m_isDragging = false;
            
            // Final position adjustment
            glm::vec3 delta = m_currentWorldPos - m_dragStartPos;
            for (size_t i = 0; i < m_selectedNodes.size(); ++i) {
                glm::vec3 newPos = m_originalPositions[i] + delta;
                if (m_snapToGrid) {
                    newPos = snapToGrid(newPos);
                }
                // Cast to TransformNode to access setPos() method
                if (auto transformNode = std::dynamic_pointer_cast<TransformNode>(m_selectedNodes[i])) {
                    transformNode->setPos(newPos);
                }
            }
        }
    }
    
    // Call the click callback if set
    if (m_onClickCallback) {
        m_onClickCallback(m_currentWorldPos.x, m_currentWorldPos.y, m_currentWorldPos.z, 
                         button, action, mods);
    }
}

void PlacementManipulator::handleMouseMove(double screenX, double screenY)
{
    updateCursorPosition(screenX, screenY);
    
    if (m_isDragging && !m_selectedNodes.empty()) {
        // Update node positions during drag
        glm::vec3 delta = m_currentWorldPos - m_dragStartPos;
        
        for (size_t i = 0; i < m_selectedNodes.size(); ++i) {
            glm::vec3 newPos = m_originalPositions[i] + delta;
            if (m_snapToGrid) {
                newPos = snapToGrid(newPos);
            }
            // Cast to TransformNode to access setPos() method
            if (auto transformNode = std::dynamic_pointer_cast<TransformNode>(m_selectedNodes[i])) {
                transformNode->setPos(newPos);
            }
        }
        
        // Call drag callback
        if (m_onDragCallback) {
            glm::vec3 deltaFromLast = m_currentWorldPos - m_lastWorldPos;
            m_onDragCallback(m_currentWorldPos.x, m_currentWorldPos.y, m_currentWorldPos.z,
                           deltaFromLast.x, deltaFromLast.y, deltaFromLast.z);
        }
    }
}

void PlacementManipulator::selectNode(ivf::NodePtr node)
{
    if (!node) return;
    
    // Clear existing selection and add new node
    m_selectedNodes.clear();
    m_selectedNodes.push_back(node);
    
    if (m_onSelectCallback) {
        m_onSelectCallback(node);
    }
}

void PlacementManipulator::selectNodes(const std::vector<ivf::NodePtr>& nodes)
{
    m_selectedNodes = nodes;
}

void PlacementManipulator::clearSelection()
{
    m_selectedNodes.clear();
}

void PlacementManipulator::setGridSnapping(bool enabled, float gridSize)
{
    m_snapToGrid = enabled;
    m_gridSize = gridSize;
}

glm::vec3 PlacementManipulator::screenToWorld(double screenX, double screenY)
{
    // Get ray from camera through screen point
    glm::vec3 rayOrigin, rayDirection;
    getRayFromScreen(screenX, screenY, rayOrigin, rayDirection);
    
    // Project ray onto current placement plane
    return projectToPlane(rayOrigin, rayDirection);
}

glm::vec3 PlacementManipulator::projectToPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDirection)
{
    glm::vec3 planeNormal = getPlaneNormal();
    glm::vec3 planeOrigin = getPlaneOrigin();
    
    // Calculate intersection with plane
    // Ray: P = rayOrigin + t * rayDirection
    // Plane: dot(P - planeOrigin, planeNormal) = 0
    // Solving: t = dot(planeOrigin - rayOrigin, planeNormal) / dot(rayDirection, planeNormal)
    
    float denominator = glm::dot(rayDirection, planeNormal);
    if (std::abs(denominator) < 1e-6f) {
        // Ray is parallel to plane, return a default position
        return planeOrigin;
    }
    
    float t = glm::dot(planeOrigin - rayOrigin, planeNormal) / denominator;
    return rayOrigin + t * rayDirection;
}

glm::vec3 PlacementManipulator::snapToGrid(const glm::vec3& position)
{
    if (!m_snapToGrid || m_gridSize <= 0.0f) {
        return position;
    }
    
    glm::vec3 snapped;
    snapped.x = std::round(position.x / m_gridSize) * m_gridSize;
    snapped.y = std::round(position.y / m_gridSize) * m_gridSize;
    snapped.z = std::round(position.z / m_gridSize) * m_gridSize;
    
    return snapped;
}

glm::vec3 PlacementManipulator::getPlaneNormal() const
{
    switch (m_currentPlane) {
        case PlacementPlane::XZ:
            return glm::vec3(0.0f, 1.0f, 0.0f);  // Y is up
        case PlacementPlane::XY:
            return glm::vec3(0.0f, 0.0f, 1.0f);  // Z is forward
        case PlacementPlane::YZ:
            return glm::vec3(1.0f, 0.0f, 0.0f);  // X is right
        default:
            return glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

glm::vec3 PlacementManipulator::getPlaneOrigin() const
{
    switch (m_currentPlane) {
        case PlacementPlane::XZ:
            return glm::vec3(0.0f, m_planeOffset, 0.0f);
        case PlacementPlane::XY:
            return glm::vec3(0.0f, 0.0f, m_planeOffset);
        case PlacementPlane::YZ:
            return glm::vec3(m_planeOffset, 0.0f, 0.0f);
        default:
            return glm::vec3(0.0f, m_planeOffset, 0.0f);
    }
}

void PlacementManipulator::getRayFromScreen(double screenX, double screenY, glm::vec3& rayOrigin, glm::vec3& rayDirection)
{
    // Convert screen coordinates to normalized device coordinates (NDC)
    float x = (2.0f * static_cast<float>(screenX)) / m_viewportWidth - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(screenY)) / m_viewportHeight;
    
    // Get camera position and target from camera manipulator
    rayOrigin = m_cameraManip->cameraPosition();
    glm::vec3 cameraTarget = m_cameraManip->cameraTarget();
    
    // Calculate camera's coordinate system
    glm::vec3 forward = glm::normalize(cameraTarget - rayOrigin);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(right, forward));
    
    // Estimate field of view and aspect ratio
    // This is a simplification - in a real implementation you'd get these from the camera
    float fov = 45.0f * glm::pi<float>() / 180.0f;  // 45 degrees in radians
    float aspect = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
    
    // Calculate the ray direction in world space
    float tanHalfFov = tan(fov * 0.5f);
    float u = x * tanHalfFov * aspect;
    float v = y * tanHalfFov;
    
    rayDirection = glm::normalize(forward + u * right + v * up);
}