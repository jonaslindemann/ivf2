#pragma once

/**
 * @file ui_window.h
 * @brief Declares the UiWindow class for ImGui-based window management in the ivfui library.
 */

#include <memory>
#include <string>
#include <imgui.h>

namespace ivfui {

/**
 * @class UiWindow
 * @brief Base class for ImGui-based UI windows.
 *
 * The UiWindow class provides a common interface for user interface windows
 * in the ivfui library. It manages window visibility, drawing, and update logic,
 * and can be extended to implement custom UI windows.
 */
class UiWindow {
private:
    std::string m_name; ///< Window name.
    bool m_visible;     ///< Window visibility state.

    int m_x{-1};      ///< Window position (x and y coordinates).
    int m_y{-1};      ///< Window position (x and y coordinates).
    int m_width{-1};  ///< Window dimensions (width and height).
    int m_height{-1}; ///< Window dimensions (width and height).

public:
    /**
     * @brief Construct a UiWindow with the given name.
     * @param name Window name.
     */
    UiWindow(const std::string name);

    /**
     * @brief Virtual destructor.
     */
    virtual ~UiWindow();

    /**
     * @brief Factory method to create a shared pointer to a UiWindow instance.
     * @param name Window name.
     * @return std::shared_ptr<UiWindow> New UiWindow instance.
     */
    static std::shared_ptr<UiWindow> create(const std::string name);

    /**
     * @brief Draw the window (calls doDraw if visible).
     */
    void draw();

    /**
     * @brief Set the window visibility.
     * @param flag True to show, false to hide.
     */
    void setVisible(bool flag);

    /**
     * @brief Check if the window is visible.
     * @return bool True if visible.
     */
    bool visible();

    /**
     * @brief Show the window.
     */
    void show();

    /**
     * @brief Hide the window.
     */
    void hide();

    /**
     * @brief Update the window state (calls doUpdate).
     */
    void update();

    /**
     * @brief Set the window size.
     * @param width Width in pixels.
     * @param height Height in pixels.
     */
    void setSize(int width, int height);

    /**
     * @brief Get the window width.
     * @return int Width in pixels.
     */
    int width() const;

    /**
     * @brief Get the window height.
     * @return int Height in pixels.
     */
    int height() const;

    /**
     * @brief Set the window position.
     * @param x X coordinate in pixels.
     * @param y Y coordinate in pixels.
     */
    void setPosition(int x, int y);

    /**
     * @brief Get the window X position.
     * @return int X coordinate in pixels.
     */
    int x() const;

    /**
     * @brief Get the window Y position.
     * @return int Y coordinate in pixels.
     */
    int y() const;

protected:
    /**
     * @brief Draw the window contents (override for custom UI).
     */
    virtual void doDraw();

    virtual void doPreDraw();

    virtual void doPostDraw();

    /**
     * @brief Update the window state (override for custom logic).
     */
    virtual void doUpdate();

    virtual ImGuiWindowFlags doWindowFlags() const;
};

/**
 * @typedef UiWindowPtr
 * @brief Shared pointer type for UiWindow.
 */
typedef std::shared_ptr<UiWindow> UiWindowPtr;

} // namespace ivfui
