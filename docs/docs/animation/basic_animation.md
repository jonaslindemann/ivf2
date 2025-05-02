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

## Animation Types

ivf2 offers several built-in animation types:

1. **Direct manipulation** - Updating properties each frame
2. **SplineAnimation** - Animation along a spline path
3. **KeyframeAnimation** - Animation between key poses
4. **Procedural animation** - Using mathematical functions