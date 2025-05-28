#pragma once

#include <memory>
#include <string>
#include <functional>

#include <imgui.h>

namespace ivfui {

class UiMenuItem {
private:
    std::string m_name;
    std::string m_shortcut;
    bool m_enabled{true};
    bool m_selected{true};
    std::function<void()> m_action;

public:
    UiMenuItem(const std::string &name);

    void draw();

    const std::string &name() const;
    void setShortcut(const std::string &shortcut);
    const std::string &shortcut() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setSelected(bool selected);
    bool isSelected() const;
};

class UiMainMenu {
private:

public:
    UiMainMenu();
    virtual ~UiMainMenu();

    static std::shared_ptr<UiMainMenu> create();

    void draw();

protected:
    virtual void doDraw();
    virtual void doUpdate();
};

typedef std::shared_ptr<UiMainMenu> UiMainMenuPtr;

} // namespace ivfui
