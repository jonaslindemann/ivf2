#include "twist_window.h"

using namespace ivfui;

TwistWindow::TwistWindow()
    : UiWindow("Twist Example"), m_angle(0.0f), m_falloff(1.0f), m_startDistance(0.0f), m_endDistance(1.0f),
      m_wireframe(false), m_angleIncrement(0.5f)
{}

std::shared_ptr<TwistWindow> TwistWindow::create()
{
    return std::make_shared<TwistWindow>();
}

void TwistWindow::doDraw()
{
    ImGui::SliderFloat("Twist Angle (degrees)", &m_angle, -180.0f, 180.0f);
    ImGui::SliderFloat("Falloff", &m_falloff, 0.1f, 5.0f);
    ImGui::SliderFloat("Start Distance", &m_startDistance, -5.0f, 5.0f);
    ImGui::SliderFloat("End Distance", &m_endDistance, -10.0f, 10.0f);
    ImGui::InputFloat3("Twist Center", m_center, "%.2f");
    ImGui::InputFloat3("Twist Axis", m_axis, "%.2f");
    ImGui::Checkbox("Wireframe Mode", &m_wireframe);
}

float TwistWindow::angle() const
{
    return m_angle;
}

void TwistWindow::setAngle(float angle)
{
    m_angle = angle;
}

float TwistWindow::falloff() const
{
    return m_falloff;
}

void TwistWindow::setFalloff(float falloff)
{
    m_falloff = falloff;
}

float TwistWindow::startDistance() const
{
    return m_startDistance;
}

void TwistWindow::setStartDistance(float startDistance)
{
    m_startDistance = startDistance;
}

float TwistWindow::endDistance() const
{
    return m_endDistance;
}

void TwistWindow::setEndDistance(float endDistance)
{
    m_endDistance = endDistance;
}

bool TwistWindow::wireframe() const
{
    return m_wireframe;
}

void TwistWindow::setWireframe(bool wireframe)
{
    m_wireframe = wireframe;
}

const float *TwistWindow::center() const
{
    return m_center;
}

void TwistWindow::setCenter(const float center[3])
{
    m_center[0] = center[0];
    m_center[1] = center[1];
    m_center[2] = center[2];
}

const float *TwistWindow::axis() const
{
    return m_axis;
}

void TwistWindow::setAxis(const float axis[3])
{
    m_axis[0] = axis[0];
    m_axis[1] = axis[1];
    m_axis[2] = axis[2];
}
