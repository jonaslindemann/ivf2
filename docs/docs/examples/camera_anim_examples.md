# Camera Animation Examples

## camera_anim1 — Spline Fly-Through with Shake

This example demonstrates `CameraAnimation`: a smooth camera path that orbits and flies over a colourful grid of procedurally-sized boxes. The path is defined by eight eye-position keyframes, the camera looks forward along the spline tangent, and a shake effect can be triggered on demand.

**Scene setup:**

- 11×11 grid of `Box` primitives with height and hue derived from sine/cosine patterns
- Single `PointLight` overhead for shading
- `CameraAnimation` in `Loop` mode with `autoLookForward` enabled
- Manipulator input blocked during playback

**Keyboard controls:**

| Key | Action |
|-----|--------|
| `P` | Play / stop the animation |
| `S` | Trigger camera shake (0.4 units, 0.5 s) |
| `L` | Toggle Loop vs. Once mode |
| `F` | Toggle auto-look-forward vs. explicit target spline |
| `ESC` | Quit |

**What to notice:**

- The `update()` call in `onUpdate()` passes `TimeController::instance()->delta()` — the fly-through automatically pauses if the TimeController is paused.
- `setBlockManipulatorDuringPlayback(true)` prevents orbit/pan input from fighting the animation.
- Duplicating the first keyframe at position 8 closes the loop without a visible jump.
- `shake()` works even while playing — it adds a decaying random offset on top of the spline position.

### camera_anim1.cpp

```cpp
--8<-- "examples/camera_anim1/camera_anim1.cpp"
```
