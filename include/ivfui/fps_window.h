#pragma once

/**
 * @file fps_window.h
 * @brief Declares the FpsWindow class for displaying frames-per-second (FPS) in the ivfui library.
 */

#include <ivfui/ui_window.h>

namespace ivfui {

/**
 * @class FpsWindow
 * @brief UI window for displaying the current frames-per-second (FPS) value.
 *
 * The FpsWindow class provides a simple user interface window that displays
 * the current rendering performance in frames per second. It is useful for
 * real-time applications to monitor and debug rendering speed.
 */
class FpsWindow : public ivfui::UiWindow {
private:
public:
    /**
     * @brief Default constructor. Initializes the FPS window.
     */
    FpsWindow();

    /**
     * @brief Factory method to create a shared pointer to an FpsWindow instance.
     * @return std::shared_ptr<FpsWindow> New FpsWindow instance.
     */
    static std::shared_ptr<FpsWindow> create();

    /**
     * @brief Draw the FPS window UI (called per frame).
     */
    void doDraw() override;
};

/**
 * @typedef FpsWindowPtr
 * @brief Shared pointer type for FpsWindow.
 */
typedef std::shared_ptr<FpsWindow> FpsWindowPtr;

} // namespace ivfui
