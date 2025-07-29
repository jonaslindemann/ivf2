#pragma once

/**
 * @file ui_menu.h
 * @brief Declares menu and main menu classes for ImGui-based UI in the ivfui library.
 */

#include <memory>
#include <string>
#include <functional>
#include <imgui.h>

namespace ivfui {

/**
 * @class UiMenuItem
 * @brief Represents a single menu item in a UI menu.
 *
 * The UiMenuItem class encapsulates a menu entry with a name, optional shortcut,
 * enabled/selected state, and an action callback. It supports drawing itself in an ImGui menu.
 */
class UiMenuItem {
private:
    std::string m_name;                       ///< Menu item name.
    std::string m_shortcut;                   ///< Keyboard shortcut string.
    bool m_enabled{true};                     ///< Enabled state.
    bool m_selected{false};                   ///< Selected state.
    std::function<void()> m_actionCallback;   ///< Action callback.
    std::function<bool()> m_selectedCallback; ///< Selection callback (optional).

public:
    /**
     * @brief Construct a UiMenuItem with name, shortcut, and action.
     * @param name Menu item name.
     * @param shortcut Keyboard shortcut (optional).
     * @param action Callback to invoke when selected (optional).
     */
    UiMenuItem(const std::string &name, const std::string &shortcut = "", const std::function<void()> action = nullptr,
               const std::function<bool()> selected = nullptr);

    /**
     * @brief Factory method to create a shared pointer to a UiMenuItem instance.
     * @param name Menu item name.
     * @param shortcut Keyboard shortcut (optional).
     * @param action Callback to invoke when selected (optional).
     * @return std::shared_ptr<UiMenuItem> New UiMenuItem instance.
     */
    static std::shared_ptr<UiMenuItem> create(const std::string &name, const std::string &shortcut = "",
                                              const std::function<void()> action = nullptr,
                                              const std::function<bool()> selected = nullptr);

    /**
     * @brief Draw the menu item in the current ImGui menu.
     */
    void draw();

    /**
     * @brief Get the menu item name.
     * @return const std::string& Name.
     */
    const std::string &name() const;

    /**
     * @brief Set the keyboard shortcut string.
     * @param shortcut Shortcut string.
     */
    void setShortcut(const std::string &shortcut);

    /**
     * @brief Get the keyboard shortcut string.
     * @return const std::string& Shortcut.
     */
    const std::string &shortcut() const;

    /**
     * @brief Enable or disable the menu item.
     * @param enabled True to enable, false to disable.
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if the menu item is enabled.
     * @return bool True if enabled.
     */
    bool isEnabled() const;

    /**
     * @brief Set the selected state of the menu item.
     * @param selected True if selected.
     */
    void setSelected(bool selected);

    /**
     * @brief Check if the menu item is selected.
     * @return bool True if selected.
     */
    bool isSelected() const;

protected:
    /**
     * @brief Draw the menu item (override for custom behavior).
     */
    virtual void doDraw();
};

/**
 * @typedef UiMenuItemPtr
 * @brief Shared pointer type for UiMenuItem.
 */
typedef std::shared_ptr<UiMenuItem> UiMenuItemPtr;

/**
 * @class UiMenu
 * @brief Represents a menu containing multiple menu items.
 *
 * The UiMenu class manages a collection of UiMenuItem objects and provides
 * methods to add items and draw the menu in an ImGui context.
 */
class UiMenu {
private:
    std::string m_name;                               ///< Menu name.
    std::vector<std::shared_ptr<UiMenuItem>> m_items; ///< Menu items.

public:
    /**
     * @brief Construct a UiMenu with the given name.
     * @param name Menu name.
     */
    UiMenu(const std::string &name);

    /**
     * @brief Factory method to create a shared pointer to a UiMenu instance.
     * @param name Menu name.
     * @return std::shared_ptr<UiMenu> New UiMenu instance.
     */
    static std::shared_ptr<UiMenu> create(const std::string &name);

    /**
     * @brief Add a menu item to the menu.
     * @param item Shared pointer to the menu item.
     */
    void addItem(const std::shared_ptr<UiMenuItem> &item);

    /**
     * @brief Draw the menu and its items in the current ImGui context.
     */
    void draw();

    /**
     * @brief Get the menu name.
     * @return const std::string& Menu name.
     */
    const std::string &name() const;

    /**
     * @brief Get the list of menu items.
     * @return const std::vector<std::shared_ptr<UiMenuItem>>& Menu items.
     */
    const std::vector<std::shared_ptr<UiMenuItem>> &items() const;

protected:
    /**
     * @brief Draw the menu (override for custom behavior).
     */
    virtual void doDraw();
};

/**
 * @typedef UiMenuPtr
 * @brief Shared pointer type for UiMenu.
 */
typedef std::shared_ptr<UiMenu> UiMenuPtr;

/**
 * @class UiMainMenu
 * @brief Represents the main menu bar containing multiple menus.
 *
 * The UiMainMenu class manages a collection of UiMenu objects and provides
 * methods to add menus and draw the main menu bar in an ImGui context.
 */
class UiMainMenu {
private:
    std::vector<std::shared_ptr<UiMenu>> m_menus; ///< List of menus in the main menu bar.

public:
    /**
     * @brief Default constructor.
     */
    UiMainMenu();

    /**
     * @brief Destructor.
     */
    virtual ~UiMainMenu();

    /**
     * @brief Factory method to create a shared pointer to a UiMainMenu instance.
     * @return std::shared_ptr<UiMainMenu> New UiMainMenu instance.
     */
    static std::shared_ptr<UiMainMenu> create();

    /**
     * @brief Add a menu to the main menu bar.
     * @param menu Shared pointer to the menu.
     */
    void addMenu(const std::shared_ptr<UiMenu> &menu);

    /**
     * @brief Get the list of menus in the main menu bar.
     * @return const std::vector<std::shared_ptr<UiMenu>>& Menus.
     */
    const std::vector<std::shared_ptr<UiMenu>> &menus() const;

    /**
     * @brief clera all menus in the main menu bar.
     */
    void clear();

    /**
     * @brief Draw the main menu bar and its menus in the current ImGui context.
     */
    void draw();

protected:
    /**
     * @brief Draw the main menu bar (override for custom behavior).
     */
    virtual void doDraw();

    /**
     * @brief Update the main menu bar state (override for custom behavior).
     */
    virtual void doUpdate();
};

/**
 * @typedef UiMainMenuPtr
 * @brief Shared pointer type for UiMainMenu.
 */
typedef std::shared_ptr<UiMainMenu> UiMainMenuPtr;

} // namespace ivfui
