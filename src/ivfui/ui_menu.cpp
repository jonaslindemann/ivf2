#include <ivfui/ui_menu.h>

#include <imgui.h>

using namespace ivfui;

UiMenuItem::UiMenuItem(const std::string &name) 
    : m_name(name), m_selected(true), m_enabled(true)
{}

void UiMenuItem::draw()
{
    if (ImGui::MenuItem(m_name.c_str(), m_shortcut.c_str(), &m_selected, &m_enabled))
    {
        if (m_action)
            m_action();
    }
}
const std::string& UiMenuItem::name() const
{
    return m_name;
}

void ivfui::UiMenuItem::setShortcut(const std::string &shortcut)
{
    m_shortcut = shortcut;
}

const std::string &ivfui::UiMenuItem::shortcut() const
{
    return m_shortcut;
}

void ivfui::UiMenuItem::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool ivfui::UiMenuItem::isEnabled() const
{
    return m_enabled;
}

void ivfui::UiMenuItem::setSelected(bool selected)
{
    m_selected = selected;
}

bool ivfui::UiMenuItem::isSelected() const
{
    return m_selected;
}

UiMainMenu::UiMainMenu()
{}

UiMainMenu::~UiMainMenu()
{}

std::shared_ptr<UiMainMenu> ivfui::UiMainMenu::create()
{
    return std::make_shared<UiMainMenu>();
}

void ivfui::UiMainMenu::draw()
{

}

void ivfui::UiMainMenu::doDraw()
{}

void ivfui::UiMainMenu::doUpdate()
{}
