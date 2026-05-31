#pragma once

/**
 * @file time_control_panel.h
 * @brief Declares the TimeControlPanel UI window for controlling TimeController in the ivfui library.
 */

#include <ivfui/ui_window.h>

namespace ivfui {

/**
 * @class TimeControlPanel
 * @brief ImGui panel for controlling the global TimeController.
 *
 * Provides a pause/resume button, a time-scale slider (0–4×), a reset button,
 * and a live readout of the current scaled elapsed time and frame delta.
 *
 * Add to a GLFWSceneWindow with:
 * @code
 * auto panel = TimeControlPanel::create();
 * addUiWindow(panel);
 * @endcode
 */
class TimeControlPanel : public UiWindow {
public:
    TimeControlPanel();

    static std::shared_ptr<TimeControlPanel> create();

protected:
    void doDraw() override;
};

typedef std::shared_ptr<TimeControlPanel> TimeControlPanelPtr;

} // namespace ivfui
