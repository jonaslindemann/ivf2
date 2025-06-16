#pragma once

/**
 * @file glfw_window.h
 * @brief Declares the GLFWWindow class for window management and event handling in the ivfui library.
 */

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <ivfui/ui_manager.h>

namespace ivfui {

/**
 * @class GLFWWindow
 * @brief Window abstraction for OpenGL rendering and UI integration using GLFW.
 *
 * The GLFWWindow class encapsulates a GLFW window, providing methods for window management,
 * event handling, OpenGL context control, and UI rendering. It supports mouse and keyboard
 * input, window resizing, frame timing, and integration with the ivfui UI system.
 */
class GLFWWindow {
private:
    GLFWwindow *m_window;       ///< Native GLFW window handle.
    GLFWwindow *m_sharedWindow; ///< Shared OpenGL context window.
    GLFWmonitor *m_monitor;     ///< Monitor for fullscreen mode.
    int m_width, m_height;      ///< Window dimensions.
    std::string m_title;        ///< Window title.
    int m_mouseButton;          ///< Last mouse button event.
    int m_mouseAction;          ///< Last mouse action event.
    int m_mouseMods;            ///< Last mouse modifier flags.
    int m_mouseX;               ///< Last mouse X position.
    int m_mouseY;               ///< Last mouse Y position.
    bool m_shiftDown;           ///< Shift key state.
    bool m_ctrlDown;            ///< Control key state.
    bool m_altDown;             ///< Alt key state.
    bool m_escQuit;             ///< ESC key closes window if true.
    bool m_enabled;             ///< Window enabled state.
    bool m_runSetup;            ///< True if setup should be run.
    int m_currentKey;           ///< Last key event.
    std::mutex m_mutex;         ///< Mutex for thread safety.
    double m_t0;                ///< Start time for frame timing.
    double m_t1;                ///< End time for frame timing.
    double m_frameTime;         ///< Last frame time in seconds.
    int m_frameCount;           ///< Number of frames rendered.
    int m_lastError;            ///< Last error code.
    UiRendererPtr m_uiRenderer; ///< UI renderer for this window.

public:
    /**
     * @brief Construct a GLFWWindow with the given size and title.
     * @param width Window width in pixels.
     * @param height Window height in pixels.
     * @param title Window title.
     * @param monitor Optional monitor for fullscreen mode.
     * @param shared Optional shared OpenGL context.
     */
    GLFWWindow(int width, int height, const std::string title, GLFWmonitor *monitor = nullptr,
               GLFWwindow *shared = nullptr);

    /**
     * @brief Destructor. Cleans up window resources.
     */
    virtual ~GLFWWindow();

    /**
     * @brief Factory method to create a shared pointer to a GLFWWindow instance.
     * @param width Window width in pixels.
     * @param height Window height in pixels.
     * @param title Window title.
     * @param monitor Optional monitor for fullscreen mode.
     * @param shared Optional shared OpenGL context.
     * @return std::shared_ptr<GLFWWindow> New GLFWWindow instance.
     */
    static std::shared_ptr<GLFWWindow> create(int width, int height, const std::string title,
                                              GLFWmonitor *monitor = nullptr, GLFWwindow *shared = nullptr);

    /**
     * @brief Make this window's OpenGL context current.
     */
    void makeCurrent();

    /**
     * @brief Check if the window is closing.
     * @return bool True if the window is closing.
     */
    bool isClosing();

    /**
     * @brief Request the window to close.
     */
    void close();

    /**
     * @brief Swap the front and back buffers.
     */
    void swapBuffers();

    /**
     * @brief Destroy the window and release resources.
     */
    void destroy();

    /**
     * @brief Get the window width in pixels.
     * @return int Window width.
     */
    int width();

    /**
     * @brief Get the window height in pixels.
     * @return int Window height.
     */
    int height();

    /**
     * @brief Get the window size.
     * @param width Reference to store width.
     * @param height Reference to store height.
     */
    void getSize(int &width, int &height);

    /**
     * @brief Set the window icon from an image file.
     * @param filename Path to the icon image.
     */
    void setWindowIcon(const std::string filename);

    /**
     * @brief Maximize the window.
     */
    void maximize();

    /**
     * @brief Enable the window (allow rendering and events).
     */
    void enable();

    /**
     * @brief Disable the window (suspend rendering and events).
     */
    void disable();

    /**
     * @brief Check if the window is enabled.
     * @return bool True if enabled.
     */
    bool isEnabled();

    /**
     * @brief Draw the window contents (calls doDraw and doDrawUi).
     */
    void draw();

    /**
     * @brief Draw the 3D scene (calls doDraw).
     */
    void drawScene();

    /**
     * @brief Get the native GLFWwindow pointer.
     * @return GLFWwindow* Native window handle.
     */
    GLFWwindow *ref();

    /**
     * @brief Get the last mouse button event.
     * @return int Mouse button code.
     */
    int mouseButton();

    /**
     * @brief Get the last mouse action event.
     * @return int Mouse action code.
     */
    int mouseAction();

    /**
     * @brief Get the last mouse modifier flags.
     * @return int Modifier flags.
     */
    int mouseMods();

    /**
     * @brief Get the last mouse X position.
     * @return int Mouse X position.
     */
    int mouseX();

    /**
     * @brief Get the last mouse Y position.
     * @return int Mouse Y position.
     */
    int mouseY();

    /**
     * @brief Check if any mouse button is currently pressed.
     * @return bool True if any mouse button is down.
     */
    bool isAnyMouseButtonDown();

    /**
     * @brief Check if the Shift key is currently pressed.
     * @return bool True if Shift is down.
     */
    bool isShiftDown();

    /**
     * @brief Check if the Control key is currently pressed.
     * @return bool True if Control is down.
     */
    bool isCtrlDown();

    /**
     * @brief Check if the Alt key is currently pressed.
     * @return bool True if Alt is down.
     */
    bool isAltDown();

    /**
     * @brief Check if ESC key closes the window.
     * @return bool True if ESC closes the window.
     */
    bool useEscQuit();

    /**
     * @brief Set whether ESC key closes the window.
     * @param flag True to enable, false to disable.
     */
    void setUseEscQuit(bool flag);

    /**
     * @brief Get the last error code.
     * @return int Last error code.
     */
    int lastError() const;

    /**
     * @brief Clear the last error code.
     */
    void clearError();

    /**
     * @brief Set the last error code.
     * @param error Error code to set.
     */
    void setError(int error);

    /**
     * @brief Get the time taken to render the last frame (seconds).
     * @return double Frame time.
     */
    double frameTime() const;

    /**
     * @brief Get the number of frames rendered.
     * @return int Frame count.
     */
    int frameCount() const;

    /**
     * @brief Get the elapsed time since window creation (seconds).
     * @return double Elapsed time.
     */
    double elapsedTime() const;

public:
    /**
     * @brief Handle key events (override for custom behavior).
     * @param key Key code.
     * @param scancode Scan code.
     * @param action Key action.
     * @param mods Modifier flags.
     */
    virtual void doKey(int key, int scancode, int action, int mods);

    /**
     * @brief Handle mouse position events (override for custom behavior).
     * @param x Mouse X position.
     * @param y Mouse Y position.
     */
    virtual void doMousePosition(double x, double y);

    /**
     * @brief Handle mouse button events (override for custom behavior).
     * @param button Mouse button code.
     * @param action Mouse action code.
     * @param mods Modifier flags.
     */
    virtual void doMouseButton(int button, int action, int mods);

    /**
     * @brief Handle window resize events (override for custom behavior).
     * @param width New window width.
     * @param height New window height.
     */
    virtual void doResize(int width, int height);

    /**
     * @brief Update the window state (override for custom behavior).
     */
    virtual void doUpdate();

    /**
     * @brief Draw the window contents (override for custom behavior).
     */
    virtual void doDraw();

    /**
     * @brief Draw UI overlays (override for custom behavior).
     */
    virtual void doDrawUi();

    /**
     * @brief Called after drawing is complete (override for custom behavior).
     */
    virtual void doDrawComplete();

    /**
     * @brief Update other UI elements (override for custom behavior).
     */
    virtual void doUpdateOtherUi();

    /**
     * @brief Perform setup operations for the window (override for custom behavior).
     * @return int Status code.
     */
    virtual int doSetup();

    /**
     * @brief Key event callback (calls doKey by default).
     * @param key Key code.
     * @param scancode Scan code.
     * @param action Key action.
     * @param mods Modifier flags.
     */
    virtual void onKey(int key, int scancode, int action, int mods);

    /**
     * @brief Mouse position event callback (calls doMousePosition by default).
     * @param x Mouse X position.
     * @param y Mouse Y position.
     */
    virtual void onMousePosition(double x, double y);

    /**
     * @brief Mouse button event callback (calls doMouseButton by default).
     * @param button Mouse button code.
     * @param action Mouse action code.
     * @param mods Modifier flags.
     */
    virtual void onMouseButton(int button, int action, int mods);

    /**
     * @brief Window resize event callback (calls doResize by default).
     * @param width New window width.
     * @param height New window height.
     */
    virtual void onResize(int width, int height);

    /**
     * @brief Update callback (calls doUpdate by default).
     */
    virtual void onUpdate();

    /**
     * @brief Draw callback (calls doDraw by default).
     */
    virtual void onDraw();

    /**
     * @brief Draw UI callback (calls doDrawUi by default).
     */
    virtual void onDrawUi();

    /**
     * @brief Draw complete callback (calls doDrawComplete by default).
     */
    virtual void onDrawComplete();

    /**
     * @brief Update other UI callback (calls doUpdateOtherUi by default).
     */
    virtual void onUpdateOtherUi();

    /**
     * @brief Setup callback (calls doSetup by default).
     * @return int Status code.
     */
    virtual int onSetup();
};

/**
 * @typedef GLFWWindowPtr
 * @brief Shared pointer type for GLFWWindow.
 */
typedef std::shared_ptr<GLFWWindow> GLFWWindowPtr;

}; // namespace ivfui
