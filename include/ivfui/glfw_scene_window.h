#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <ivfui/ui.h>
#include <ivf/nodes.h>
#include <ivf/gl.h>

namespace ivfui {

class GLFWSceneWindow : public GLFWWindow {
private:
    ivf::CompositeNodePtr m_scene;
    ivfui::CameraManipulatorPtr m_camManip;
    std::vector<ivfui::UiWindowPtr> m_uiWindows;
    ivf::BufferSelectionPtr m_bufferSelection;

    bool m_selectionEnabled{false};
    ivf::Node *m_lastNode;
    ivf::Node *m_currentNode;

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

    void addUiWindow(ivfui::UiWindowPtr uiWindow);

    ivf::CompositeNodePtr scene();

    ivfui::CameraManipulatorPtr cameraManipulator();

    virtual void onUpdateUi();

    virtual void onEnterNode(ivf::Node *node);
    virtual void onOverNode(ivf::Node *node);
    virtual void onLeaveNode(ivf::Node *node);

protected:
    virtual void doResize(int width, int height) override;
    virtual void doEnterNode(ivf::Node *node);
    virtual void doOverNode(ivf::Node *node);
    virtual void doLeaveNode(ivf::Node *node);
    virtual void doUpdateUi();
    virtual int doSetup() override;
    virtual void doDraw() override;
    virtual void doUpdateOtherUi() override;
    virtual void doDrawUi() override;
    virtual void doDrawComplete() override;
};

typedef std::shared_ptr<GLFWSceneWindow> GLFWSceneWindowPtr;

}; // namespace ivfui
