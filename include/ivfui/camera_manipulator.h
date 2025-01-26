#pragma once

#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <ivf/light_manager.h>

namespace ivfui {

class CameraManipulator {
private:
    GLFWwindow *m_window;
    int m_width;
    int m_height;
    bool m_firstClick;
    bool m_firstTime;
    bool m_leftMouseButton;
    bool m_middleMouseButton;
    bool m_rightMouseButton;
    bool m_anyMouseButton;
    bool m_shiftKey;
    bool m_ctrlKey;
    bool m_altKey;
    double m_mouseStartX;
    double m_mouseStartY;
    double m_mouseX;
    double m_mouseY;
    double m_mouseScaleX;
    double m_mouseScaleY;
    glm::vec3 m_cameraTarget;
    glm::vec3 m_cameraPosition;
    glm::vec3 m_cameraNewPos;
    glm::vec3 m_cameraNewTarget;

    glm::vec3 m_savedCameraTarget;
    glm::vec3 m_savedCameraPosition;

    ivf::DirectionalLightPtr m_headlight;

    double m_fov;
    double m_nearZ;
    double m_farZ;

private:
    void updateProjectMatrix();
    void updateLookAt();

public:
    CameraManipulator(GLFWwindow *window);

    static std::shared_ptr<CameraManipulator> create(GLFWwindow *window);

    void update();

    glm::vec3 cameraTarget();
    glm::vec3 cameraPosition();

    void setCameraTarget(glm::vec3 target);
    void setCameraPosition(glm::vec3 position);

    void setFov(double fov);
    void setNearZ(double nearZ);
    void setFarZ(double farZ);

    double fov();
    double nearZ();
    double farZ();

    void reset();

    void saveState();
    void restoreState();

    void setMouseScaling(double sx, double sy);

    void setHeadlight(ivf::DirectionalLightPtr dirLight);
    ivf::DirectionalLightPtr headlight();
};

typedef std::shared_ptr<CameraManipulator> CameraManipulatorPtr;

} // namespace ivfui
