#pragma once

/**
 * @file enhanced_scene_window.h
 * @brief Enhanced GLFWSceneWindow with integrated 3D placement capabilities.
 */

#include <ivfui/glfw_scene_window.h>
#include <ivfui/placement_mixin.h>
#include <memory>

namespace ivfui {

/**
 * @class EnhancedSceneWindow
 * @brief GLFWSceneWindow with built-in 3D object placement and manipulation.
 *
 * This class extends GLFWSceneWindow to include 3D cursor functionality,
 * object placement, and manipulation capabilities. It demonstrates how to
 * integrate the PlacementMixin with an existing scene window.
 */
class EnhancedSceneWindow : public GLFWSceneWindow {
private:
    std::unique_ptr<PlacementMixin> m_placementMixin;
    bool m_placementEnabled;

public:
    EnhancedSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor = nullptr,
                        GLFWwindow *shared = nullptr);
    virtual ~EnhancedSceneWindow();

    static std::shared_ptr<EnhancedSceneWindow> create(int width, int height, const std::string title,
                                                       GLFWmonitor *monitor = nullptr, GLFWwindow *shared = nullptr);

    // Placement functionality
    void enablePlacement(bool enable = true);
    void disablePlacement()
    {
        enablePlacement(false);
    }
    bool isPlacementEnabled() const
    {
        return m_placementEnabled;
    }

    // Direct access to placement functionality
    PlacementMixin *placementMixin()
    {
        return m_placementMixin.get();
    }

    // Convenience methods
    void setPlacementPlane(PlacementPlane plane);
    void setPlacementMode(PlacementMode mode);
    void setGridSnapping(bool enabled, float gridSize = 1.0f);
    void setCursorVisible(bool visible);
    void setPlacementGridVisible(bool visible);

    // Node creation callback setup
    void setNodeCreationCallback(PlacementMixin::NodeCreationCallback callback);
    void setNodeSelectionCallback(PlacementMixin::NodeSelectionCallback callback);
    void setNodeMovedCallback(PlacementMixin::NodeMovedCallback callback);

protected:
    // Override event handlers to integrate placement functionality
    virtual void onMouseButton(int button, int action, int mods) override;
    virtual void onMousePosition(double x, double y) override;
    virtual void onKey(int key, int scancode, int action, int mods) override;
    virtual void doResize(int width, int height) override;

    // Setup method
    virtual void doPreSetup() override;

    // Menu integration
    virtual void onAddMenuItems(UiMenu *menu) override;
};

using EnhancedSceneWindowPtr = std::shared_ptr<EnhancedSceneWindow>;

} // namespace ivfui
