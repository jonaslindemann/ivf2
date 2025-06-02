#pragma once

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

#include <ivfui/scene_control_panel.h>
#include <ivfui/camera_window.h>
#include <ivfui/ui_menu.h>

namespace ivfui {

class GLFWSceneWindow : public GLFWWindow {
private:
    ivf::CompositeNodePtr m_scene;
    ivfui::CameraManipulatorPtr m_camManip;
    std::vector<ivfui::UiWindowPtr> m_uiWindows;
    ivf::BufferSelectionPtr m_bufferSelection;
    ivf::FrameBufferPtr m_frameBuffer;
    ivf::PostProcessorPtr m_postProcessor;

    ivfui::UiMainMenuPtr m_mainMenu;

    SceneControlPanelPtr m_sceneControlPanel;
    CameraWindowPtr m_cameraWindow;

    bool m_selectionEnabled{false};
    ivf::Node *m_lastNode;
    ivf::Node *m_currentNode;

    bool m_renderToTexture{false};
    bool m_selectionRendering{false};
    bool m_showAxis{false};
    bool m_showGrid{false};

    ivf::AxisPtr m_axis;
    ivf::GridPtr m_grid;

    std::vector<ivf::EffectPtr> m_effects;

public:
    GLFWSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor = nullptr,
                    GLFWwindow *shared = nullptr);
    virtual ~GLFWSceneWindow();

    static std::shared_ptr<GLFWSceneWindow> create(int width, int height, const std::string title,
                                                   GLFWmonitor *monitor = nullptr, GLFWwindow *shared = nullptr);

    void add(ivf::NodePtr node);
    void remove(ivf::NodePtr node);
    void clear();

    void setSelectionEnabled(bool enabled);
    bool selectionEnabled();

    void setRenderToTexture(bool renderToTexture);
    bool renderToTexture();

    void addUiWindow(ivfui::UiWindowPtr uiWindow);

    void addEffect(ivf::EffectPtr effect);
    void clearEffects();

    void enableHeadlight();
    void disableHeadlight();

    void enableAxis();
    void disableAxis();
    bool axisEnabled();
    void setAxisVisible(bool visible);
    bool axisVisible();

    void enableGrid();
    void disableGrid();
    bool gridEnabled();

    void setGridVisible(bool visible);
    bool gridVisible();

    void setAxisLength(float length);

    void setGridTicks(int x, int y, int z);
    void setGridSpacing(float x, float y, float z);

    void resetView();
    void saveView();

    void showControlPanel();
    void showCameraWindow();

    ivf::CompositeNodePtr scene();

    ivfui::CameraManipulatorPtr cameraManipulator();

    ivfui::UiMainMenu *mainMenu();

    virtual void onUpdateUi();
    virtual void onUpdateEffects();

    virtual void onEnterNode(ivf::Node *node);
    virtual void onOverNode(ivf::Node *node);
    virtual void onLeaveNode(ivf::Node *node);

protected:
    virtual void doEnterNode(ivf::Node *node);
    virtual void doOverNode(ivf::Node *node);
    virtual void doLeaveNode(ivf::Node *node);
    virtual void doUpdateUi();
    virtual void doUpdateEffects();

    virtual void doResize(int width, int height) override;
    virtual int doSetup() override;
    virtual void doDraw() override;
    virtual void doUpdateOtherUi() override;
    virtual void doDrawUi() override;
    virtual void doDrawComplete() override;
    virtual void doKey(int key, int scancode, int action, int mods) override;
};

typedef std::shared_ptr<GLFWSceneWindow> GLFWSceneWindowPtr;

}; // namespace ivfui
