# Time-Driven Animation

`TimeController` makes time a first-class creative parameter. Instead of animations playing at fixed wall-clock speed, you can pause mid-motion, scrub forward, and run in slow-motion — all without changing any animation code.

## The Core Pattern

Replace `elapsedTime()` with `TimeController::elapsed()` and `frameTime()` with `TimeController::delta()`:

```cpp
void onUpdate() override
{
    double t  = ivf::TimeController::instance()->elapsed();
    double dt = ivf::TimeController::instance()->delta();

    // Everything that depends on t/dt now respects pause and scale
    m_deformer->setTime(static_cast<float>(t));
    m_particles->update(static_cast<float>(dt));
    m_cube->setRotAngle(static_cast<float>(t) * 45.0f);
}
```

That's the whole change. Pause, slow-motion, and reset become available to every system that uses this pattern.

## Combining with Deformers

Wave and turbulence deformers are driven by a time parameter. Wire them to `TimeController`:

```cpp
// Setup
m_wave1 = ivf::WaveDeformer::create(0.3f, 0.6f, 1.1f,
    glm::normalize(glm::vec3(1.0f, 0.0f, 0.3f)),
    glm::vec3(0.0f, 1.0f, 0.0f));
m_ocean->addDeformer(m_wave1);

// Per frame — pause freezes the surface, slow-motion makes waves glide
void onUpdate() override
{
    double t = ivf::TimeController::instance()->elapsed();
    m_wave1->setTime(static_cast<float>(t));
    m_ocean->applyDeformers();
}
```

## Slow-Motion and Speed-Up

Time scale is a creative tool, not just a debug feature. A slow-motion moment draws attention; a speed-up can compress routine transitions.

```cpp
// Dramatic slow-motion on impact
ivf::TimeController::instance()->setScale(0.15f);

// Resume normal speed
ivf::TimeController::instance()->setScale(1.0f);

// Hyperspeed
ivf::TimeController::instance()->setScale(3.0f);
```

### Keyboard Scrubbing

```cpp
void onKey(int key, int, int action, int) override
{
    if (action != GLFW_PRESS) return;
    auto *tc = ivf::TimeController::instance();

    if (key == GLFW_KEY_SPACE)
        tc->togglePause();
    else if (key == GLFW_KEY_EQUAL)
        tc->setScale(std::min(4.0f, tc->scale() + 0.1f));
    else if (key == GLFW_KEY_MINUS)
        tc->setScale(std::max(0.0f, tc->scale() - 0.1f));
    else if (key == GLFW_KEY_R)
        tc->reset();
}
```

## Orbiting Objects

Procedural orbital motion driven by scaled time:

```cpp
void onUpdate() override
{
    double t = ivf::TimeController::instance()->elapsed();

    for (int i = 0; i < k_count; ++i) {
        float angle  = static_cast<float>(t) * 0.8f
                     + i * (2.0f * std::numbers::pi_v<float> / k_count);
        float radius = 5.5f;
        float y      = 1.5f + 0.6f * std::sin(static_cast<float>(t) * 1.3f + i);
        m_cubes[i]->setPos({radius * std::cos(angle), y, radius * std::sin(angle)});
        m_cubes[i]->setRotAngle(static_cast<float>(t) * 60.0f + i * 40.0f);
    }
}
```

Pausing freezes every orbit simultaneously. Scaling makes them all slow or fast together.

## Adding a Control Panel

Drop in the built-in UI for zero-effort time control:

```cpp
#include <ivfui/time_control_panel.h>

int onSetup() override
{
    addUiWindow(ivfui::TimeControlPanel::create());
    return 0;
}
```

The panel exposes pause/resume, a time-scale slider, a reset button, and a live readout. See [Time Control Panel](../ui/time_control_panel.md).

## Resetting for Loops

`reset()` sets elapsed back to zero without changing scale or pause state. Use it to restart a generative scene or synchronise looping animations:

```cpp
// After a performance loop ends, restart cleanly
ivf::TimeController::instance()->reset();
```

## Checklist for Time-Aware Code

- Use `TimeController::elapsed()` instead of `elapsedTime()`
- Use `TimeController::delta()` instead of `frameTime()`
- Call `setTime(t)` on deformers each frame
- Pass `delta()` to `ParticleSystem::update()`
- Drive `CameraAnimation::update()` with `delta()` so fly-throughs respect scale

See the full runnable example at [Time Control Examples](../examples/time_control_examples.md).
