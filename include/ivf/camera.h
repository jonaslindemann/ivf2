#pragma once

#include <ivf/glbase.h>

#include <glm/glm.hpp>

namespace ivf {

class Camera : public GLBase {
private:
    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::vec3 m_up;
    float m_fov;
    float m_nearZ;
    float m_farZ;
    bool m_perspective;
    bool m_ortho;
    int m_width;
    int m_height;

    void updateProjectMatrix();
    void updateLookAt();

public:
    Camera();

    static std::shared_ptr<Camera> create();

    void apply();

    void setPosition(const glm::vec3 &position);
    void setTarget(const glm::vec3 &target);
    void setUp(const glm::vec3 &up);
    void setLookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up);

    glm::vec3 position() const;
    glm::vec3 target() const;
    glm::vec3 up() const;

    void setFov(float fov);
    void setNearZ(float nearZ);
    void setFarZ(float farZ);
    void setPerspective(float fov, float nearZ, float farZ);
    void setPerspective(bool perspective);
    void setOrtho(bool ortho);

    float fov() const;
    float nearZ() const;
    float farZ() const;

    bool isPerspective() const;
    bool isOrtho() const;

    void setViewport(int width, int height);
    int width() const;
    int height() const;
};

typedef std::shared_ptr<Camera> CameraPtr;

} // namespace ivf
