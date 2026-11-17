# Custom UI

ivf2's user interface is built on [Dear ImGui](https://github.com/ocornut/imgui), an *immediate-mode* UI library. Instead of creating buttons and sliders once and wiring up events, you describe the whole UI every frame in plain code. A slider is a function call that draws itself and returns `true` when its value changed. This makes small tools very quick to write.

[UI Windows](ui_windows.md) covers the basics of `UiWindow`. This page goes further.

## Two ways to add UI

**Inline in the scene window.** For a quick panel, draw directly in `onUpdateUi()`:

```cpp
void onUpdateUi() override
{
    ImGui::Begin("Controls");
    ImGui::SliderFloat("Speed", &m_speed, 0.0f, 5.0f);
    if (ImGui::Button("Reset"))
        reset();
    ImGui::End();
}
```

**As a `UiWindow` class.** For anything larger or reusable, subclass `UiWindow` and add it with `addUiWindow()`. The window handles `Begin()`/`End()`, visibility, size and position for you, and you can show and hide it from code.

## The UiWindow lifecycle

`UiWindow` has several methods you can override:

| Method | Called | Use it for |
|--------|--------|------------|
| `doDraw()` | Every frame, inside the window | The window's contents |
| `doPreDraw()` | Every frame, before the window begins | `ImGui::SetNextWindow...` calls, style pushes |
| `doPostDraw()` | Every frame, after the window ends | Popping styles pushed in `doPreDraw()` |
| `doUpdate()` | When `update()` is called | Refreshing data from the scene |
| `doWindowFlags()` | Every frame | Returning `ImGuiWindowFlags`, e.g. `ImGuiWindowFlags_AlwaysAutoResize` |

```cpp
class StatsWindow : public ivfui::UiWindow {
public:
    StatsWindow() : UiWindow("Stats") {}
    static std::shared_ptr<StatsWindow> create() { return std::make_shared<StatsWindow>(); }

protected:
    ImGuiWindowFlags doWindowFlags() const override
    {
        return ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
    }

    void doPreDraw() override
    {
        UiWindow::doPreDraw();               // keeps setSize() and setPosition() working
        ImGui::SetNextWindowBgAlpha(0.6f);   // semi-transparent background
    }

    void doDraw() override
    {
        ImGui::Text("Particles: %d", m_count);
    }
};
```

`setSize()` and `setPosition()` set the window's initial size and position; the user can still move and resize it afterwards. `show()`, `hide()` and `toggleVisibility()` control whether it is drawn, for example from a key:

```cpp
void onKey(int key, int scancode, int action, int mods) override
{
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        m_help->toggleVisibility();
}
```

## Talking to the scene

A UI window shouldn't reach into the scene directly. Two patterns keep things tidy:

- **Dirty flag.** The window stores values and sets a flag when they change; the scene window checks it in `onUpdate()`. The Lissajous example in [Lissajous Curves](../animation/lissajous.md) does this.
- **Callbacks.** Pass a `std::function` to the window, and call it when something changes.

```cpp
class ColorWindow : public ivfui::UiWindow {
    glm::vec3 m_color{1.0f};
    std::function<void(glm::vec3)> m_onChange;

public:
    ColorWindow(std::function<void(glm::vec3)> onChange)
        : UiWindow("Colour"), m_onChange(std::move(onChange)) {}

    void doDraw() override
    {
        if (ImGui::ColorEdit3("Colour", &m_color.x) && m_onChange)
            m_onChange(m_color);
    }
};

// in onSetup():
addUiWindow(std::make_shared<ColorWindow>([this](glm::vec3 c) {
    m_material->setDiffuseColor(glm::vec4(c, 1.0f));
}));
```

## Making your classes inspectable

Instead of writing a window, make your class `PropertyInspectable` and let an `ObjectInspector` build the UI for you. Register each field in `setupProperties()`:

```cpp
class Swarm : public ivf::PropertyInspectable {
    float m_speed{1.0f};
    int m_count{500};
    glm::vec3 m_color{1.0f, 0.6f, 0.2f};
    bool m_trails{true};

protected:
    void setupProperties() override
    {
        addProperty("Speed", &m_speed, 0.0, 10.0, "Motion");   // with a range: shown as a slider
        addProperty("Count", &m_count, 1, 5000, "Motion");
        addProperty("Colour", &m_color, "Look");
        addProperty("Trails", &m_trails, "Look");
    }
};
```

```cpp
auto inspector = ivfui::ObjectInspector::create("Swarm");
inspector->setObject(m_swarm);
addUiWindow(inspector);
```

Supported types include `float`, `double`, `int`, `bool`, `std::string`, `glm::vec3`, `glm::vec4`, `glm::uint`, `glm::uvec3` and `glm::uvec4`. The last argument is the category, which groups properties under a heading.

## Tips

- **Keep `doDraw()` light.** It runs every frame; do expensive work only when a value has changed.
- **Give widgets unique labels.** ImGui identifies widgets by their label. Use `"Speed##wave1"` and `"Speed##wave2"` to show two sliders both labelled "Speed".
- **Explore what's possible** with `ImGui::ShowDemoWindow()`, which shows every widget with its source; the `scene_ui1` example has a toggle for it.

## Related

- [UI Windows](ui_windows.md)
- [Built-in Scene Tools](scene_ui_integration.md)
- [Ready-made Widgets](widgets.md)
