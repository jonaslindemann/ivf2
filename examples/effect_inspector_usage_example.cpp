// Example usage of EffectInspector in a GLFWSceneWindow-based application

#include <ivfui/glfw_scene_window.h>
#include <ivfui/effect_inspector.h>
#include <ivf/effect.h>

class ExampleWindow : public ivfui::GLFWSceneWindow {
private:
    ivfui::EffectInspectorPtr m_effectInspector;

public:
    ExampleWindow(int width, int height, std::string title) 
        : GLFWSceneWindow(width, height, title) {}

    virtual int onSetup() override {
        // The effect inspector is automatically created and integrated in doSetup()
        // You can access it and customize it here if needed
        
        // Example: Show the effect inspector window by default
        this->showEffectInspector();
        
        return 0;
    }

    virtual void onKey(int key, int scancode, int action, int mods) override {
        if (key == GLFW_KEY_E && action == GLFW_PRESS) {
            // Toggle effect inspector with 'E' key
            this->showEffectInspector();
        }
        
        GLFWSceneWindow::onKey(key, scancode, action, mods);
    }
};

// The EffectInspector is now available through:
// - View menu -> "Effect Inspector" (with 'e' shortcut)
// - Windows menu -> "Effect Inspector" (toggle visibility)
// - Programmatically via showEffectInspector() method