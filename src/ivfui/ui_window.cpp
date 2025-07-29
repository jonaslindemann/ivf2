#include <ivfui/ui_window.h>

#include <imgui.h>

using namespace ivfui;

UiWindow::UiWindow(const std::string name) : m_name(name), m_visible(true)
{}

UiWindow::~UiWindow()
{}

std::shared_ptr<UiWindow> ivfui::UiWindow::create(const std::string name)
{
    return std::make_shared<UiWindow>(name);
}

void ivfui::UiWindow::draw()
{
    if (m_visible)
    {
        doPreDraw();
        ImGui::Begin(m_name.c_str(), &m_visible, doWindowFlags()); //, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        doDraw();
        ImGui::End();
        doPostDraw();
    }
}

void ivfui::UiWindow::setVisible(bool flag)
{
    m_visible = flag;
}

bool ivfui::UiWindow::visible()
{
    return m_visible;
}

void ivfui::UiWindow::toggleVisibility()
{
    m_visible = !m_visible;
}

void ivfui::UiWindow::show()
{
    m_visible = true;
}

void ivfui::UiWindow::hide()
{
    m_visible = false;
}

void ivfui::UiWindow::update()
{
    doUpdate();
}

void ivfui::UiWindow::setSize(int width, int height)
{
    m_width = width;
    m_height = height;
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height), ImGuiCond_FirstUseEver);
}

int ivfui::UiWindow::width() const
{
    return m_width;
}

int ivfui::UiWindow::height() const
{
    return m_height;
}

void ivfui::UiWindow::setPosition(int x, int y)
{
    m_x = x;
    m_y = y;
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_FirstUseEver);
}

int ivfui::UiWindow::x() const
{
    return m_x;
}

int ivfui::UiWindow::y() const
{
    return m_y;
}

void ivfui::UiWindow::setName(const std::string &name)
{
    m_name = name;
}

const std::string &ivfui::UiWindow::name() const
{
    return m_name;
}

void ivfui::UiWindow::doDraw()
{}

void ivfui::UiWindow::doPreDraw()
{
    // Set initial size and position for the window
    if (m_width > 0 && m_height > 0)
    {
        ImGui::SetNextWindowSize(ImVec2(m_width, m_height), ImGuiCond_FirstUseEver);
    }

    if (m_x >= 0 && m_y >= 0)
    {
        ImGui::SetNextWindowPos(ImVec2(m_x, m_y), ImGuiCond_FirstUseEver);
    }
}

void ivfui::UiWindow::doPostDraw()
{}

void ivfui::UiWindow::doUpdate()
{}

ImGuiWindowFlags ivfui::UiWindow::doWindowFlags() const
{
    return ImGuiWindowFlags_AlwaysAutoResize;
}
