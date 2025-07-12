#pragma once

/**
 * @file scene_control_panel.h
 * @brief Declares the SceneControlPanel class for scene axis/grid control UI in the ivfui library.
 */

#include <memory>
#include <ivfui/ui_window.h>
#include <ivfui/scene_inspector.h>

namespace ivfui {

class GLFWSceneWindow;

/**
 * @class SceneControlPanel
 * @brief UI window for controlling scene axis, grid, and related visualization settings.
 *
 * The SceneControlPanel class provides a user interface for toggling and configuring
 * scene overlays such as axis and grid, as well as axis length and grid tick/spacing parameters.
 * It integrates with a GLFWSceneWindow to apply changes and supports UI-driven updates.
 */
class SceneControlPanel : public ivfui::UiWindow {
private:
    bool m_isDirty{false};                          ///< True if panel settings have been modified via the UI.
    ivfui::GLFWSceneWindow *m_sceneWindow{nullptr}; ///< Associated scene window.
    /**
     * @brief Whether to show the axis overlay.
     * This controls whether the axis visualization is enabled in the scene.
     */
    ivfui::SceneInspectorPtr m_sceneInspector{nullptr}; ///< Associated scene inspector for node selection.

    bool m_showAxis{false}; ///< Show axis overlay.
    bool m_showGrid{false}; ///< Show grid overlay.

    float m_axisLength{1.0f};     ///< Axis length.
    int m_tickX{11};              ///< Number of grid ticks along X.
    int m_tickY{11};              ///< Number of grid ticks along Y.
    int m_tickZ{11};              ///< Number of grid ticks along Z.
    float m_tickSpacingX{1.0e-0}; ///< Grid spacing along X.
    float m_tickSpacingY{1.0e-0}; ///< Grid spacing along Y.
    float m_tickSpacingZ{1.0e-0}; ///< Grid spacing along Z.

public:
    /**
     * @brief Construct a SceneControlPanel with the given caption and scene window.
     * @param caption Window caption.
     * @param sceneWindow Pointer to the associated GLFWSceneWindow.
     */
    SceneControlPanel(std::string caption, GLFWSceneWindow *sceneWindow);

    /**
     * @brief Factory method to create a shared pointer to a SceneControlPanel instance.
     * @param caption Window caption.
     * @param sceneWindow Pointer to the associated GLFWSceneWindow.
     * @return std::shared_ptr<SceneControlPanel> New SceneControlPanel instance.
     */
    static std::shared_ptr<SceneControlPanel> create(std::string caption, GLFWSceneWindow *sceneWindow);

    /**
     * @brief Check if the panel settings have been modified via the UI.
     * @return bool True if dirty, false otherwise.
     */
    bool isDirty();

protected:
    /**
     * @brief Draw the scene control panel UI (called per frame).
     */
    virtual void doDraw() override;

    /**
     * @brief Update the scene control panel state (called per frame).
     */
    virtual void doUpdate() override;
};

/**
 * @typedef SceneControlPanelPtr
 * @brief Shared pointer type for SceneControlPanel.
 */
typedef std::shared_ptr<SceneControlPanel> SceneControlPanelPtr;

} // namespace ivfui
