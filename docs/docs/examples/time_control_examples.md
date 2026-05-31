# Time Control Examples

## time_control1 — Global Pause, Slow-Motion, and Time Scaling

This example demonstrates `TimeController` as a single control point for all animation in a scene. A wave-deformed ocean plane and nine orbiting coloured cubes all derive their motion from `TimeController::elapsed()`. Pausing, slowing, or resetting time affects every element simultaneously.

**Scene setup:**

- `DeformablePrimitive<Plane>` with two `WaveDeformer` instances (different directions and speeds)
- 9 `Box` objects orbiting at evenly-spaced angular offsets with vertical oscillation
- `TimeControlPanel` for interactive control

**Keyboard controls:**

| Key | Action |
|-----|--------|
| `Space` | Pause / resume |
| `+` / `-` | Increase / decrease time scale (0.1 steps, clamped 0–4×) |
| `R` | Reset elapsed time to 0 |
| `ESC` | Quit |

The key pattern to observe: every animation reads `TimeController::instance()->elapsed()` rather than `elapsedTime()`. No special pause logic is needed anywhere else — the single call to `togglePause()` freezes everything.

### time_control1.cpp

```cpp
--8<-- "examples/time_control1/time_control1.cpp"
```
