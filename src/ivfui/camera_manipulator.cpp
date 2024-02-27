#include <ivfui/camera_manipulator.h>

#include <ivf/transform_manager.h>

#include <glm/gtx/rotate_vector.hpp>

#include <iostream>

using namespace ivfui;
using namespace ivf;
using namespace std;

CameraManipulator::CameraManipulator(GLFWwindow *window)
    : m_window(window), m_width(-1), m_height(-1), m_firstClick(true), m_firstTime(true), m_mouseX(-1), m_mouseY(-1),
      m_mouseStartX(-1), m_mouseStartY(-1), m_cameraTarget(glm::vec3(0.0, 0.0, 0.0)),
      m_cameraPosition(glm::vec3(0.0, 0.0, 5.0)), m_cameraNewPos(m_cameraPosition), m_cameraNewTarget(m_cameraTarget),
      m_leftMouseButton(false), m_middleMouseButton(false), m_rightMouseButton(false), m_anyMouseButton(false),
      m_shiftKey(false), m_ctrlKey(false), m_altKey(false), m_fov(45.0), m_nearZ(1.0), m_farZ(100.0),
      m_mouseScaleX(0.01), m_mouseScaleY(0.01)
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

    /*
    // Handles key inputs
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_position += m_speed * m_orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_position += m_speed * -glm::normalize(glm::cross(m_orientation, m_up));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_position += m_speed * -m_orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_position += m_speed * glm::normalize(glm::cross(m_orientation, m_up));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        m_position += m_speed * m_up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        m_position += m_speed * -m_up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        m_speed = 0.1f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        m_speed = 0.025f;
    }

    */

    // glfwGetWindowSize(m_window, &m_width, &m_height);
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
    }

    if (m_firstTime)
    {
        xfmMgr->enableViewMatrix();
        xfmMgr->identity();
        xfmMgr->lookAt(m_cameraPosition, m_cameraTarget);
        xfmMgr->enableModelMatrix();
        m_firstTime = false;
    }

    m_leftMouseButton = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    m_middleMouseButton = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    m_rightMouseButton = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    m_anyMouseButton = m_leftMouseButton || m_middleMouseButton || m_rightMouseButton;

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

        xfmMgr->enableModelMatrix();
    }
    else if (m_rightMouseButton)
    {
        double movX = -m_mouseScaleX * (m_mouseX - m_mouseStartX);
        double movY = m_mouseScaleY * (m_mouseY - m_mouseStartY);

        glm::vec3 forward = glm::normalize(m_cameraPosition - m_cameraTarget);
        glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        glm::vec3 realUp = glm::normalize(glm::cross(forward, right));

        m_cameraNewPos = m_cameraPosition + float(movX) * right + float(movY) * realUp;
        m_cameraNewTarget = m_cameraTarget + float(movX) * right + float(movY) * realUp;

        std::printf("forward: %f %f %f\n", forward.x, forward.y, forward.z);
        std::printf("up: %f %f %f\n", up.x, up.y, up.z);
        std::printf("right: %f %f %f\n", right.x, right.y, right.z);
        std::printf("realUp: %f %f %f\n", realUp.x, realUp.y, realUp.z);
        std::printf("movX: %f movY: %f\n", movX, movY);

        xfmMgr->enableViewMatrix();
        xfmMgr->identity();
        xfmMgr->lookAt(m_cameraNewPos, m_cameraNewTarget);

        xfmMgr->enableModelMatrix();
    }
    else if (m_middleMouseButton)
    {
        double movX = -m_mouseScaleX * (m_mouseX - m_mouseStartX);
        double movY = m_mouseScaleY * (m_mouseY - m_mouseStartY);

        glm::vec3 forward = glm::normalize(m_cameraPosition - m_cameraTarget);
        glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        glm::vec3 realUp = glm::normalize(glm::cross(forward, right));

        m_cameraNewPos = m_cameraPosition + float(movY) * forward;

        glm::vec3 posTargetDist = m_cameraNewPos - m_cameraNewTarget;

        cout << glm::length(posTargetDist) << endl;

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

void ivfui::CameraManipulator::setMouseScaling(double sx, double sy)
{
    m_mouseScaleX = sx;
    m_mouseScaleY = sy;
}
