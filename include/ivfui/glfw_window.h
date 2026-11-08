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

#include <vector>

#include <ivfui/frame_writer.h>
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
    bool m_closeNotified;       ///< True after close callback has been dispatched.
    int m_currentKey;           ///< Last key event.
    std::mutex m_mutex;         ///< Mutex for thread safety.
    double m_t0;                ///< Start time for frame timing.
    double m_t1;                ///< End time for frame timing.
    double m_frameTime;         ///< Last frame time in seconds (wall-clock delta between frames).
    double m_lastFrameTime;     ///< glfwGetTime() at the start of the previous frame.
    int m_frameCount;           ///< Number of frames rendered.
    int m_lastError;            ///< Last error code.
    UiRendererPtr m_uiRenderer; ///< UI renderer for this window.

    // Scroll
    double m_scrollX;           ///< Last scroll X offset.
    double m_scrollY;           ///< Last scroll Y offset.

    // Drag tracking
    double m_prevMouseX;        ///< Mouse X on previous position event.
    double m_prevMouseY;        ///< Mouse Y on previous position event.
    bool m_isDragging;          ///< True while a button is held and mouse moves.

    // Double-click detection
    double m_lastClickTime;     ///< glfwGetTime() of the most recent button press.
    int m_lastClickButton;      ///< Button code of the most recent click.
    static constexpr double k_doubleClickInterval = 0.3; ///< Max seconds between two clicks.

    // Simulation time
    double m_simTime{0.0};       ///< Time of the current frame, see elapsedTime().
    double m_timeOffset{0.0};    ///< glfwGetTime() minus simulation time in real-time mode.
    double m_fixedTimeStep{0.0}; ///< Fixed frame time in seconds, 0 for real time.
    bool m_fixedStepStarted{false};

    // Screenshot / recording
    bool        m_recording{false};       ///< Real-time recording (startRecording).
    bool        m_exporting{false};       ///< Fixed-step export (startFrameExport).
    bool        m_exportPending{false};   ///< Export starts with the next frame.
    double      m_exportStartTime{-1.0};
    std::string m_recordPath;
    int         m_recordFps{60};
    int         m_recordFrameIdx{0};
    int         m_recordedFrames{0};
    int         m_exportFrameCount{0};
    bool        m_recordIncludeUi{true};
    bool        m_exportCloseWhenDone{false};
    double      m_previousFixedTimeStep{0.0};
    double      m_recordAccum{0.0};
    std::string m_screenshotPath;
    bool        m_screenshotIncludeUi{false};
    std::unique_ptr<FrameWriter> m_frameWriter;

    std::vector<unsigned char> readFramePixels(int &width, int &height);
    void captureFrames(bool uiDrawn);
    void finishExport();

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

    void scaleAllSizes(double scale);

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
     * @brief Get the time of the current frame in seconds.
     *
     * Sampled once at the start of each frame, so it is the same everywhere
     * during a frame. Follows the wall clock in real-time mode and advances by
     * exactly the fixed step with setFixedTimeStep() or startFrameExport().
     * @return double Elapsed time.
     */
    double elapsedTime() const;

    /**
     * @brief Get the UI renderer for this window.
     * @return UiRendererPtr Shared pointer to the UI renderer.
     */
    UiRendererPtr uiRenderer() const { return m_uiRenderer; }

    /**
     * @brief Save a PNG screenshot of the next rendered frame.
     *
     * The frame is captured once it has been drawn, so this can be called from
     * onUpdate() or a key handler. The file is written on a background thread.
     * @param path Output file path (should end in .png).
     * @param includeUi Include the ImGui interface in the screenshot.
     */
    void saveScreenshot(const std::string& path, bool includeUi = false);

    /**
     * @brief Start capturing PNG frames in real time, e.g. to record an interactive session.
     *
     * Frames are sampled at @p fps of wall-clock time; when rendering is slower,
     * frames are repeated. For a smooth video at an exact frame rate, use
     * startFrameExport() instead.
     * @param directory Output directory, created if missing. Frames are named frame_000000.png, etc.
     * @param fps Capture rate.
     * @param startFrame Number of the first file, to continue an earlier recording.
     * @param includeUi Include the ImGui interface in the frames.
     */
    void startRecording(const std::string& directory, int fps = 60, int startFrame = 0, bool includeUi = true);

    /**
     * @brief Settings for startFrameExport().
     */
    struct FrameExportSettings {
        std::string directory;     ///< Output directory, created if missing.
        int fps{60};               ///< Frame rate of the exported video.
        int startFrame{0};         ///< Number of the first file (frame_000000.png, ...).
        int frameCount{0};         ///< Frames to export; 0 exports until stopRecording().
        bool includeUi{false};     ///< Include the ImGui interface in the frames.
        bool closeWhenDone{false}; ///< Close the window after frameCount frames.
        double startTime{-1.0};    ///< elapsedTime() of the first frame; negative continues the current time.
    };

    /**
     * @brief Export a PNG sequence at an exact frame rate, for rendering video.
     *
     * Switches to a fixed time step of 1/fps, so every rendered frame advances
     * elapsedTime() and frameTime() by exactly one video frame regardless of how
     * long it takes to render, and every frame is written. Time-driven animation
     * (tweens, timers, behaviors, effects) is therefore identical between runs.
     * The export starts with the next frame; with a startTime, frame k is
     * rendered at exactly startTime + k / fps. Assemble the video with e.g.
     * `ffmpeg -framerate 60 -i frame_%06d.png -i song.wav -c:v libx264 -pix_fmt yuv420p -crf 16 -shortest out.mp4`.
     *
     * The image size is the window's framebuffer size.
     */
    void startFrameExport(const FrameExportSettings &settings);

    /**
     * @brief Stop recording or exporting, and wait until all frames are written.
     */
    void stopRecording();

    /**
     * @brief Check if real-time recording or frame export is active.
     */
    bool isRecording() const { return m_recording || isExporting(); }

    /**
     * @brief Check if a fixed-step frame export is active or about to start.
     */
    bool isExporting() const { return m_exporting || m_exportPending; }

    /**
     * @brief Number of frames captured by the current or last recording/export.
     */
    int recordedFrameCount() const { return m_recordedFrames; }

    /**
     * @brief Advance time by a fixed step per frame instead of wall-clock time.
     *
     * With a fixed step, frameTime() returns @p dt and elapsedTime() advances by
     * @p dt every frame. Switching back to real time continues from the current
     * time without a jump.
     * @param dt Time step in seconds, 0 for real time.
     */
    void setFixedTimeStep(double dt);

    /**
     * @brief Current fixed time step, 0 in real-time mode.
     */
    double fixedTimeStep() const { return m_fixedTimeStep; }

public:
    /**
     * @brief Handle key events (override for custom behavior).
     * @param key Key code.
     * @param scancode Scan code.
     * @param action Key action.
     * @param mods Modifier flags.
     */
    virtual void doScroll(double xoffset, double yoffset);
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
     * @brief Handle window close events (override for custom behavior).
     */
    virtual void doClose();

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

    virtual void doPreSetup();
    /**
     * @brief Perform setup operations for the window (override for custom behavior).
     * @return int Status code.
     */
    virtual int doSetup();

    virtual void doPostSetup();

    /**
     * @brief Key event callback (calls doKey by default).
     * @param key Key code.
     * @param scancode Scan code.
     * @param action Key action.
     * @param mods Modifier flags.
     */
    /** Scroll wheel event. xoffset/yoffset are the scroll deltas. */
    virtual void onScroll(double xoffset, double yoffset);

    /**
     * Mouse drag event — fired every frame the mouse moves while a button is held.
     * dx/dy are the pixel deltas since the last position event.
     */
    virtual void onMouseDrag(double x, double y, double dx, double dy, int button);

    /** Double-click event — fired when the same button is clicked twice within k_doubleClickInterval seconds. */
    virtual void onDoubleClick(int button, int mods);

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
     * @brief Window close callback (calls doClose by default).
     */
    virtual void onClose();

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
     * @brief Called at the start of the first frame of a frame export, before onUpdate().
     *
     * Reset animation state here (tweens, random seeds, song position) so that
     * repeated exports produce identical frames.
     */
    virtual void onFrameExportStarted() {}

    /**
     * @brief Called when a frame export has finished and all frames are written.
     */
    virtual void onFrameExportFinished() {}

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
