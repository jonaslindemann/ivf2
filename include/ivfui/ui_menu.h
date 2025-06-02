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
    bool m_selected{false};
    bool m_useSelection{false};
    std::function<void()> m_action;

public:
    UiMenuItem(const std::string &name, const std::string &shortcut = "", const std::function<void()> action = nullptr);

    static std::shared_ptr<UiMenuItem> create(const std::string &name, const std::string &shortcut = "",
                                              const std::function<void()> action = nullptr);

    void draw();

    const std::string &name() const;
    void setShortcut(const std::string &shortcut);
    const std::string &shortcut() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setSelected(bool selected);
    bool isSelected() const;

protected:
    virtual void doDraw();
};

typedef std::shared_ptr<UiMenuItem> UiMenuItemPtr;

class UiMenu {
private:
    std::string m_name;
    std::vector<std::shared_ptr<UiMenuItem>> m_items;

public:
    UiMenu(const std::string &name);

    static std::shared_ptr<UiMenu> create(const std::string &name);

    void addItem(const std::shared_ptr<UiMenuItem> &item);
    void draw();
    const std::string &name() const;
    const std::vector<std::shared_ptr<UiMenuItem>> &items() const;

protected:
    virtual void doDraw();
};

typedef std::shared_ptr<UiMenu> UiMenuPtr;

class UiMainMenu {
private:
    std::vector<std::shared_ptr<UiMenu>> m_menus;

public:
    UiMainMenu();
    virtual ~UiMainMenu();

    static std::shared_ptr<UiMainMenu> create();

    void addMenu(const std::shared_ptr<UiMenu> &menu);
    const std::vector<std::shared_ptr<UiMenu>> &menus() const;

    void draw();

protected:
    virtual void doDraw();
    virtual void doUpdate();
};

typedef std::shared_ptr<UiMainMenu> UiMainMenuPtr;

} // namespace ivfui
