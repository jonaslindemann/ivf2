#pragma once

#include <memory>
#include <string>

#include <imgui.h>

namespace ivfui {

class UiWindow {
private:
    std::string m_name;
    bool m_visible;

public:
    UiWindow(const std::string name);
    virtual ~UiWindow();

    static std::shared_ptr<UiWindow> create(const std::string name);

    void draw();

    void setVisible(bool flag);
    bool visible();
    void show();
    void hide();
    void update();

protected:
    virtual void doDraw();
    virtual void doUpdate();
};

typedef std::shared_ptr<UiWindow> UiWindowPtr;

} // namespace ivfui
