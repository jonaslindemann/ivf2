#include "bend_window.h"

using namespace ivfui;

BendWindow::BendWindow() : UiWindow("Bend properties"), m_curvature(2.0f), m_startDistance(-1.0f), m_endDistance(1.0f)
{}

std::shared_ptr<BendWindow> BendWindow::create()
{
    return std::make_shared<BendWindow>();
}

void BendWindow::doDraw()
{
    ImGui::SliderFloat("Curvature", &m_curvature, 0.1f, 10.0f);
    ImGui::SliderFloat("Start Distance", &m_startDistance, -5.0f, 5.0f);
    ImGui::SliderFloat("End Distance", &m_endDistance, -10.0f, 10.0f);
    ImGui::InputFloat3("Twist Center", m_center, "%.2f");
    ImGui::InputFloat3("Twist Axis", m_axis, "%.2f");
}

float ivfui::BendWindow::curvature() const
{
    return m_curvature;
}

void ivfui::BendWindow::setCurvature(float curvature)
{
    m_curvature = curvature;
}

float ivfui::BendWindow::startDistance() const
{
    return m_startDistance;
}

void ivfui::BendWindow::setStartDistance(float startDistance)
{
    m_startDistance = startDistance;
}

float ivfui::BendWindow::endDistance() const
{
    return m_endDistance;
}

void ivfui::BendWindow::setEndDistance(float endDistance)
{
    m_endDistance = endDistance;
}

const float *BendWindow::center() const
{
    return m_center;
}

void BendWindow::setCenter(const float center[3])
{
    m_center[0] = center[0];
    m_center[1] = center[1];
    m_center[2] = center[2];
}

const float *BendWindow::axis() const
{
    return m_axis;
}

void BendWindow::setAxis(const float axis[3])
{
    m_axis[0] = axis[0];
    m_axis[1] = axis[1];
    m_axis[2] = axis[2];
}
