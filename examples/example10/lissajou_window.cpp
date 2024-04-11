#include "lissajou_window.h"

LissajouWindow::LissajouWindow()
    : UiWindow("Lissajou Control"), m_a(1.0), m_b(1.0), m_c(1.0), m_d(0.7), m_e(3.0), m_f(0.4), m_g(1.0), m_h(2.0),
      m_i(1.0), m_dirty(true), m_speed(1.0), m_size(300)
{}

std::shared_ptr<LissajouWindow> LissajouWindow::create()
{
    return std::make_shared<LissajouWindow>();
}

bool LissajouWindow::is_dirty()
{
    auto retVal = m_dirty;
    m_dirty = false;
    return retVal;
}

void LissajouWindow::get_params(float &a, float &b, float &c, float &d, float &e, float &f, float &g, float &h,
                                float &i)
{
    a = m_a;
    b = m_b;
    c = m_c;
    d = m_d;
    e = m_e;
    f = m_f;
    g = m_g;
    h = m_h;
    i = m_i;
}

float LissajouWindow::speed() const
{
    return m_speed;
}

size_t LissajouWindow::size() const
{
    return m_size;
}

void LissajouWindow::doDraw()
{
    if (ImGui::SliderFloat("a", &m_a, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("b", &m_b, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("c", &m_c, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("d", &m_d, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("e", &m_e, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("f", &m_f, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("g", &m_g, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("h", &m_h, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("i", &m_i, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderFloat("Speed", &m_speed, 0.1f, 10.0f))
        m_dirty = true;
    if (ImGui::SliderInt("Size", &m_size, 10, 1000))
        m_dirty = true;
}
