# Camera Animation

`CameraAnimation` animates the camera along a smooth spline path defined by a series of eye-position and look-at-target keyframes. It supports looping, ping-pong, auto-look-forward, and a decaying shake effect for cinematic impact moments.

## Quick Start

```cpp
#include <ivfui/camera_animation.h>

// In onSetup():
auto anim = ivfui::CameraAnimation::create(cameraManipulator());

anim->addKeyframe({0.0f,  5.0f, 15.0f}, {0.0f, 0.0f, 0.0f});
anim->addKeyframe({10.0f, 8.0f,  5.0f}, {0.0f, 1.0f, 0.0f});
anim->addKeyframe({0.0f,  6.0f,-12.0f}, {0.0f, 0.0f, 0.0f});

anim->setDuration(8.0f);
anim->setMode(ivf::SplineAnimMode::Loop);
anim->setAutoLookForward(true);
anim->play();

// In onUpdate():
m_anim->update(ivf::TimeController::instance()->delta());
```

`CameraAnimation` uses the `TimeController` delta so playback respects global pause and time scaling.

## Keyframes

Each keyframe is a pair: the camera eye position and a look-at target.

```cpp
anim->addKeyframe(glm::vec3 pos, glm::vec3 target);
anim->clearKeyframes();  // remove all and reset
```

The positions follow a CatmullRom spline for smooth, constant-speed travel. Targets follow their own spline unless `autoLookForward` is enabled.

!!! tip
    Repeat the first keyframe at the end to close a loop without a visible seam.

## Playback Modes

| Mode | Behaviour |
|------|-----------|
| `SplineAnimMode::Once` | Play once and stop at the end |
| `SplineAnimMode::Loop` | Jump back to start when the end is reached |
| `SplineAnimMode::PingPong` | Reverse direction at each end |

```cpp
anim->setMode(ivf::SplineAnimMode::Loop);
```

## Auto-Look-Forward

When enabled, the camera automatically looks along the tangent of the position spline. The target keyframes are ignored. This gives a natural flying-through-the-scene feel.

```cpp
anim->setAutoLookForward(true);   // look where you're going
anim->setAutoLookForward(false);  // use explicit target spline
```

## Playback Control

```cpp
anim->play();             // start or resume
anim->stop();             // stop (keeps current position)
anim->rewind();           // reset elapsed to start

bool playing = anim->isPlaying();
float p      = anim->progress();  // 0.0 = start, 1.0 = end
```

Block manipulator input (orbit, pan) while the animation plays to avoid conflicts:

```cpp
anim->setBlockManipulatorDuringPlayback(true);
```

## Camera Shake

`shake()` applies a brief, exponentially-decaying random jitter to the camera position — useful for explosions, impacts, or dramatic moments.

```cpp
anim->shake(0.4f, 0.5f);  // 0.4 units intensity, 0.5 seconds duration
```

Shake works independently of playback: you can shake a stopped camera.

## Keyboard Toggle Example

```cpp
void onKey(int key, int, int action, int) override
{
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_P) {
        if (m_anim->isPlaying()) m_anim->stop();
        else                     m_anim->play();
    } else if (key == GLFW_KEY_S) {
        m_anim->shake(0.4f, 0.5f);
    } else if (key == GLFW_KEY_L) {
        m_looping = !m_looping;
        m_anim->setMode(m_looping ? ivf::SplineAnimMode::Loop : ivf::SplineAnimMode::Once);
    }
}
```

## Orbit Fly-Through Recipe

A wide orbit around a scene centre with altitude variation. Eight keyframes close the loop seamlessly.

```cpp
auto anim = ivfui::CameraAnimation::create(cameraManipulator());
anim->setMode(ivf::SplineAnimMode::Loop);
anim->setDuration(12.0f);
anim->setAutoLookForward(true);
anim->setBlockManipulatorDuringPlayback(true);

// Eye positions — orbit at varying radius and height
anim->addKeyframe({ 18.0f,  8.0f,  0.0f}, {0, 1, 0});
anim->addKeyframe({  8.0f, 12.0f, 14.0f}, {0, 1, 0});
anim->addKeyframe({ -5.0f,  5.0f, 18.0f}, {0, 1, 0});
anim->addKeyframe({-18.0f,  9.0f,  3.0f}, {0, 1, 0});
anim->addKeyframe({-10.0f,  6.0f,-14.0f}, {0, 1, 0});
anim->addKeyframe({  3.0f, 14.0f,-16.0f}, {0, 1, 0});
anim->addKeyframe({ 14.0f,  7.0f, -8.0f}, {0, 1, 0});
anim->addKeyframe({ 18.0f,  8.0f,  0.0f}, {0, 1, 0});  // close the loop
anim->play();
```

See the full runnable example at [Camera Animation Examples](../examples/camera_anim_examples.md).
