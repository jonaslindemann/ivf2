# Time Control

ivf2 provides a global time controller that lets you pause, resume, and scale time across your entire scene — animations, deformers, particle systems, and any custom logic that depends on elapsed time.

## Why TimeController?

`GLFWWindow` exposes `elapsedTime()` and `frameTime()` tied to the wall clock. They cannot be paused or slowed. `TimeController` adds a logical time layer on top: the same real delta is fed in each frame, but your code reads a scaled, pausable version. All systems that consume `TimeController::elapsed()` or `delta()` automatically respect the current state.

## Basic Usage

Include the header (already part of `<ivf/gl.h>`) and read from the singleton in `onUpdate()`:

```cpp
#include <ivf/time_controller.h>

void onUpdate() override
{
    auto *tc = ivf::TimeController::instance();
    double t  = tc->elapsed();   // accumulated scaled time (seconds)
    double dt = tc->delta();     // scaled frame delta (0 when paused)

    m_wave->setTime(static_cast<float>(t));
    m_particles->update(static_cast<float>(dt));
}
```

`GLFWSceneWindow` feeds the real frame delta into `TimeController` automatically — you never call `update()` yourself.

## Pause and Resume

```cpp
void onKey(int key, int, int action, int) override
{
    if (action != GLFW_PRESS) return;
    auto *tc = ivf::TimeController::instance();

    if (key == GLFW_KEY_SPACE)
        tc->togglePause();          // flip pause state
    else if (key == GLFW_KEY_P)
        tc->pause();
    else if (key == GLFW_KEY_R)
        tc->resume();
}
```

When paused, `delta()` returns `0.0` and `elapsed()` stops accumulating. Systems driven by `delta()` freeze in place automatically.

## Time Scaling

Scale adjusts how fast logical time runs relative to wall time. A scale of `0.5` gives half-speed slow motion; `2.0` doubles the animation speed.

```cpp
auto *tc = ivf::TimeController::instance();

tc->setScale(0.5f);  // half speed — slow motion
tc->setScale(2.0f);  // double speed
tc->setScale(0.0f);  // freeze (equivalent to pause but scale stays 0)
tc->setScale(1.0f);  // back to normal
```

Scale and pause are independent. A paused controller at scale `2.0` resumes at double speed.

```cpp
// Increase/decrease with keyboard
if (key == GLFW_KEY_EQUAL)
    tc->setScale(std::min(4.0f, tc->scale() + 0.1f));
else if (key == GLFW_KEY_MINUS)
    tc->setScale(std::max(0.0f, tc->scale() - 0.1f));
```

## Reset

```cpp
tc->reset();   // sets elapsed back to 0; scale and pause state unchanged
```

Useful for restarting a scene or resynchronising looping animations.

## API Reference

| Method | Description |
|--------|-------------|
| `instance()` | Get the singleton pointer |
| `elapsed()` | Accumulated scaled time in seconds |
| `delta()` | Scaled frame delta (0 when paused) |
| `setScale(float)` | Set time multiplier (0 = frozen, 1 = normal) |
| `scale()` | Current time scale |
| `pause()` / `resume()` / `togglePause()` | Pause control |
| `isPaused()` | Query pause state |
| `reset()` | Reset elapsed to 0 |

## TimeControlPanel

The easiest way to expose time control to users is the built-in ImGui panel:

```cpp
#include <ivfui/time_control_panel.h>

int onSetup() override
{
    addUiWindow(ivfui::TimeControlPanel::create());
    return 0;
}
```

The panel provides a pause/resume button, a time-scale slider (0–4×), a reset button, and a live readout of elapsed time. See [Time Control Panel](../ui/time_control_panel.md) for details.
