# Ready-made Widgets

ivf2 includes UI windows for common tasks, so you don't have to build them yourself. Create one, add it with `addUiWindow()`, and it appears in the scene window.

## Overview

| Widget | Purpose |
|--------|---------|
| `FpsWindow` | Shows frames per second |
| `PointLightWindow`, `DirectionalLightWindow`, `SpotLightWindow` | Edit a light's colours, position, direction and attenuation |
| `ObjectInspector` | Edit the properties of any inspectable object |
| `SceneInspector` | Tree view of the scene with a property editor |
| `EffectInspector` | Manage the post-processing chain |
| `CameraWindow` | Camera position, target and projection |
| `SceneControlPanel` | Axis and grid settings |
| `TimeControlPanel` | Pause, slow motion and time scale |
| `UiInputDialog` | Modal dialog asking for one number or string |

The inspectors, camera window and control panel are created automatically by every scene window; see [Built-in Scene Tools](scene_ui_integration.md).

## FpsWindow

A small window showing the current frame rate. Useful while optimising a scene.

```cpp
addUiWindow(ivfui::FpsWindow::create());
```

## Light windows

One window per light, for adjusting lighting interactively. After a change, the lights must be re-applied, so check `isDirty()` in `onUpdateUi()`:

```cpp
int onSetup() override
{
    auto lm = renderContext()->lights();
    auto sun = lm->addDirectionalLight();
    // ...
    m_sunWindow = ivfui::DirectionalLightWindow::create(sun, "Sun");
    addUiWindow(m_sunWindow);
    return 0;
}

void onUpdateUi() override
{
    if (m_sunWindow->isDirty())
        renderContext()->lights()->apply();
}
```

`PointLightWindow` and `SpotLightWindow` work the same way. See the `lighting1` example.

## ObjectInspector

Shows every property of a `PropertyInspectable` object (nodes, materials, effects, deformers, or your own classes) as editable fields, grouped by category.

```cpp
auto inspector = ivfui::ObjectInspector::create("Bloom");
inspector->setObject(m_bloom, "Bloom effect");
inspector->setVisible(true);
addUiWindow(inspector);
```

| Method | Description |
|--------|-------------|
| `setObject(obj, name)` | Object to inspect; pass a new one to switch |
| `setPropertyChangedCallback(fn)` | Called with the property name after each edit |
| `setCategoryExpanded(category, open)` | Open or collapse a category |

## TimeControlPanel

Play, pause, slow motion and time scaling for everything driven by `TimeController`. See [Time Control Panel](time_control_panel.md).

## UiInputDialog

A small modal dialog that asks for a single value. It calls you back when the user confirms or cancels:

```cpp
m_dialog = ivfui::UiInputDialog::create("Particle count", "Count:");

// to open it, e.g. from a menu item:
m_dialog->openInt(m_count, [this](bool accepted, const ivfui::InputValue &value) {
    if (accepted)
        m_count = *std::get<int *>(value);
});

// every frame, in onUpdateUi():
m_dialog->draw();
```

Use `openFloat()`, `openInt()`, `openDouble()` or `openString()` depending on the type. The numeric versions accept optional minimum, maximum and step values. Unlike the other widgets, the dialog is not added with `addUiWindow()`; call its `draw()` yourself each frame.

## Related

- [UI Windows](ui_windows.md)
- [Custom UI](custom_ui.md)
- [UI Examples](../examples/ui_examples.md)
