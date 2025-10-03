#include <ivfui/camera_manipulator.h>

#include <ivf/transform_manager.h>
#include <ivf/extent_visitor.h>

#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace ivfui;
using namespace ivf;
using namespace std;

CameraManipulator::CameraManipulator(GLFWwindow *window)
    : m_window(window), m_width(-1), m_height(-1), m_firstClick(true), m_firstTime(true), m_mouseX(-1), m_mouseY(-1),
      m_mouseStartX(-1), m_mouseStartY(-1), m_cameraTarget(glm::vec3(0.0, 0.0, 0.0)),
      m_cameraPosition(glm::vec3(0.0, 0.0, 5.0)), m_cameraNewPos(m_cameraPosition), m_cameraNewTarget(m_cameraTarget),
      m_leftMouseButton(false), m_middleMouseButton(false), m_rightMouseButton(false), m_anyMouseButton(false),
      m_shiftKey(false), m_ctrlKey(false), m_altKey(false), m_fov(45.0), m_nearZ(1.0), m_farZ(100.0),
      m_mouseScaleX(0.01), m_mouseScaleY(0.01), m_headlight(nullptr), m_savedCameraTarget(m_cameraTarget),
      m_savedCameraPosition(m_cameraPosition), m_manipulationBlocked(false)
{}

std::shared_ptr<CameraManipulator> ivfui::CameraManipulator::create(GLFWwindow *window)
{
    return std::make_shared<CameraManipulator>(window);
}

void ivfui::CameraManipulator::updateProjectMatrix()
{
    auto xfmMgr = TransformManager::instance();

    int width, height;

    glfwGetFramebufferSize(m_window, &width, &height);
    m_width = width;
    m_height = height;

    xfmMgr->enableProjectionMatrix();
    xfmMgr->identity();
    xfmMgr->perspective(m_fov, (GLfloat)width / (GLfloat)height, m_nearZ, m_farZ);
    xfmMgr->enableModelMatrix();
}

void ivfui::CameraManipulator::updateLookAt()
{
    auto xfmMgr = TransformManager::instance();

    xfmMgr->enableViewMatrix();
    xfmMgr->identity();
    xfmMgr->lookAt(m_cameraNewPos, m_cameraNewTarget);
    xfmMgr->enableModelMatrix();
}

void CameraManipulator::update()
{
    TransformManager *xfmMgr = TransformManager::instance();

    int width, height;

    glfwGetFramebufferSize(m_window, &width, &height);

    if ((width != m_width) || (height != m_height))
    {
        m_width = width;
        m_height = height;
        glViewport(0, 0, m_width, m_height);

        xfmMgr->enableProjectionMatrix();
        xfmMgr->identity();
        xfmMgr->perspective(m_fov, (GLfloat)width / (GLfloat)height, m_nearZ, m_farZ);
        xfmMgr->enableModelMatrix();
        LightManager::instance()->apply();
    }

    if (m_firstTime)
    {
        xfmMgr->enableViewMatrix();
        xfmMgr->identity();
        xfmMgr->lookAt(m_cameraPosition, m_cameraTarget);
        xfmMgr->enableModelMatrix();
        LightManager::instance()->apply();
        m_firstTime = false;
    }

    // Skip mouse manipulation if blocked
    if (m_manipulationBlocked)
    {
        return;
    }

    m_leftMouseButton = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    m_middleMouseButton = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    m_rightMouseButton = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    m_anyMouseButton = m_leftMouseButton || m_middleMouseButton || m_rightMouseButton;

    m_shiftKey = (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

    if (m_anyMouseButton)
    {
        glfwGetWindowSize(m_window, &m_width, &m_height);

        if (m_firstClick)
        {
            glfwGetCursorPos(m_window, &m_mouseStartX, &m_mouseStartY);
            m_firstClick = false;
            m_cameraNewPos = m_cameraPosition;
            m_cameraNewTarget = m_cameraTarget;

            m_mouseX = m_mouseStartX;
            m_mouseY = m_mouseStartY;
        }
        else
        {
            glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);
        }
    }

    if (m_leftMouseButton)
    {
        double rotX = 0.01 * (m_mouseX - m_mouseStartX);
        double rotY = 0.01 * (m_mouseY - m_mouseStartY);

        glm::vec3 forward = m_cameraPosition - m_cameraTarget;
        glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
        glm::vec3 right = glm::cross(up, forward);
        glm::vec3 realUp = glm::cross(forward, right);

        glm::vec3 rotForward = glm::rotate(forward, float(-rotY), right);
        m_cameraNewPos = m_cameraTarget + rotForward;
        rotForward = glm::rotate(rotForward, float(-rotX), up);
        m_cameraNewPos = m_cameraTarget + rotForward;

        xfmMgr->enableViewMatrix();
        xfmMgr->identity();
        xfmMgr->lookAt(m_cameraNewPos, m_cameraNewTarget);

        if (m_headlight)
        {
            m_headlight->setDirection(glm::normalize(m_cameraTarget - m_cameraNewPos));
        }
        LightManager::instance()->apply();

        xfmMgr->enableModelMatrix();
    }
    else if ((m_rightMouseButton) && (!m_shiftKey))
    {
        double movX = -m_mouseScaleX * (m_mouseX - m_mouseStartX);
        double movY = m_mouseScaleY * (m_mouseY - m_mouseStartY);

        glm::vec3 forward = glm::normalize(m_cameraPosition - m_cameraTarget);
        glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        glm::vec3 realUp = glm::normalize(glm::cross(forward, right));

        m_cameraNewPos = m_cameraPosition + float(movX) * right + float(movY) * realUp;
        m_cameraNewTarget = m_cameraTarget + float(movX) * right + float(movY) * realUp;

        xfmMgr->enableViewMatrix();
        xfmMgr->identity();
        xfmMgr->lookAt(m_cameraNewPos, m_cameraNewTarget);

        if (m_headlight)
        {
            m_headlight->setDirection(glm::normalize(m_cameraTarget - m_cameraNewPos));
        }

        LightManager::instance()->apply();
        xfmMgr->enableModelMatrix();
    }
    else if ((m_middleMouseButton) || ((m_rightMouseButton) && (m_shiftKey)))
    {
        double movX = -m_mouseScaleX * (m_mouseX - m_mouseStartX);
        double movY = m_mouseScaleY * (m_mouseY - m_mouseStartY);

        glm::vec3 forward = glm::normalize(m_cameraPosition - m_cameraTarget);
        glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        glm::vec3 realUp = glm::normalize(glm::cross(forward, right));

        m_cameraNewPos = m_cameraPosition + float(movY) * forward;

        glm::vec3 posTargetDist = m_cameraNewPos - m_cameraNewTarget;

        if (glm::length(posTargetDist) > 1.5f)
            m_cameraNewTarget = m_cameraTarget;
        else
        {
            m_cameraNewTarget = m_cameraTarget + float(movY + 1.5f) * forward;
            m_cameraTarget = m_cameraNewTarget;
        }

        xfmMgr->enableViewMatrix();
        xfmMgr->identity();
        xfmMgr->lookAt(m_cameraNewPos, m_cameraNewTarget);

        if (m_headlight)
        {
            m_headlight->setDirection(glm::normalize(m_cameraTarget - m_cameraNewPos));
        }
        LightManager::instance()->apply();

        xfmMgr->enableModelMatrix();
    }
    else if ((glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) ||
             (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) ||
             (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE))
    {
        if (!m_firstClick)
        {
            m_cameraPosition = m_cameraNewPos;
            m_cameraTarget = m_cameraNewTarget;
            m_firstClick = true;
        }
    }
}

glm::vec3 ivfui::CameraManipulator::cameraTarget()
{
    return m_cameraNewTarget;
}

glm::vec3 ivfui::CameraManipulator::cameraPosition()
{
    return m_cameraNewPos;
}

void ivfui::CameraManipulator::setCameraTarget(glm::vec3 target)
{
    m_cameraNewTarget = target;
    m_cameraTarget = target;
    updateLookAt();
}

void ivfui::CameraManipulator::setCameraPosition(glm::vec3 position)
{
    m_cameraNewPos = position;
    m_cameraPosition = position;
    updateLookAt();
}

void ivfui::CameraManipulator::setFov(double fov)
{
    m_fov = fov;
    updateProjectMatrix();
}

void ivfui::CameraManipulator::setNearZ(double nearZ)
{
    m_nearZ = nearZ;
    updateProjectMatrix();
}

void ivfui::CameraManipulator::setFarZ(double farZ)
{
    m_farZ = farZ;
    updateProjectMatrix();
}

double ivfui::CameraManipulator::fov()
{
    return m_fov;
}

double ivfui::CameraManipulator::nearZ()
{
    return m_nearZ;
}

double ivfui::CameraManipulator::farZ()
{
    return m_farZ;
}

void ivfui::CameraManipulator::reset()
{
    restoreState();
    updateLookAt();
    update();
}

void ivfui::CameraManipulator::saveState()
{
    m_savedCameraPosition = m_cameraPosition;
    m_savedCameraTarget = m_cameraTarget;
}

void ivfui::CameraManipulator::restoreState()
{
    m_cameraPosition = m_savedCameraPosition;
    m_cameraNewPos = m_savedCameraPosition;
    m_cameraTarget = m_savedCameraTarget;
    m_cameraNewTarget = m_savedCameraTarget;
    updateLookAt();
}

void ivfui::CameraManipulator::setMouseScaling(double sx, double sy)
{
    m_mouseScaleX = sx;
    m_mouseScaleY = sy;
}

void ivfui::CameraManipulator::setHeadlight(ivf::DirectionalLightPtr dirLight)
{
    m_headlight = dirLight;

    if (m_headlight)
    {
        m_headlight->setDirection(glm::normalize(m_cameraTarget - m_cameraNewPos));
        LightManager::instance()->apply();
    }
}

ivf::DirectionalLightPtr ivfui::CameraManipulator::headlight()
{
    return m_headlight;
}

void ivfui::CameraManipulator::zoomToExtent(ivf::CompositeNodePtr scene, bool includeInvisible)
{
    if (!scene)
        return;

    // Use ExtentVisitor to compute the bounding box of the scene
    ivf::ExtentVisitor extentVisitor(includeInvisible);
    scene->accept(&extentVisitor);

    auto bbox = extentVisitor.bbox();

    if (!bbox.isValid())
    {
        // If no valid bounding box, reset to default view
        setCameraPosition(glm::vec3(0.0, 0.0, 5.0));
        setCameraTarget(glm::vec3(0.0, 0.0, 0.0));
        return;
    }

    // Calculate camera position based on bounding box
    glm::vec3 center = bbox.center();
    glm::vec3 size = bbox.size();

    // Calculate distance needed to fit the object in view
    float maxDim = std::max({size.x, size.y, size.z});
    float distance = maxDim / (2.0f * tan(glm::radians(m_fov * 0.5f))) * 1.2f; // Add 20% margin

    // Position camera looking at center from a good angle
    glm::vec3 cameraPos = center + glm::vec3(distance * 0.7f, distance * 0.5f, distance);

    // Update camera parameters
    setCameraTarget(center);
    setCameraPosition(cameraPos);

    // Adjust near/far planes based on distance
    setNearZ(distance * 0.01f);
    setFarZ(distance * 10.0f);

    // Save this as the default state
    saveState();
}

void ivfui::CameraManipulator::saveStateToSlot(int slot)
{
    if (slot < 0 || slot >= 10)
        return;

    m_viewSlots[slot].position = m_cameraPosition;
    m_viewSlots[slot].target = m_cameraTarget;
    m_viewSlots[slot].fov = m_fov;
    m_viewSlots[slot].nearZ = m_nearZ;
    m_viewSlots[slot].farZ = m_farZ;
    m_viewSlots[slot].hasData = true;
}

void ivfui::CameraManipulator::restoreStateFromSlot(int slot)
{
    if (slot < 0 || slot >= 10 || !m_viewSlots[slot].hasData)
        return;

    setCameraPosition(m_viewSlots[slot].position);
    setCameraTarget(m_viewSlots[slot].target);
    setFov(m_viewSlots[slot].fov);
    setNearZ(m_viewSlots[slot].nearZ);
    setFarZ(m_viewSlots[slot].farZ);
}

bool ivfui::CameraManipulator::hasSlotData(int slot) const
{
    if (slot < 0 || slot >= 10)
        return false;
    return m_viewSlots[slot].hasData;
}

void ivfui::CameraManipulator::setManipulationBlocked(bool blocked)
{
    m_manipulationBlocked = blocked;
}

bool ivfui::CameraManipulator::isManipulationBlocked() const
{
    return m_manipulationBlocked;
}

glm::vec3 ivfui::CameraManipulator::computeMouseRay(int mouseX, int mouseY)
{
    if (m_width <= 0 || m_height <= 0)
        return glm::vec3(0.0f, 0.0f, -1.0f); // Default forward vector
    // Convert mouse coordinates to normalized device coordinates (NDC)
    float x = (2.0f * mouseX) / m_width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / m_height; // Invert Y for OpenGL
    float z = 1.0f;                              // Forward direction in NDC
    glm::vec3 rayNDC(x, y, z);
    // Convert NDC to clip space
    glm::vec4 rayClip(rayNDC.x, rayNDC.y, -1.0f, 1.0f);
    // Convert clip space to eye space
    glm::mat4 projMatrix = TransformManager::instance()->projectionMatrix();
    glm::mat4 invProj = glm::inverse(projMatrix);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f); // Set forward direction
    // Convert eye space to world space
    glm::mat4 viewMatrix = TransformManager::instance()->viewMatrix();
    glm::mat4 invView = glm::inverse(viewMatrix);
    glm::vec4 rayWorld = invView * rayEye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld));
    return rayDir;
}
