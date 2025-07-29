#include <ivfui/ui_menu.h>

#include <imgui.h>

using namespace ivfui;

UiMenuItem::UiMenuItem(const std::string &name, const std::string &shortcut, const std::function<void()> action,
                       const std::function<bool()> selected)
    : m_name(name), m_selected(false), m_enabled(true), m_shortcut(shortcut), m_actionCallback(action),
      m_selectedCallback(selected)
{}

std::shared_ptr<UiMenuItem> ivfui::UiMenuItem::create(const std::string &name, const std::string &shortcut,
                                                      const std::function<void()> action,
                                                      const std::function<bool()> selected)
{
    return std::make_shared<UiMenuItem>(name, shortcut, action, selected);
}

void UiMenuItem::draw()
{
    doDraw();
}

const std::string &UiMenuItem::name() const
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

void ivfui::UiMenuItem::doDraw()
{
    if (m_selectedCallback)
    {
        m_selected = m_selectedCallback();

        if (ImGui::MenuItem(m_name.c_str(), m_shortcut.c_str(), &m_selected, &m_enabled))
        {
            if (m_actionCallback)
                m_actionCallback();
        }
    }
    else
    {
        if (ImGui::MenuItem(m_name.c_str(), m_shortcut.c_str(), nullptr, m_enabled))
        {
            if (m_actionCallback)
                m_actionCallback();
        }
    }
}

ivfui::UiMenu::UiMenu(const std::string &name) : m_name(name)
{}

std::shared_ptr<UiMenu> ivfui::UiMenu::create(const std::string &name)
{
    return std::make_shared<UiMenu>(name);
}

void ivfui::UiMenu::addItem(const std::shared_ptr<UiMenuItem> &item)
{
    if (item)
        m_items.push_back(item);
}

void ivfui::UiMenu::draw()
{
    doDraw();
}

const std::string &ivfui::UiMenu::name() const
{
    return m_name;
}

const std::vector<std::shared_ptr<UiMenuItem>> &ivfui::UiMenu::items() const
{
    return m_items;
}

void ivfui::UiMenu::doDraw()
{
    if (ImGui::BeginMenu(m_name.c_str()))
    {
        for (const auto &item : m_items)
        {
            item->draw();
        }
        ImGui::EndMenu();
    }
}

UiMainMenu::UiMainMenu()
{}

UiMainMenu::~UiMainMenu()
{}

std::shared_ptr<UiMainMenu> ivfui::UiMainMenu::create()
{
    return std::make_shared<UiMainMenu>();
}

void ivfui::UiMainMenu::addMenu(const std::shared_ptr<UiMenu> &menu)
{
    if (menu)
        m_menus.push_back(menu);
}

const std::vector<std::shared_ptr<UiMenu>> &ivfui::UiMainMenu::menus() const
{
    return m_menus;
}

void ivfui::UiMainMenu::clear()
{
    m_menus.clear();
}

void ivfui::UiMainMenu::draw()
{
    doDraw();
}

void ivfui::UiMainMenu::doDraw()
{
    if (m_menus.empty())
        return;

    if (ImGui::BeginMainMenuBar())
    {
        for (const auto &menu : m_menus)
        {
            menu->draw();
        }
        ImGui::EndMainMenuBar();
    }
}

void ivfui::UiMainMenu::doUpdate()
{}
