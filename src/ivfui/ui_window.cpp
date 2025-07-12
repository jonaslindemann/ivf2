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
        ImGui::Begin(m_name.c_str(), 0, doWindowFlags()); //, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        doDraw();
        ImGui::End();
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

void ivfui::UiWindow::doDraw()
{}

void ivfui::UiWindow::doUpdate()
{}

ImGuiWindowFlags ivfui::UiWindow::doWindowFlags() const
{
    return ImGuiWindowFlags_AlwaysAutoResize;
}
