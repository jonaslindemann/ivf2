#include <ivfui/camera_window.h>

using namespace ivfui;
using namespace ivf;

CameraWindow::CameraWindow(CameraManipulatorPtr camManip, std::string caption)
    : UiWindow(caption), m_cameraManipulator(camManip), m_isDirty(false), m_fov(45.0f), m_nearZ(0.1f),
      m_farZ(100.0f), m_cameraPosition{0.0f, 0.0f, 0.0f}, m_cameraTarget{0.0f, 0.0f, 0.0f}
{
    updateControls();
}

std::shared_ptr<CameraWindow> CameraWindow::create(CameraManipulatorPtr camManip, std::string caption)
{
    return std::make_shared<CameraWindow>(camManip, caption);
}

void ivfui::CameraWindow::updateControls()
{
    m_cameraPosition[0] = m_cameraManipulator->cameraPosition().x;
    m_cameraPosition[1] = m_cameraManipulator->cameraPosition().y;
    m_cameraPosition[2] = m_cameraManipulator->cameraPosition().z;
    m_cameraTarget[0] = m_cameraManipulator->cameraTarget().x;
    m_cameraTarget[1] = m_cameraManipulator->cameraTarget().y;
    m_cameraTarget[2] = m_cameraManipulator->cameraTarget().z;

    m_fov = m_cameraManipulator->fov();
    m_nearZ = m_cameraManipulator->nearZ();
    m_farZ = m_cameraManipulator->farZ();
}

void ivfui::CameraWindow::updateCamera()
{
    m_cameraManipulator->setCameraPosition(glm::vec3(m_cameraPosition[0], m_cameraPosition[1], m_cameraPosition[2]));
    m_cameraManipulator->setCameraTarget(glm::vec3(m_cameraTarget[0], m_cameraTarget[1], m_cameraTarget[2]));
    m_cameraManipulator->setFov(m_fov);
    m_cameraManipulator->setNearZ(m_nearZ);
    m_cameraManipulator->setFarZ(m_farZ);
}

void CameraWindow::doDraw()
{
    if (m_cameraManipulator == nullptr)
        return;

    updateControls();

    if (ImGui::InputFloat3("Camera Position", m_cameraPosition))
    {
        updateCamera();
    }
    if (ImGui::InputFloat3("Camera Target", m_cameraTarget))
    {
        updateCamera();
    }
    if (ImGui::SliderFloat("FOV", &m_fov, 0.0f, 180.0f))
    {
        updateCamera();
    }
    if (ImGui::SliderFloat("Near Z", &m_nearZ, 0.0f, 100.0f))
    {
        updateCamera();
    }
    if (ImGui::SliderFloat("Far Z", &m_farZ, 0.0f, 1000.0f))
    {
        updateCamera();
    }
}

void ivfui::CameraWindow::doUpdate()
{
    if (m_cameraManipulator == nullptr)
        return;
}

bool ivfui::CameraWindow::isDirty()
{
    return m_isDirty;
}
