# UI Windows

ivf2 provides a user interface system built on Dear ImGui.

## Overview

The UI window system in ivf2 allows you to create customizable user interface elements for controlling your application. UI windows are implemented using the Dear ImGui library, providing a lightweight and immediate-mode interface.

## Creating a UI Window

To create a UI window, subclass the `ivfui::UiWindow` class:

```cpp
class MyControlWindow : public ivfui::UiWindow {
private:
    float m_value;
    bool m_toggle;
    
public:
    MyControlWindow() : UiWindow("My Controls"), m_value(0.0f), m_toggle(false) {}
    
    static std::shared_ptr<MyControlWindow> create() {
        return std::make_shared<MyControlWindow>();
    }
    
    void doDraw() override {
        ImGui::Text("Control Panel");
        ImGui::SliderFloat("Value", &m_value, 0.0f, 1.0f);
        ImGui::Checkbox("Toggle", &m_toggle);
        
        if (ImGui::Button("Reset")) {
            m_value = 0.0f;
            m_toggle = false;
        }
    }
    
    float getValue() const { return m_value; }
    bool getToggle() const { return m_toggle; }
};
```

## Adding UI Windows to a Scene

To add a UI window to your scene:

```cpp
class MyApplication : public ivfui::GLFWSceneWindow {
private:
    MyControlWindowPtr m_controlWindow;
    
public:
    int onSetup() override {
        // Create UI window
        m_controlWindow = MyControlWindow::create();
        
        // Add UI window to the scene
        this->addUiWindow(m_controlWindow);
        
        return 0;
    }
    
    void onUpdateUi() override {
        // Use the UI values in your scene
        if (m_controlWindow->getToggle()) {
            // Do something based on toggle state
        }
        
        float value = m_controlWindow->getValue();
        // Use the value
    }
};
```

## UI Window Features

- **Show/Hide**: Control window visibility with `show()` and `hide()` methods
- **Window title**: Set in the constructor
- **State access**: Create accessors to get/set internal state
- **ImGui widgets**: Use any ImGui widget in your `doDraw()` method