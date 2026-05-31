# Flow Field Examples

## flow_field1 — Curl-Noise Particle Steering

This example shows `FlowField` steering 8 000 particles through organic, fluid-like trajectories using curl noise. The field evolves slowly over time and the parameters can be tweaked live via ImGui sliders. Pausing via `TimeController` freezes both the field and the particles in place.

**Scene setup:**

- `ParticleSystem` with 8 000 particles emitted from a flat box (−8 to +8 on XZ)
- Particles fade from blue to purple as they age
- `FlowField` with offset `(31.4, 17.3, 57.1)` to reach a well-varied noise region
- `TimeControlPanel` for pause/resume/scale

**Keyboard controls:**

| Key | Action |
|-----|--------|
| `Space` | Pause / resume (via `TimeController::togglePause()`) |
| `ESC` | Quit |

**Live-tweak sliders:**

| Slider | Range | Effect |
|--------|-------|--------|
| Scale | 0.05–2.0 | Broader vs. tighter swirl features |
| Strength | 0.1–10.0 | Peak particle speed |
| Octaves | 1–4 | Coarse vs. fine vortex detail |
| Time speed | 0.0–0.5 | Field evolution rate (0 = static pattern) |

The `onUpdate()` pattern is minimal: set the field's time from `TimeController::elapsed()`, then call `ps->update(dt)`. The steering function installed by `applyToParticleSystem()` runs inside `update()` automatically.

### flow_field1.cpp

```cpp
--8<-- "examples/flow_field1/flow_field1.cpp"
```
