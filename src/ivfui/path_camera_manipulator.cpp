#include <ivfui/path_camera_manipulator.h>

#include <ivfui/camera_manipulator.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

using namespace ivfui;

std::shared_ptr<PathCameraManipulator> PathCameraManipulator::create()
{
    return std::make_shared<PathCameraManipulator>();
}

PathCameraManipulator::Settings &PathCameraManipulator::settings()
{
    return m_settings;
}

const PathCameraManipulator::Settings &PathCameraManipulator::settings() const
{
    return m_settings;
}

void PathCameraManipulator::look(float dx, float dy)
{
    m_yaw = std::clamp(m_yaw - dx * m_settings.lookSpeed, -m_settings.maxYaw, m_settings.maxYaw);
    m_pitch = std::clamp(m_pitch - dy * m_settings.lookSpeed, -m_settings.maxPitch, m_settings.maxPitch);
    m_idle = 0.0;
}

void PathCameraManipulator::move(glm::vec3 delta)
{
    m_offset += delta;
    const float length = glm::length(m_offset);
    if (length > m_settings.maxOffset)
        m_offset *= m_settings.maxOffset / length;
    m_idle = 0.0;
}

void PathCameraManipulator::handleMouseDrag(double dx, double dy, int button)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        look(float(dx), float(dy));
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        move(glm::vec3(float(dx), float(-dy), 0.0f) * m_settings.moveSpeed);
}

void PathCameraManipulator::handleScroll(double yoffset)
{
    move(glm::vec3(0.0f, 0.0f, float(yoffset) * m_settings.scrollSpeed));
}

void PathCameraManipulator::update(double dt)
{
    m_idle += dt;
    if (m_idle < m_settings.returnDelay)
        return;

    const float keep = std::exp(-m_settings.returnRate * float(dt));
    m_yaw *= keep;
    m_pitch *= keep;
    m_offset *= keep;
}

void PathCameraManipulator::reset()
{
    m_yaw = m_pitch = 0.0f;
    m_offset = glm::vec3(0.0f);
    m_idle = 0.0;
}

void PathCameraManipulator::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool PathCameraManipulator::enabled() const
{
    return m_enabled;
}

ivf::TimelineCamera PathCameraManipulator::apply(const ivf::TimelineCamera &path) const
{
    if (!m_enabled)
        return path;

    const glm::vec3 worldUp =
        glm::length(path.up) > 1e-4f ? glm::normalize(path.up) : glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 view = path.target - path.position;
    const float distance = std::max(glm::length(view), 1e-3f);
    const glm::vec3 forward = view / distance;
    glm::vec3 right = glm::cross(forward, worldUp);
    right = glm::length(right) > 1e-4f ? glm::normalize(right) : glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 up = glm::cross(right, forward);

    ivf::TimelineCamera camera = path;
    camera.position = path.position + right * m_offset.x + up * m_offset.y + forward * m_offset.z;
    camera.position.y = std::max(camera.position.y, m_settings.minHeight);

    // Yaw around the world up axis, pitch around the camera's right axis
    const glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), m_yaw, worldUp) * glm::rotate(glm::mat4(1.0f), m_pitch, right);
    const glm::vec3 direction = glm::vec3(rotation * glm::vec4(forward, 0.0f));
    camera.target = camera.position + direction * distance;
    return camera;
}

void PathCameraManipulator::applyTo(const std::shared_ptr<CameraManipulator> &manipulator,
                                    const ivf::TimelineCamera &path) const
{
    if (!manipulator)
        return;

    const ivf::TimelineCamera camera = apply(path);
    manipulator->setCameraPosition(camera.position);
    manipulator->setCameraTarget(camera.target);
}
