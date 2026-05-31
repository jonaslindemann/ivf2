# Time Control Panel

`TimeControlPanel` is a built-in ImGui window that exposes the global `TimeController` to the user without any custom UI code.

## Adding the Panel

```cpp
#include <ivfui/time_control_panel.h>

int onSetup() override
{
    addUiWindow(ivfui::TimeControlPanel::create());
    return 0;
}
```

One line. The panel appears automatically and manages itself.

## What the Panel Shows

| Control | Function |
|---------|----------|
| Pause / Resume button | Toggles `TimeController::isPaused()` |
| Time scale slider (0–4×) | Calls `setScale()` in 0.1 increments |
| Reset button | Calls `reset()` — elapsed returns to 0 |
| Elapsed readout | Live display of `elapsed()` in seconds |
| Delta readout | Live display of `delta()` (0 when paused) |
| Status label | Shows **PAUSED** or the current scale value |

## Combining with Keyboard Controls

The panel and keyboard controls operate on the same `TimeController` singleton, so they stay in sync automatically:

```cpp
void onKey(int key, int, int action, int) override
{
    if (action != GLFW_PRESS) return;
    auto *tc = ivf::TimeController::instance();
    if (key == GLFW_KEY_SPACE) tc->togglePause();
    if (key == GLFW_KEY_R)     tc->reset();
}
```

Moving the panel slider and pressing Space both affect the same state.

## Extending with Custom Logic

For more control, read `TimeController` state directly and build your own ImGui layout alongside the panel:

```cpp
void onDrawUi() override
{
    auto *tc = ivf::TimeController::instance();
    ImGui::Text("Elapsed: %.2f s", tc->elapsed());
    if (ImGui::Button("Slow motion"))
        tc->setScale(0.2f);
    ImGui::SameLine();
    if (ImGui::Button("Normal"))
        tc->setScale(1.0f);
}
```

See [Time Control](../core/time_controller.md) for the full `TimeController` API.
