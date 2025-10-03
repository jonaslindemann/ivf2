#pragma once

/**
 * @file glfw_scene_window.h
 * @brief Declares the GLFWSceneWindow class for interactive 3D scene rendering and UI in the ivfui library.
 */

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <ivfui/ui.h>
#include <ivf/nodes.h>
#include <ivf/gl.h>
#include <ivf/framebuffer.h>
#include <ivf/program.h>
#include <ivf/post_processor.h>
#include <ivf/effect.h>
#include <ivf/cursor.h>

#include <ivfui/scene_control_panel.h>
#include <ivfui/camera_window.h>
#include <ivfui/effect_inspector.h>
#include <ivfui/ui_menu.h>
#include <ivfui/ui_input_dialog.h>

namespace ivfui {

/**
 * @class GLFWSceneWindow
 * @brief Main window for 3D scene rendering, UI integration, and post-processing effects.
 *
 * The GLFWSceneWindow class provides a comprehensive window for rendering a 3D scene using
 * a scene graph (CompositeNode), camera manipulation, UI overlays, and post-processing effects.
 * It supports node selection, render-to-texture, axis/grid overlays, and integration with
 * custom UI windows and control panels. Inherits from GLFWWindow and implements EffectListProvider
 * to provide effect management capabilities to UI components like EffectInspector.
 */
class GLFWSceneWindow : public GLFWWindow, public EffectListProvider {
private:
    ivf::CompositeNodePtr m_scene;               ///< Root scene node.
    ivfui::CameraManipulatorPtr m_camManip;      ///< Camera manipulator for navigation.
    std::vector<ivfui::UiWindowPtr> m_uiWindows; ///< List of additional UI windows.
    ivf::BufferSelectionPtr m_bufferSelection;   ///< Buffer for selection rendering.
    ivf::FrameBufferPtr m_frameBuffer;           ///< Framebuffer for offscreen rendering.
    ivf::PostProcessorPtr m_postProcessor;       ///< Post-processing pipeline.
    ivfui::UiMainMenuPtr m_mainMenu;             ///< Main menu UI.
    ivf::CursorPtr m_cursor;
    ivf::SpherePtr m_sphere;

    SceneControlPanelPtr m_sceneControlPanel; ///< Scene control panel UI.
    CameraWindowPtr m_cameraWindow;           ///< Camera control window UI.
    EffectInspectorPtr m_effectInspector;     ///< Effect inspector window UI.
    SceneInspectorPtr m_sceneInspector;       ///< Scene inspector for debugging.

    bool m_selectionEnabled{false}; ///< Selection mode enabled.
    ivf::Node *m_lastNode;          ///< Last node under the cursor.
    ivf::Node *m_currentNode;       ///< Current node under the cursor.

    bool m_renderToTexture{false};    ///< Render to texture enabled.
    bool m_selectionRendering{false}; ///< Selection rendering in progress.
    bool m_showAxis{false};           ///< Show axis overlay.
    bool m_showGrid{false};           ///< Show grid overlay.
    bool m_showMainMenu{true};        ///< Show main menu.

    ivf::AxisPtr m_axis; ///< Axis overlay object.
    ivf::GridPtr m_grid; ///< Grid overlay object.

    float m_gridSnapValue{0.1f}; ///< Grid snapping value.
    bool m_snapToGrid{false};    ///< Snap to grid enabled.

    bool m_lockPosXZ{false}; ///< Lock position in XZ plane.

    glm::vec3 m_currentIntersectionPoint; ///< Current intersection point with the grid.

    // Dialog state for grid snap value
    UiInputDialogPtr m_inputDialog; ///< Generic input dialog.

    std::vector<ivf::EffectPtr> m_effects; ///< List of post-processing effects.

public:
    /**
     * @brief Construct a GLFWSceneWindow with the given size and title.
     * @param width Window width in pixels.
     * @param height Window height in pixels.
     * @param title Window title.
     * @param monitor Optional monitor for fullscreen mode.
     * @param shared Optional shared OpenGL context.
     */
    GLFWSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor = nullptr,
                    GLFWwindow *shared = nullptr);

    /**
     * @brief Destructor. Cleans up resources.
     */
    virtual ~GLFWSceneWindow();

    /**
     * @brief Factory method to create a shared pointer to a GLFWSceneWindow instance.
     * @param width Window width in pixels.
     * @param height Window height in pixels.
     * @param title Window title.
     * @param monitor Optional monitor for fullscreen mode.
     * @param shared Optional shared OpenGL context.
     * @return std::shared_ptr<GLFWSceneWindow> New GLFWSceneWindow instance.
     */
    static std::shared_ptr<GLFWSceneWindow> create(int width, int height, const std::string title,
                                                   GLFWmonitor *monitor = nullptr, GLFWwindow *shared = nullptr);

    /**
     * @brief Add a node to the scene.
     * @param node Shared pointer to the node to add.
     */
    void add(ivf::NodePtr node);

    /**
     * @brief Remove a node from the scene.
     * @param node Shared pointer to the node to remove.
     */
    void remove(ivf::NodePtr node);

    /**
     * @brief Remove all nodes from the scene.
     */
    void clear();

    /**
     * @brief Enable or disable selection mode.
     * @param enabled True to enable selection, false to disable.
     */
    void setSelectionEnabled(bool enabled);

    /**
     * @brief Check if selection mode is enabled.
     * @return bool True if selection is enabled.
     */
    bool selectionEnabled();

    /**
     * @brief Enable or disable render-to-texture mode.
     * @param renderToTexture True to enable, false to disable.
     */
    void setRenderToTexture(bool renderToTexture);

    /**
     * @brief Check if render-to-texture mode is enabled.
     * @return bool True if enabled.
     */
    bool renderToTexture();

    /**
     * @brief Add a custom UI window to the scene window.
     * @param uiWindow Shared pointer to the UI window.
     */
    void addUiWindow(ivfui::UiWindowPtr uiWindow);

    /**
     * @brief Add a post-processing effect to the scene.
     * @param effect Shared pointer to the effect.
     */
    void addEffect(ivf::EffectPtr effect);

    // EffectListProvider interface implementation

    /**
     * @brief Get the number of effects in the list.
     * @return int Number of effects.
     */
    virtual int getEffectCount() const override;

    /**
     * @brief Get an effect by index.
     * @param index Effect index.
     * @return std::shared_ptr<ivf::Effect> Effect at the given index, or nullptr if invalid.
     */
    virtual std::shared_ptr<ivf::Effect> getEffect(int index) const override;

    /**
     * @brief Enable a specific effect by index.
     * @param index Effect index.
     */
    virtual void enableEffect(int index) override;

    /**
     * @brief Disable a specific effect by index.
     * @param index Effect index.
     */
    virtual void disableEffect(int index) override;

    /**
     * @brief Check if a specific effect is enabled.
     * @param index Effect index.
     * @return bool True if enabled, false otherwise.
     */
    virtual bool isEffectEnabled(int index) const override;

    /**
     * @brief Disable all effects.
     */
    virtual void disableAllEffects() override;

    /**
     * @brief Remove all effects.
     */
    virtual void clearEffects() override;

    /**
     * @brief Reorder an effect from one index to another.
     * @param fromIndex Source index.
     * @param toIndex Target index.
     * @return bool True if reordering was successful.
     */
    virtual bool reorderEffect(int fromIndex, int toIndex) override;

    // Legacy effect interface methods (for backward compatibility)

    /**
     * @brief Get a specific effect by index (legacy method).
     * @param index Effect index.
     * @return ivf::EffectPtr Shared pointer to the effect.
     */
    ivf::EffectPtr effect(int index);

    /**
     * @brief Enable the headlight (camera-attached directional light).
     */
    void enableHeadlight();

    /**
     * @brief Disable the headlight.
     */
    void disableHeadlight();

    /**
     * @brief Enable the axis overlay.
     */
    void enableAxis();

    /**
     * @brief Disable the axis overlay.
     */
    void disableAxis();

    /**
     * @brief Check if the axis overlay is enabled.
     * @return bool True if enabled.
     */
    bool axisEnabled();

    /**
     * @brief Set the axis overlay visibility.
     * @param visible True to show, false to hide.
     */
    void setAxisVisible(bool visible);

    /**
     * @brief Check if the axis overlay is visible.
     * @return bool True if visible.
     */
    bool axisVisible();

    /**
     * @brief Enable the grid overlay.
     */
    void enableGrid();

    /**
     * @brief Disable the grid overlay.
     */
    void disableGrid();

    /**
     * @brief Check if the grid overlay is enabled.
     * @return bool True if enabled.
     */
    bool gridEnabled();

    /**
     * @brief Set the grid overlay visibility.
     * @param visible True to show, false to hide.
     */
    void setGridVisible(bool visible);

    /**
     * @brief Check if the grid overlay is visible.
     * @return bool True if visible.
     */
    bool gridVisible();

    /**
     * @brief Set the length of the axis overlay.
     * @param length Axis length.
     */
    void setAxisLength(float length);

    /**
     * @brief Set the number of grid ticks along each axis.
     * @param x Number of ticks along X.
     * @param y Number of ticks along Y.
     * @param z Number of ticks along Z.
     */
    void setGridTicks(int x, int y, int z);

    /**
     * @brief Set the grid spacing along each axis.
     * @param x Spacing along X.
     * @param y Spacing along Y.
     * @param z Spacing along Z.
     */
    void setGridSpacing(float x, float y, float z);

    ivf::GridPtr grid();

    /**
     * @brief Get the cursor node.
     * @return ivf::CursorPtr Shared pointer to the cursor.
     */
    ivf::CursorPtr cursor();

    /**
     * @brief Enable the cursor overlay.
     */
    void enableCursor();

    /**
     * @brief Disable the cursor overlay.
     */
    void disableCursor();

    /**
     * @brief Check if the cursor overlay is enabled.
     * @return bool True if enabled.
     */
    bool cursorEnabled();

    /**
     * @brief Set the cursor overlay visibility.
     * @param visible True to show, false to hide.
     */
    void setCursorVisible(bool visible);

    /**
     * @brief Check if the cursor overlay is visible.
     * @return bool True if visible.
     */
    bool cursorVisible();

    /**
     * @brief Set the cursor position.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     */
    void setCursorPosition(float x, float y, float z);

    /**
     * @brief Set the grid snapping value for transformations.
     * @param value Snapping value.
     */
    void setGridSnapValue(float value);

    /**
     * @brief Get the current grid snapping value.
     * @return float Snapping value.
     */
    float gridSnapValue();

    /**
     * @brief Enable or disable snapping to the grid during transformations.
     * @param snap True to enable snapping, false to disable.
     */
    void setSnapToGrid(bool snap);

    /**
     * @brief Check if snapping to the grid is enabled.
     * @return bool True if snapping is enabled.
     */
    bool snapToGrid();

    /**
     * @brief Reset the camera view to the default state.
     */
    void resetView();

    /**
     * @brief Save the current camera view state.
     */
    void saveView();

    /**
     * @brief Show the scene control panel UI.
     */
    void showControlPanel();

    /**
     * @brief Show the camera control window UI.
     */
    void showCameraWindow();

    /**
     * @brief Show the effect inspector window UI.
     */
    void showEffectInspector();

    /**
     * @brief Show scene inspector window UI.
     */
    void showSceneInspector();

    /**
     * @brief Show grid snap value dialog.
     */
    void showGridSnapDialog();

    /**
     * @brief Zoom camera to fit the entire scene.
     * @param includeInvisible Whether to include invisible nodes.
     */
    void zoomToExtent(bool includeInvisible = false);

    void showMainMenu();

    /**
     * @brief Hide the main menu UI.
     */
    void hideMainMenu();

    /**
     * @brief Check if the main menu is currently visible.
     * @return bool True if the main menu is visible.
     */
    bool isMainMenuVisible() const;

    /**
     * @brief Get the root scene node.
     * @return ivf::CompositeNodePtr Shared pointer to the scene.
     */
    ivf::CompositeNodePtr scene();

    /**
     * @brief Get the camera manipulator.
     * @return ivfui::CameraManipulatorPtr Shared pointer to the camera manipulator.
     */
    ivfui::CameraManipulatorPtr cameraManipulator();

    /**
     * @brief Get the main menu UI.
     * @return ivfui::UiMainMenu* Pointer to the main menu.
     */
    ivfui::UiMainMenu *mainMenu();

    /**
     * @brief Called when the UI should be updated (override for custom behavior).
     */
    virtual void onUpdateUi();

    /**
     * @brief Called when effects should be updated (override for custom behavior).
     */
    virtual void onUpdateEffects();

    /**
     * @brief Called when the mouse enters a node (override for custom behavior).
     * @param node Pointer to the node entered.
     */
    virtual void onEnterNode(ivf::Node *node);

    /**
     * @brief Called when the mouse is over a node (override for custom behavior).
     * @param node Pointer to the node.
     */
    virtual void onOverNode(ivf::Node *node);

    /**
     * @brief Called when the mouse leaves a node (override for custom behavior).
     * @param node Pointer to the node left.
     */
    virtual void onLeaveNode(ivf::Node *node);

    virtual void onAddMenuItems(ivfui::UiMenu *menu);

    virtual void onMousePosition3D(double x, double y, double z);

protected:
    /**
     * @brief Internal handler for mouse entering a node.
     * @param node Pointer to the node entered.
     */
    virtual void doEnterNode(ivf::Node *node);

    /**
     * @brief Internal handler for mouse over a node.
     * @param node Pointer to the node.
     */
    virtual void doOverNode(ivf::Node *node);

    /**
     * @brief Internal handler for mouse leaving a node.
     * @param node Pointer to the node left.
     */
    virtual void doLeaveNode(ivf::Node *node);

    /**
     * @brief Internal handler for UI updates.
     */
    virtual void doUpdateUi();

    /**
     * @brief Internal handler for effect updates.
     */
    virtual void doUpdateEffects();

    /**
     * @brief Handle window resize events.
     * @param width New window width.
     * @param height New window height.
     */
    virtual void doResize(int width, int height) override;

    /**
     * @brief Perform setup operations for the window.
     * @return int Status code.
     */
    virtual int doSetup() override;

    virtual void doSetupMainMenu();

    /**
     * @brief Draw the scene (called per frame).
     */
    virtual void doDraw() override;

    /**
     * @brief Draw additional UI elements (called per frame).
     */
    virtual void doUpdateOtherUi() override;

    /**
     * @brief Draw UI overlays (called per frame).
     */
    virtual void doDrawUi() override;

    /**
     * @brief Called after drawing is complete (called per frame).
     */
    virtual void doDrawComplete() override;

    /**
     * @brief Handle key events.
     * @param key Key code.
     * @param scancode Scan code.
     * @param action Key action.
     * @param mods Modifier flags.
     */
    virtual void doKey(int key, int scancode, int action, int mods) override;

    virtual void doMousePosition(double x, double y) override;
};

/**
 * @typedef GLFWSceneWindowPtr
 * @brief Shared pointer type for GLFWSceneWindow.
 */
typedef std::shared_ptr<GLFWSceneWindow> GLFWSceneWindowPtr;

}; // namespace ivfui
