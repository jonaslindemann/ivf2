#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <ivfui/ui.h>
#include <ivf/nodes.h>

namespace ivfui {

class GLFWSceneWindow : public GLFWWindow {
private:
    ivf::CompositeNodePtr m_scene;
    ivfui::CameraManipulatorPtr m_camManip;
    std::vector<ivfui::UiWindowPtr> m_uiWindows;

public:
    GLFWSceneWindow(int width, int height, const std::string title, GLFWmonitor *monitor = nullptr,
                    GLFWwindow *shared = nullptr);
    virtual ~GLFWSceneWindow();

    static std::shared_ptr<GLFWSceneWindow> create(int width, int height, const std::string title,
                                                   GLFWmonitor *monitor = nullptr, GLFWwindow *shared = nullptr);

    void add(ivf::NodePtr node);
    void remove(ivf::NodePtr node);
    void clear();

    void addUiWindow(ivfui::UiWindowPtr uiWindow);

    ivf::CompositeNodePtr scene();

    ivfui::CameraManipulatorPtr cameraManipulator();

    virtual void onResize(int width, int height) override;
    virtual void onUpdateOtherUi() override;
    virtual void onDraw() override;
    virtual void onDrawUi() override;

    virtual void onUpdateUi();
};

typedef std::shared_ptr<GLFWSceneWindow> GLFWSceneWindowPtr;

}; // namespace ivfui
