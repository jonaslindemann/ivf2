#pragma once

/**
 * @file fly_camera_manipulator.h
 * @brief First-person (fly) camera manipulator for free navigation.
 *
 * @code
 * // In GLFWSceneWindow subclass constructor or doSetup():
 * m_flyCamera = FlyCameraManipulator::create(ref());
 * m_flyCamera->setPosition({0, 2, 10});
 * m_flyCamera->setMoveSpeed(5.0f);
 *
 * // In doUpdateOtherUi() or overridden update path:
 * m_flyCamera->update(frameTime());
 * @endcode
 *
 * Controls:
 *   W/S       — move forward / backward
 *   A/D       — strafe left / right
 *   Q/E       — move down / up
 *   RMB drag  — look (yaw + pitch)
 *   Shift     — move 3× faster
 */

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace ivfui {

class FlyCameraManipulator {
public:
    explicit FlyCameraManipulator(GLFWwindow* window);

    static std::shared_ptr<FlyCameraManipulator> create(GLFWwindow* window);

    // ---- Camera parameters -----------------------------------------------

    void setPosition(glm::vec3 pos);
    void setYaw(float degrees);
    void setPitch(float degrees);
    void setFov(double fov);
    void setNearZ(double nearZ);
    void setFarZ(double farZ);
    void setMoveSpeed(float speed)         { m_moveSpeed    = speed; }
    void setLookSensitivity(float sens)    { m_lookSens     = sens; }

    glm::vec3 position() const { return m_pos; }
    glm::vec3 forward()  const { return m_forward; }
    float yaw()   const { return m_yaw; }
    float pitch() const { return m_pitch; }

    // ---- Call once per frame ---------------------------------------------

    void update(double dt);

private:
    GLFWwindow* m_window;

    glm::vec3 m_pos{0, 0, 5};
    glm::vec3 m_forward{0, 0, -1};
    glm::vec3 m_right{1, 0, 0};
    glm::vec3 m_up{0, 1, 0};

    float m_yaw{-90.0f};   // degrees, -90 = looking along -Z
    float m_pitch{0.0f};   // degrees

    float m_moveSpeed{5.0f};
    float m_lookSens{0.15f};

    double m_fov{45.0};
    double m_nearZ{0.1};
    double m_farZ{1000.0};

    int m_width{-1};
    int m_height{-1};
    bool m_firstTime{true};

    // For right-mouse-button look
    bool   m_looking{false};
    double m_lastMouseX{0.0};
    double m_lastMouseY{0.0};

    void updateVectors();
    void updateProjection();
    void updateView();
};

using FlyCameraManipulatorPtr = std::shared_ptr<FlyCameraManipulator>;

} // namespace ivfui
