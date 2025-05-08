# Animation Examples

This section explores the animation examples provided with ivf2.

## animation1 Example

The `animation1` example demonstrates simple animations using direct manipulation within the update loop.

### Key Concepts Demonstrated

- Basic rotation animation
- Time-based animation
- Multiple animated objects

### Code Walkthrough

```cpp
virtual void onUpdate()
{
    m_cube->setRotAxis(glm::vec3(1.0, 1.0, 1.0));
    m_cube->setRotAngle(20.0 * elapsedTime());

    m_sphereXfm->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
    m_sphereXfm->setRotAngle(20.0 * elapsedTime());
}
```

The above code rotates a cube around a diagonal axis and a sphere around the Y axis. The rotation speed is controlled by the elapsed time.

## animation2 Example

The `animation2` example shows a more complex Lissajous animation with an interactive UI.

### Key Concepts Demonstrated

- Lissajous curve animation
- LineTrace for path visualization
- Interactive parameter control with ImGui
- Dynamic parameter updates

### Lissajous Class

```cpp
class Lissajous {
private:
    float m_a, m_b, m_c, m_d, m_e, m_f, m_g, m_h, m_i;

public:
    // ... constructor and methods ...

    glm::vec3 operator()(float t) {
        return glm::vec3(m_a * sin(m_b * t + m_d), m_c * sin(m_e * t + m_f), m_g * sin(m_h * t + m_i));
    }
};
```

### Update Loop

```cpp
virtual void onUpdate() override
{
    // Check if UI parameters were changed
    if (m_lissajouWindow->is_dirty())
    {
        float a, b, c, d, e, f, g, h, i;
        m_lissajouWindow->get_params(a, b, c, d, e, f, g, h, i);
        m_lissajous.setParameters(a, b, c, d, e, f, g, h, i);
        m_speed = m_lissajouWindow->speed();
        m_trace->setSize(m_lissajouWindow->size());
        m_trace->reset();
    }

    // Update sphere position
    auto pos = m_lissajous(elapsedTime() * m_speed);
    m_sphere->setPos(pos);
    m_trace->add(pos);
    m_trace->refresh();
}
```