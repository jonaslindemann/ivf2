# Music-Driven Scenes

The **ivfmusic** library turns a song into the clock of a scene. Everything that moves reads the *song position* rather than wall-clock time, so the visuals stay in sync while you play, pause, scrub and seek, and a frame export renders exactly what you saw.

It provides:

- **`GLFWMusicSceneWindow`**: a scene window driven by the song, with a transport bar, markers, live-reloaded automation and a presentation mode
- **`SongTransport`**: song playback and musical time (beats, bars, beat phase)
- **`Automation`**: parameters keyframed in bars, loaded from a JSON file that reloads while the scene runs
- **`Score`**: the note events of the composition (Renoise `.xrns`), turned into control values with `TrackEnvelope`
- **`LiveControls`**: MIDI faders, knobs and pads mapped to parameters, triggers and actions by a JSON file, for playing a scene live; **`AutomationRecorder`** records a performance as a take that plays back and exports frame-exact

Building blocks that are useful in any scene live in the core libraries: `MipBloomEffect`, `BlendShaderMaterial` and the ready-made `GlowMaterial`, `MistMaterial` and `GradientSkyMaterial` in **ivf**, and `PathCameraManipulator` in **ivfui**. See [Glow, blending and guided cameras](#glow-blending-and-guided-cameras).

## A Minimal Scene

Derive from `GLFWMusicSceneWindow`, build the scene in `onSceneSetup()` and animate it in `onSceneUpdate()`. `ivfmusic::run()` parses the command line and runs the window.

```cpp
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfmusic/ivfmusic.h>

class MyScene : public ivfmusic::GLFWMusicSceneWindow {
    ivf::SpherePtr m_sphere;

public:
    using GLFWMusicSceneWindow::GLFWMusicSceneWindow;

protected:
    int onSceneSetup() override
    {
        m_sphere = ivf::Sphere::create(1.0);
        add(m_sphere);
        return 0;
    }

    void onSceneUpdate(double time, double dt) override
    {
        // Pulse on every beat; grow with the "size" automation parameter
        const float pulse = std::exp(-6.0f * float(transport().beatPhase()));
        m_sphere->setScale(glm::vec3(param("size", 1.0f) * (1.0f + 0.3f * pulse)));
    }
};

int main(int argc, char **argv)
{
    ivfmusic::MusicSceneConfig config;
    config.title = "My Scene";
    config.audioFile = "assets/song.mp3";
    config.automationFile = "assets/automation.json";
    config.bpm = 120.0; // or set config.scoreFile to take the tempo from the composition
    return ivfmusic::run<MyScene>(config, argc, argv);
}
```

Link with `ivf2::ivfmusic`. It brings in `ivf2::ivf2`, so nothing else is needed. The `ivfmusic_add_executable()` helper also sets up the assets (see [Building](#building)).

## Song Time

`SongTransport` plays the song through the music stream of `AudioManager` and is the master clock. Each frame the window advances it and passes the song time to `onSceneUpdate(time, dt)`.

| Function | Meaning |
|----------|---------|
| `time()` | Song position in seconds |
| `beats()`, `bars()` | Fractional beats and bars since the first downbeat |
| `beatPhase()` | Position within the current beat, in [0, 1): 0 on the beat |
| `beatsAt(t)`, `timeAtBeat(b)`, `timeAtBar(b)` | Convert between seconds and musical time |
| `play()`, `pause()`, `seek(t)` | Transport control |

The tempo is constant. It comes from the score file, from `MusicSceneConfig::bpm`, or from `--bpm` on the command line, and `firstBeat` shifts the grid when the song does not start on a downbeat.

The window also passes three values to every `ShaderMaterial` as global uniforms, so shaders can follow the music without any C++ code:

```glsl
uniform float time;      // song time in seconds
uniform float beat;      // fractional beats
uniform float beatPhase; // [0, 1) within the current beat

float flash = exp(-6.0 * beatPhase); // 1 on the beat, decaying until the next
```

Without audio (no file, or no audio device) the transport runs silently on frame time. Its length is then taken from the score, or from `MusicSceneConfig::silentLength`.

!!! tip "Keep animation a function of time"
    Compute values from `time` (or the bar position) instead of accumulating them frame by frame. Then scrubbing, jumping to a marker and frame export all give the same picture. To accumulate a rate without state, use `Automation::integral()`.

## Automation

`Automation` holds named parameters keyed by **bar position**, as musicians count bars: `1.0` is the start of the song, `17.0` the start of bar 17, `17.5` halfway through it. It is usually loaded from a JSON file:

```json
// Comments are allowed
{
  "markers": [
    { "bar": 1,  "name": "Intro" },
    { "bar": 17, "name": "Verse" }
  ],
  "params": {
    "fog":   [ [1, 0.12], [17, 0.06, "sine"] ],
    "color": [ [1, [0.3, 0.6, 1.0]], [17, [1.0, 0.4, 0.2], "smooth"] ],
    "cameraPos": [
      [1,  [0, 2, 14]],
      [9,  [10, 4, 8], "spline"],
      [17, [0, 7, -10], "spline"]
    ]
  }
}
```

- A key is `[bar, value]` or `[bar, value, "easing"]`. The value is a number or a list of 1 to 4 numbers.
- The easing describes how the value **arrives** at that key from the previous one.
- Before the first key a parameter holds the first value; after the last key it holds the last value.
- Runs of `"spline"` keys form one smooth curve (Catmull-Rom) that keeps moving through its keys. That makes them suited to camera paths.

| Easing name | Curve |
|-------------|-------|
| `linear` | Straight line (default) |
| `step` | Hold the previous value, jump at the key |
| `smooth` | Smoothstep |
| `sine` | Sine in-out |
| `in`, `out` | Quadratic ease-in / ease-out |
| `cubic` | Cubic in-out |
| `spline` | Smooth curve through the neighbouring keys |
| Any `ivf::Easing` name | e.g. `quadInOut`, `elasticOut`, `bounceOut` (case-insensitive) |

Read the parameters at the current position from the window:

```cpp
float fog       = param("fog", 0.1f);           // first component, with a fallback
glm::vec3 color = param3("color", {1, 1, 1});
glm::vec4 v     = param4("tint");

// At any other position: automation().value("fog", barAt(time))
// Accumulated rate, still a pure function of the position:
float angle = float(automation().integral("spin", 1.0, barPosition()));
```

When the file changes on disk it is reloaded within half a second, so you can tune a scene while the music plays. A file with an error keeps the last good contents and shows the error in the transport bar and the automation panel (F3). Automation can also be built in code with `setKey()` and `setSplineKey()`.

## Score and Envelopes

Audio analysis can only guess which instrument plays when. With the composition itself, every note of every track is known exactly. Set `MusicSceneConfig::scoreFile` to a Renoise song (`.xrns`). The tempo and time signature are then taken from it, and `score()` gives the tracks, their notes, and the pattern sequence as `ScoreSection`s.

A `TrackEnvelope` turns one or more tracks into a smooth control value in [0, 1] using an ADSR shape:

```cpp
// In onSceneSetup()
m_bass = ivfmusic::TrackEnvelope(score(), {"Kick", "Bass"},
                                 ivfmusic::AdsrShape{0.01f, 0.25f, 0.0f, 0.2f});
m_pads = ivfmusic::TrackEnvelope(score(), {"Pads"},
                                 ivfmusic::AdsrShape{0.8f, 1.0f, 0.6f, 2.0f}); // slow "breathing"

// In onSceneUpdate()
m_glow->setIntensity(0.5f + m_bass.value(time));

// One event per note, e.g. to spawn a ripple
for (const auto &note : m_lead.notesBetween(time - dt, time))
    spawnRipple(note.pitch, note.velocity);
```

Envelopes are pure functions of time, so they are exact under scrubbing and export. The score monitor (F2) shows the envelope of every track while the song plays.

Renoise support needs miniz and pugixml and is controlled by the CMake option `IVF_WITH_RENOISE` (on by default). Without it, loading an `.xrns` file fails with an explanatory error, and `RenoiseScoreReader::available()` returns false.

## The Camera Path

By default the camera follows the automation parameters `cameraPos` and `cameraTarget`. Use `"spline"` keys for smooth flights. For a path computed in code, override `cameraAt()`. It must be a pure function of time:

```cpp
std::optional<ivf::TimelineCamera> cameraAt(double time) const override
{
    const float a = float(time) * 0.1f;
    return ivf::TimelineCamera::lookAt({12.0f * std::cos(a), 3.0f, 12.0f * std::sin(a)}, {0, 1, 0});
}
```

While watching, the viewer can look around (left mouse button) and float away from the path (right mouse button, wheel), within limits. The camera drifts back onto the path when left alone. This is handled by a `PathCameraManipulator`, available through `pathCamera()`; adjust its limits with `pathCamera()->settings()`. **F4** switches to the free orbit camera.

## Development, Presentation and Export

In development mode a transport bar along the bottom shows the song position, bar and beat, the current marker and the frame rate, with play/pause and a scrub slider.

| Key | Action |
|-----|--------|
| `P` | Play / pause |
| `Left` / `Right` | Seek 5 s (with `Shift`: one bar) |
| `PgUp` / `PgDn` | Previous / next marker |
| `Home` | Back to the start |
| `M` | Mute |
| `F1` | Toggle the development overlay |
| `F2` | Score monitor: envelope of every track |
| `F3` | Automation panel: markers (click to jump) and current parameter values |
| `F4` | Path camera / free orbit camera |
| `F5` | Live panel: MIDI devices, bindings, MIDI monitor, recording |
| `F6` | Start / stop recording a take (also in presentation mode) |

Add your own panels by overriding `onSceneUi()`. It is not called in presentation mode.

Command-line options override the defaults set in `MusicSceneConfig`:

| Option | Effect |
|--------|--------|
| `--present` (or `--show`) | Presentation mode: fullscreen, no development UI |
| `--fullscreen` | Fullscreen with development UI |
| `--size WxH` | Window size, e.g. `1920x1080` |
| `--start SECONDS` | Start at this song time |
| `--paused`, `--mute` | Start paused / muted |
| `--bpm BPM` | Override the tempo |
| `--export DIR` | Render a PNG sequence into `DIR`, then exit |
| `--fps FPS` | Frame rate of the export (default 60) |
| `--end SECONDS` | Stop exporting here (default: end of the song) |
| `--controls FILE` | MIDI bindings for live control (see [Live Control](#live-control)) |
| `--take FILE` | File recorded takes are saved to (default: `<automation>.take.json`) |
| `--no-live` | Ignore MIDI controllers |

An export runs silently in fixed steps of one video frame from `--start` to `--end`, independent of how fast frames render. Combine the PNG sequence with the song in a video editor or with ffmpeg:

```
music_scene1 --export frames --start 0 --end 60 --fps 60
ffmpeg -framerate 60 -i frames/frame_%06d.png -i song.mp3 -c:v libx264 -pix_fmt yuv420p -shortest out.mp4
```

## Live Control

A scene can be played live with MIDI controllers: faders and knobs change parameters on top of the automation, pads fire one-shot triggers or switch things on and off, and buttons run commands such as recording. The mapping lives in a JSON file, set with `MusicSceneConfig::controlsFile` or `--controls`, and reloads while the scene runs. Without a controls file no MIDI port is opened, so other programs keep access to the devices.

```json
{
  "devices": { "mixer": "nanoKONTROL", "pads": "MPD218" },
  "bindings": [
    { "device": "mixer", "cc": 0,  "param": "bloom",  "range": [0.0, 2.0], "pickup": true },
    { "device": "mixer", "cc": 1,  "param": "fog",    "range": [-0.05, 0.05], "mode": "offset" },
    { "device": "mixer", "cc": 2,  "param": "color",  "component": 0 },
    { "device": "pads",  "note": 36, "param": "blackout", "smoothing": 0.15 },
    { "device": "pads",  "note": 37, "param": "strobe", "toggle": true },
    { "device": "pads",  "note": 38, "trigger": "burst" },
    { "device": "mixer", "cc": 45, "action": "record" }
  ]
}
```

**Devices** map an alias to part of a MIDI port name (case-insensitive). A device is opened when it is found, also when it is plugged in later. Without a `devices` section all ports are opened.

**Bindings** listen to a `cc` or a `note`, optionally only from one `device` and `channel` (1-16), and drive a `param`, a `trigger` or an `action`:

| Key | Meaning |
|-----|---------|
| `mode` | `override` (default): replaces the automation value once the control has moved. `offset`: added to it. `scale`: multiplies it |
| `range` | `[min, max]` the control is mapped to (default `[0, 1]`) |
| `component` | One component (0-3) of a vector parameter; without it, all components |
| `smoothing` | Time constant in seconds (default 0.08), so stepped MIDI values move smoothly |
| `pickup` | Override only: take over when the control passes the current value, so nothing jumps |
| `toggle` | Pads and buttons: switch on and off with each press, instead of while held |
| `velocity` | Pads: the pressed value follows how hard the pad is hit |

A pad drives a parameter to `max` while held and back to `min` when released. A button that sends CC 127/0 behaves like a pad. A control has no effect until it is first moved, so a scene always starts as its automation describes.

**No scene changes are needed.** `param()`, `param3()` and `param4()` return the automation value with the live controls applied. **Triggers** are read with `triggerAge()` and `triggerVelocity()`, which also include triggers recorded in a take:

```cpp
// A burst that decays over a second after each hit of the pad
const float burst = triggerVelocity("burst") * float(std::exp(-3.0 * triggerAge("burst")));
```

`onLiveEvent()` is also called for each trigger, and for actions the window does not handle itself.

The window has built-in parameters and actions, so a performance has the essentials without any scene code:

| Name | Kind | Effect |
|------|------|--------|
| `blackout` | parameter (0-1) | Fades the picture to black |
| `strobe` | parameter (0-1) | White flashes on the beat |
| `strobeRate` | parameter | Beats per flash (default 0.25) |
| `play` | action | Play / pause |
| `record` | action | Start / stop recording a take |
| `release` | action | Hand all overrides back to the automation |
| `freeze` | action | Hold the picture while the music plays on (while held, or toggled) |
| `resetCamera` | action | Bring the viewer back onto the camera path |
| `nextMarker`, `previousMarker` | action | Jump between markers |
| `undoTake` | action | Undo the last recording |

`blackout` and `strobe` are drawn over the finished frame, after the effects, so they also appear in exports. Like any parameter, they can be keyed in the automation file as well.

The **F5** panel shows the devices and the ports available, every binding with its current value, and a MIDI monitor listing the last messages received. Use the monitor to find the numbers a controller sends.

## Recording Takes

A live performance can be recorded and kept. Press **F6** (or bind the `record` action), play the song and perform, then press it again. Every parameter that was under live control and every trigger that fired is written to the **take file**: by default the automation file with the extension `.take.json`, or the file given with `--take`.

A take works like a punch-in in a DAW. It holds only the stretches that were performed, as keys plus the regions they cover:

```json
{
  "markers": [],
  "params": {
    "bloom": [ [1.7366, 0.4724], [1.9346, 0.7768], [2.5103, 1.925], [2.7449, 1.984] ],
    "burst": [ [2.1096, 0.7874, "Step"] ]
  },
  "regions": {
    "bloom": [[1.7366, 2.7449]],
    "burst": [[1.5115, 2.7535]]
  }
}
```

Inside its regions the take replaces the automation; everywhere else the automation file applies exactly as written. The automation file itself is never rewritten, so it can still be edited by hand, and its comments are kept. A new take over an earlier one replaces it where they overlap. The recorded curves are thinned out to a few keys within a small tolerance of what was played.

When a recording stops, the recorded controls are released, so the take plays back what was performed instead of adding the live values on top. Because frame export ignores MIDI and reads the take, a performance exports frame-exact. **Undo take** in the F5 panel, or the `undoTake` action, restores the take file as it was before the last recording. To discard a recorded lane for good, delete its keys and regions from the take file.

## Building

`cmake/ivfmusic.cmake` provides a helper for applications:

```cmake
ivfmusic_add_executable(my_scene
    SOURCES main.cpp my_scene.cpp
    ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets
)
```

It links `ivf2::ivfmusic`, copies the `ASSETS` directory next to the executable as `assets/`, and defines `IVFMUSIC_ASSET_SOURCE_DIR`. With that definition, `MusicSceneConfig::resolve()` reads `assets/...` paths from the project directory when the file exists there. Edits to automation files and shaders in the project are then picked up immediately, without a rebuild. Projects that import prebuilt ivf2 libraries can name their own target with `LIBS`.

## Glow, Blending and Guided Cameras

These classes come from the same work but live in the core libraries and work in any `GLFWSceneWindow`.

### MipBloomEffect

A wide, soft bloom with tone mapping. The bright parts of the image are taken from several mip levels of the rendered frame (blurs of increasing size) and added back. A soft shoulder then rolls values above 1 off into white instead of clipping them. Compared with `BloomEffect` the halo is much wider and cheap to compute. It suits glowing particles and light against dark backgrounds. Place it last in the effect chain.

```cpp
#include <ivf/mip_bloom_effect.h>

auto bloom = ivf::MipBloomEffect::create();
bloom->setThreshold(0.6f); // where the glow starts
bloom->setIntensity(0.8f); // how much is added
bloom->setSpread(0.75f);   // weight per blur level: higher gives wider halos
bloom->setExposure(1.0f);  // scale before tone mapping
bloom->load();
addEffect(bloom);
setRenderToTexture(true);
```

!!! note
    Specular highlights on large surfaces become large glowing patches under a strong bloom. For dark grounds, set the material's specular colour to black.

### BlendShaderMaterial

A `ShaderMaterial` that draws with blending and without depth writes. It restores the previous OpenGL state afterwards.

| `BlendMode` | Use |
|-------------|-----|
| `Additive` | Overlapping light adds up: glowing orbs, light trails, sparks |
| `Alpha` | Translucent surfaces: mist, veils |
| `Background` | Opaque without depth writes: a sky dome drawn first, which leaves the depth buffer empty for `FogEffect` |

```cpp
auto glow = ivf::BlendShaderMaterial::createFromFiles("shaders/orb.vert", "shaders/orb.frag",
                                                      "orb", ivf::BlendMode::Additive, true /* hot reload */);
orb->setMaterial(glow);
```

Depth testing stays on, so blended geometry is hidden behind opaque geometry. Add blended nodes **after** the opaque ones.

### Ready-made blended materials

For the common cases there is no need to write GLSL. These classes derive from `BlendShaderMaterial`, bring their own shaders, and are set up with typed setters. Each class shares one shader program between all its instances, so a material per colour is cheap.

| Class | Blend mode | Use | Setters |
|-------|------------|-----|---------|
| `GlowMaterial` | `Additive` | A ball of light: bright where the surface faces the viewer, fading towards the silhouette | `setColor`, `setBrightness`, `setFalloff` |
| `MistMaterial` | `Alpha` | A soft layer of haze on a flat shape such as a `Plane` or `Disk`, densest at the node's origin | `setColor`, `setOpacity`, `setRadius`, `setSoftness` |
| `GradientSkyMaterial` | `Background` | A sky dome: zenith-to-horizon gradient, a glow band on the horizon and optional twinkling stars | `setZenithColor`, `setHorizonColor`, `setGroundColor`, `setGradientExponent`, `setGlowColor`, `setGlowSharpness`, `setStarIntensity` |

```cpp
// Sky dome first; it is centered on the camera, so its radius only has to fit within the far plane
auto sky = ivf::GradientSkyMaterial::create({0.0f, 0.0f, 0.02f}, {0.05f, 0.04f, 0.1f});
sky->setGlowColor({0.08f, 0.05f, 0.14f});
sky->setStarIntensity(0.8f);
auto dome = ivf::Sphere::create(50.0, 64, 32);
dome->setMaterial(sky);
add(dome);

// ... opaque geometry ...

auto glow = ivf::GlowMaterial::create({1.0f, 0.55f, 0.2f}, 1.5f);
orb->setMaterial(glow);

auto mist = ivf::MistMaterial::create({0.5f, 0.45f, 0.7f}, 0.35f, 12.0f);
mistPlane->setMaterial(mist);
```

Driving them from automation is a matter of calling the setters in `onSceneUpdate()`, for example `glow->setBrightness(1.0f + param("pulse") * beatPulse)`. The stars twinkle with the global `time` uniform, which `GLFWMusicSceneWindow` sets to the song position.

Use a plain `BlendShaderMaterial` with your own shader files when a look needs its own vertex animation, instancing or scene-specific uniforms.

### PathCameraManipulator

Adds limited viewer freedom on top of an authored camera path: look around, float away, and return to the path after a moment without input. Supply the path every frame as an `ivf::TimelineCamera` and forward the mouse events:

```cpp
#include <ivfui/path_camera_manipulator.h>

void onUpdate() override
{
    m_pathCamera->update(frameTime());
    m_pathCamera->applyTo(cameraManipulator(), myPath(elapsedTime()));
}

void onMouseDrag(double x, double y, double dx, double dy, int button) override
{
    GLFWSceneWindow::onMouseDrag(x, y, dx, dy, button);
    m_pathCamera->handleMouseDrag(dx, dy, button);
}

void onScroll(double xoffset, double yoffset) override
{
    GLFWSceneWindow::onScroll(xoffset, yoffset);
    m_pathCamera->handleScroll(yoffset);
}
```

`applyTo()` sets only the camera position and target. Field of view and clip planes stay as configured on the window's camera manipulator.

## Choosing an Animation Tool

| Tool | Time base | Seekable | Use for |
|------|-----------|----------|---------|
| `ivfmusic::Automation` | Musical bars of the song | Yes, a pure function of the position | Continuous changes synchronised to music: colours, fog, camera paths, effect amounts |
| `ivf::SceneTimeline` | Seconds, advanced per frame | Seek between scenes | Sequencing whole scenes, each with its own root, camera and effects |
| `ivf::Tween` / `TweenManager` | Seconds from when the tween starts | No | Transitions triggered by events or input |
| `ivf::KeyframeAnimation`, `SplineAnimation` | Seconds, advanced per frame | Play/stop | Moving one node through keyframes or along a spline |
| `ivf::TimeController` | Scaled, pausable seconds | Reset | Pause and slow motion for a whole scene (see [Time-Driven Animation](time_and_animation.md)) |

## Examples

- `music_scene1`: a ring of orbs pulsing on the beat, with automated colour, bloom and a spline camera flight. Edit `examples/music_scene1/assets/music_scene1.json` while it runs.
- `mip_bloom1`: `MipBloomEffect`, `GlowMaterial` orbs, a `MistMaterial` layer, a `GradientSkyMaterial` sky and `PathCameraManipulator` on an orbit.

See [Music Scene Examples](../examples/music_examples.md) for the full sources.
