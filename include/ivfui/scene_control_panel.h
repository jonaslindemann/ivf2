#pragma once

#include <memory>

#include <ivfui/ui_window.h>

namespace ivfui {

class GLFWSceneWindow;

class SceneControlPanel : public ivfui::UiWindow {
private:
    bool m_isDirty{false};
    ivfui::GLFWSceneWindow *m_sceneWindow{nullptr};

    bool m_showAxis{false};
    bool m_showGrid{false};

    float m_axisLength{1.0f};
    int m_tickX{11};
    int m_tickY{11};
    int m_tickZ{11};
    float m_tickSpacingX{1.0e-0};
    float m_tickSpacingY{1.0e-0};
    float m_tickSpacingZ{1.0e-0};

public:
    SceneControlPanel(std::string caption, GLFWSceneWindow *sceneWindow);

    static std::shared_ptr<SceneControlPanel> create(std::string caption, GLFWSceneWindow *sceneWindow);

    bool isDirty();

protected:
    virtual void doDraw() override;
    virtual void doUpdate() override;
};

typedef std::shared_ptr<SceneControlPanel> SceneControlPanelPtr;

} // namespace ivfui
