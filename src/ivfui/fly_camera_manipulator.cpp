#include <ivfui/fly_camera_manipulator.h>

#include <ivf/transform_manager.h>
#include <ivf/light_manager.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

using namespace ivfui;
using namespace ivf;

FlyCameraManipulator::FlyCameraManipulator(GLFWwindow* window)
    : m_window(window)
{
    updateVectors();
}

std::shared_ptr<FlyCameraManipulator> FlyCameraManipulator::create(GLFWwindow* window)
{
    return std::make_shared<FlyCameraManipulator>(window);
}

void FlyCameraManipulator::setPosition(glm::vec3 pos)
{
    m_pos = pos;
}

void FlyCameraManipulator::setYaw(float degrees)
{
    m_yaw = degrees;
    updateVectors();
}

void FlyCameraManipulator::setPitch(float degrees)
{
    m_pitch = std::clamp(degrees, -89.0f, 89.0f);
    updateVectors();
}

void FlyCameraManipulator::setFov(double fov)   { m_fov   = fov; }
void FlyCameraManipulator::setNearZ(double n)    { m_nearZ = n; }
void FlyCameraManipulator::setFarZ(double f)     { m_farZ  = f; }

void FlyCameraManipulator::updateVectors()
{
    float yawR   = glm::radians(m_yaw);
    float pitchR = glm::radians(m_pitch);

    m_forward = glm::normalize(glm::vec3(
        std::cos(pitchR) * std::cos(yawR),
        std::sin(pitchR),
        std::cos(pitchR) * std::sin(yawR)));

    m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0, 1, 0)));
    m_up    = glm::normalize(glm::cross(m_right, m_forward));
}

void FlyCameraManipulator::updateProjection()
{
    auto* xfm = TransformManager::instance();
    xfm->enableProjectionMatrix();
    xfm->identity();
    xfm->perspective(m_fov, float(m_width) / float(m_height), m_nearZ, m_farZ);
    xfm->enableModelMatrix();
}

void FlyCameraManipulator::updateView()
{
    auto* xfm = TransformManager::instance();
    xfm->enableViewMatrix();
    xfm->identity();
    xfm->lookAt(m_pos, m_pos + m_forward);
    xfm->enableModelMatrix();
}

void FlyCameraManipulator::update(double dt)
{
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);

    bool sizeChanged = (w != m_width || h != m_height);
    m_width  = w;
    m_height = h;

    if (m_firstTime || sizeChanged) {
        updateProjection();
        LightManager::instance()->apply();
        if (m_firstTime) {
            updateView();
            m_firstTime = false;
        }
    }

    // ---- Right-mouse-button look ----------------------------------------
    bool rmb = (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    double mx, my;
    glfwGetCursorPos(m_window, &mx, &my);

    if (rmb) {
        if (!m_looking) {
            m_lastMouseX = mx;
            m_lastMouseY = my;
            m_looking = true;
        } else {
            float dx = float(mx - m_lastMouseX) * m_lookSens;
            float dy = float(m_lastMouseY - my) * m_lookSens; // inverted Y
            m_lastMouseX = mx;
            m_lastMouseY = my;

            m_yaw   += dx;
            m_pitch  = std::clamp(m_pitch + dy, -89.0f, 89.0f);
            updateVectors();
        }
    } else {
        m_looking = false;
    }

    // ---- Keyboard movement ----------------------------------------------
    float speed = m_moveSpeed * float(dt);
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        speed *= 3.0f;

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) m_pos += m_forward * speed;
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) m_pos -= m_forward * speed;
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) m_pos -= m_right   * speed;
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) m_pos += m_right   * speed;
    if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS) m_pos -= glm::vec3(0,1,0) * speed;
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS) m_pos += glm::vec3(0,1,0) * speed;

    updateView();
}
