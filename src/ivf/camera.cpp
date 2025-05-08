#include <ivf/camera.h>

#include <ivf/transform_manager.h>

using namespace ivf;

ivf::Camera::Camera()
    : m_position(glm::vec3(0.0, 0.0, 5.0)), m_target(glm::vec3(0.0, 0.0, 0.0)), m_up(glm::vec3(0.0, 1.0, 0.0)),
      m_fov(45.0f), m_nearZ(1.0f), m_farZ(100.0f), m_perspective(true), m_ortho(false), m_width(-1), m_height(-1)
{}

void ivf::Camera::updateProjectMatrix()
{
    auto xfmMgr = TransformManager::instance();

    glViewport(0, 0, m_width, m_height);

    xfmMgr->enableProjectionMatrix();
    xfmMgr->identity();
    xfmMgr->perspective(m_fov, (GLfloat)m_width / (GLfloat)m_height, m_nearZ, m_farZ);
    xfmMgr->enableModelMatrix();
}

void ivf::Camera::updateLookAt()
{
    auto xfmMgr = TransformManager::instance();

    xfmMgr->enableViewMatrix();
    xfmMgr->identity();
    xfmMgr->lookAt(m_position, m_target, m_up);
    xfmMgr->enableModelMatrix();
}

void ivf::Camera::apply()
{
    updateLookAt();
}

std::shared_ptr<Camera> ivf::Camera::create()
{
    return std::make_shared<Camera>();
}

void ivf::Camera::setPosition(const glm::vec3 &position)
{
    m_position = position;
    updateLookAt();
}

void ivf::Camera::setTarget(const glm::vec3 &target)
{
    m_target = target;
    updateLookAt();
}

void ivf::Camera::setUp(const glm::vec3 &up)
{
    m_up = up;
    updateLookAt();
}

void ivf::Camera::setLookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
{
    m_position = eye;
    m_target = center;
    m_up = up;
    updateLookAt();
}

void ivf::Camera::setFov(float fov)
{
    m_fov = fov;
    updateProjectMatrix();
}

void ivf::Camera::setNearZ(float nearZ)
{
    m_nearZ = nearZ;
    updateProjectMatrix();
}

void ivf::Camera::setFarZ(float farZ)
{
    m_farZ = farZ;
    updateProjectMatrix();
}

void ivf::Camera::setPerspective(float fov, float nearZ, float farZ)
{
    m_fov = fov;
    m_nearZ = nearZ;
    m_farZ = farZ;
    m_perspective = true;
    m_ortho = false;
    updateProjectMatrix();
}

void ivf::Camera::setPerspective(bool perspective)
{
    m_perspective = perspective;
    if (m_perspective)
    {
        m_fov = 45.0f;
        m_nearZ = 1.0f;
        m_farZ = 100.0f;
    }
    else
    {
        m_fov = 0.0f;
        m_nearZ = 0.1f;
        m_farZ = 100.0f;
    }
}

void ivf::Camera::setOrtho(bool ortho)
{
    m_ortho = ortho;
    if (m_ortho)
    {
        m_fov = 0.0f;
        m_nearZ = 0.1f;
        m_farZ = 100.0f;
    }
    else
    {
        m_fov = 45.0f;
        m_nearZ = 1.0f;
        m_farZ = 100.0f;
    }
}

glm::vec3 ivf::Camera::position() const
{
    return m_position;
}

glm::vec3 ivf::Camera::target() const
{
    return m_target;
}

glm::vec3 ivf::Camera::up() const
{
    return m_up;
}

float ivf::Camera::fov() const
{
    return m_fov;
}

float ivf::Camera::nearZ() const
{
    return m_nearZ;
}

float ivf::Camera::farZ() const
{
    return m_farZ;
}

bool ivf::Camera::isPerspective() const
{
    return m_perspective;
}

bool ivf::Camera::isOrtho() const
{
    return m_ortho;
}

void ivf::Camera::setViewport(int width, int height)
{
    m_width = width;
    m_height = height;

    glViewport(0, 0, m_width, m_height);
    updateProjectMatrix();
}

int ivf::Camera::width() const
{
    return m_width;
}

int ivf::Camera::height() const
{
    return m_width;
}
