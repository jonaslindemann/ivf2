# Basic Animation

This guide introduces the animation system in ivf2.

## Overview

ivf2 provides several ways to animate objects in your scene. The simplest form is direct manipulation of transform properties (position, rotation, scale) over time.

## Animation Basics

### Using the Update Loop

The simplest way to animate objects is to modify properties in the `onUpdate()` method:

```cpp
class MyWindow : public ivfui::GLFWSceneWindow {
private:
    ivf::CubePtr m_cube;
    
public:
    // ... constructor and other methods ...
    
    virtual void onUpdate() override {
        m_cube->setRotAxis(glm::vec3(1.0, 1.0, 1.0));
        m_cube->setRotAngle(20.0 * elapsedTime());
    }
};
```

### Time-Based Animation

Always use time-based animation instead of frame-based animation to ensure consistent speed across different systems:

```cpp
// Bad - frame-based animation
m_angle += 1.0;
object->setRotAngle(m_angle);

// Good - time-based animation
object->setRotAngle(20.0 * elapsedTime());
```

## Using TimeController

For interactive applications — pause buttons, slow-motion, time scrubbing — replace raw `elapsedTime()` with `TimeController`:

```cpp
#include <ivf/time_controller.h>

void onUpdate() override {
    double t  = ivf::TimeController::instance()->elapsed();
    double dt = ivf::TimeController::instance()->delta();

    m_wave->setTime(static_cast<float>(t));
    m_particles->update(static_cast<float>(dt));
}

void onKey(int key, int, int action, int) override {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        ivf::TimeController::instance()->togglePause();
}
```

`GLFWSceneWindow` feeds the real frame delta into `TimeController` automatically each frame. `elapsed()` and `delta()` respect the current pause state and time scale. See [Time Control](../core/time_controller.md) for the full API.

## Animation Types

ivf2 offers several built-in animation types:

1. **Direct manipulation** - Updating properties each frame via `onUpdate()`
2. **SplineAnimation** - Animation along a spline path
3. **KeyframeAnimation** - Animation between key poses
4. **CameraAnimation** - Spline fly-through with optional shake (see [Camera Animation](camera_animation.md))
5. **Procedural animation** - Using mathematical functions driven by `TimeController::elapsed()`