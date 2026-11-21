# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**ivf2** (Interactive Visualisation Framework 2) is a modern C++20 scene graph-based graphics and visualization library built on OpenGL 3.3+. It is a complete rewrite of the original ivfplusplus library. Documentation: https://interactive-visualisation-framework-2.readthedocs.io/

## Build Commands

**Requirements**: CMake 3.24+, vcpkg (auto-detected: `CMAKE_TOOLCHAIN_FILE` > `VCPKG_ROOT` > common paths > VS-bundled), C++20 compiler. Build files live in `cmake/` (`IvfVcpkgBootstrap`, `IvfTargets`, `IvfInstall`, `ivf2Helpers`, `ivfmusic`).

**Windows (Visual Studio)**:
```
cmake --preset vs
cmake --build --preset vs-debug
cmake --build --preset vs-release
ctest --preset vs-debug
```

**Linux/macOS (Ninja)**:
```
cmake --preset debug        # or release, ninja (multi-config), lib-only
cmake --build --preset debug
ctest --preset debug
```

Everything is built under `build/<preset>/`: libraries in `lib/<Config>/`, executables plus copied runtime data (`shaders/`, `fonts/`, `assets/` from `bin/`) in `bin/<Config>/`. `-DIVF2_OUTPUT_IN_SOURCE_TREE=ON` restores the old `<source>/bin/<Config>` layout.

Library targets are static, created with `ivf2_add_library()` and exported as `ivf2::<name>`; dependencies are linked per target (no global include dirs). New `.cpp`/`.h` files in `src/<lib>/` and `include/<lib>/` are picked up automatically (`CONFIGURE_DEPENDS` globs), except `src/generator`, which lists its files explicitly. Unit tests (GL-free) live in `tests/`. Optional dependencies (`IVF2_WITH_ASSIMP`, `IVF2_WITH_FREETYPE`, `IVF2_WITH_AUDIO`, `IVF2_WITH_MIDI`, all default ON) are private to `ivf`: public headers must not include assimp/FreeType/miniaudio/RtMidi/stb/nlohmann-json/ImGui (ivf core) — the `header_check_<lib>` targets built with the tests enforce this and header self-containment; guard implementations with the `IVF2_HAS_*` macros from the generated `<ivf/config.h>` and keep a no-op fallback. The `minimal` preset (all off) runs in CI.

**CI**: `.github/workflows/build.yml` runs the same presets on Windows (`vs`/`vs-release`), Ubuntu and macOS (`release`), then `ctest` and a package consumer build (`tests/package/`, a standalone project using `find_package(ivf2)`). vcpkg is pinned to `builtin-baseline` from `vcpkg.json`; keep code portable (GCC 13 / AppleClang, case-sensitive includes).

## Running Examples

Examples are the primary way to validate functionality. Each example is a standalone executable built alongside the library:

```
build/vs/bin/Debug/hello_ivfd       # Basic primitives and scene setup
build/vs/bin/Debug/lighting1d       # Lighting configuration
build/vs/bin/Debug/textures1d       # Texture mapping
build/vs/bin/Debug/animation1d      # Keyframe animation
build/vs/bin/Debug/procedural1d     # Geometry generation
bin/Debug/music_scene1    # ivfmusic: song-driven scene (--help for options)
```

Music scenes can be smoke-tested with a short frame export, which renders a few PNGs and exits by itself: `music_scene1 --export <dir> --start 10 --end 11 --fps 5`.

## Code Formatting

Clang Format is configured (`.clang-format`): Microsoft `BasedOnStyle`, 4-space indent, Microsoft-style braces, sorted includes disabled.

## Architecture

### Layer Overview

1. **ivfui** — GLFW window/event management + ImGui integration
2. **ivf** — Core scene graph, rendering, primitives, materials, shaders, effects, animation
3. **generator** — Procedural geometry generation library (headers in `include/generator/`, sources in `src/generator/`, built as `lib/*/generator.lib`)
4. **ivfmath** — Keyframe/spline interpolation, spherical math
5. **ivfmusic** — Scenes driven by a song: `GLFWMusicSceneWindow`, `SongTransport`, `Automation` (bar-keyed parameters from JSON), `Score`/`TrackEnvelope` (Renoise `.xrns` notes), `LiveControls` (MIDI bindings from JSON applied in `param()`; opens ports only with a controls file) and `AutomationRecorder` (live takes saved punch-in style to `<automation>.take.json`, laid over the automation only inside their regions). Separate target `ivf2::ivfmusic`, not part of `ivf2::ivf2`; apps use `ivfmusic_add_executable()` from `cmake/ivfmusic.cmake`. Renoise support is behind the CMake option `IVF_WITH_RENOISE` (miniz, pugixml)

### Scene Graph

All drawable entities inherit from `Node`. The hierarchy is:

- `Node` — base class (materials, textures, visibility, selection, property inspection)
  - `TransformNode` — adds position/rotation/scale transform
    - Primitives (`Cube`, `Sphere`, `Cylinder`, `Cone`, `Box`, `RoundedBox`, `Capsule`, `Plane`, `Disk`, `Dodecahedron`, `Extrusion`, ...)
    - `MeshNode` — mesh from Assimp-loaded models
    - `TextNode` — text rendering
  - `CompositeNode` — groups child nodes (Composite pattern)

### Render Context and Managers

GL-bound state lives in an `ivf::RenderContext` (`include/ivf/render_context.h`) — one per OpenGL context, defined in `ivf` with no windowing dependency. It owns, created lazily on first use: `ShaderManager` (`shaders()`), `LightManager` (`lights()`), `TextureManager` (`textures()`), `FontManager` (`fonts()`), `MeshManager` (`meshes()`), `SelectionManager` (`selection()`), `TransformManager` (`transforms()`, caches uniform locations of the context's program), `DebugDraw`, `ShaderWatcher`, and the fallback textures.

- `GLFWWindow` creates its context (loading GL via `RenderContext::create(glfwGetProcAddress)`), and `makeCurrent()` switches GL, RenderContext and ImGui context together — before drawing and before input callbacks. `GLFWSceneWindow` calls `renderContext()->setupDefaults()` (default shaders, font, light).
- In window code use `renderContext()->lights()` etc. `XxxManager::instance()`/`create()` still work: they forward to `RenderContext::current()` (thread-local), or to a leaked process-wide fallback with a warning if none is current. `drop()` is a no-op. Library internals still use `instance()` (explicit passing is planned with the phase 2 render queue).
- Non-GL globals stay singletons: `Logger`, `TimeController`, `TweenManager`, `TimerManager`, `AudioManager`.
- Don't add function-local `static` GL objects or binding caches — they break with several contexts; put per-context GL state in `RenderContext`.
- Other hosts (Qt, SDL, EGL): see `examples/embedding1`; several windows: `examples/multi_window1`.

### Key Patterns

- **Factory methods**: All major classes use `ClassName::create(...)` returning `std::shared_ptr<ClassName>`. Never use raw `new`.
- **Smart pointers**: `std::shared_ptr` throughout. Type aliases follow the pattern `using NodePtr = std::shared_ptr<Node>`.
- **Property inspection**: `PropertyInspectable` interface enables runtime editor integration.
- **Visitor pattern**: `NodeVisitor` for scene traversal.

### Application Entry Point

Subclass `ivfui::GLFWWindow` and override event methods. Launch via `ivfui::GLFWApplication`:

```cpp
#include <ivfui/glfw_application.h>
#include <ivfui/glfw_window.h>
#include <ivf/gl.h>  // includes all core ivf classes

class MyWindow : public ivfui::GLFWWindow {
    int onSetup() override { /* scene setup, renderContext()->lights() ... */ return 0; }
    void onDraw() override { /* per-frame updates */ }
    // onResize, onKey, onMouseButton, onMouseMove, onScroll, ...
};

int main() {
    auto app = ivfui::GLFWApplication::create();
    auto win = MyWindow::create(800, 600, "Title");
    app->addWindow(win);
    app->run();
}
```

### Effects / Post-Processing

Effects belong to the `GLFWSceneWindow`: call `setRenderToTexture(true)`, create the effect, call `load()`, then `addEffect()`; they run in the order added. Available effects include: `BlurEffect`, `BloomEffect`, `MipBloomEffect` (wide bloom from the mip chain with tone mapping; place last), `ChromaticEffect`, `DitheringEffect`, `FilmGrainEffect`, `FogEffect`, `PixelationEffect`, `TintEffect`, `VignetteEffect`, `EdgeDetectionEffect`.

Custom shaders on nodes use `ShaderMaterial`; `BlendShaderMaterial` adds additive, alpha or background blending without depth writes (add blended nodes after opaque ones); `GlowMaterial`, `MistMaterial` and `GradientSkyMaterial` are ready-made subclasses with built-in shaders and typed setters (same pattern as `WaterMaterial`). `ivfui::PathCameraManipulator` lets the viewer look around an authored camera path (`ivf::TimelineCamera`) and returns to it.

### Animation and Time

`ivf::Easing` (in `tween.h`, with `easingName()`/`EasingCount`) is shared by `Tween` and `ivfmusic::Automation`. Use `Automation` for seekable, song-synchronised parameters, `SceneTimeline` for sequencing scenes, `Tween` for event-driven transitions. `AudioManager`'s music stream (`startMusic`, `seekMusic`, `musicPosition`, ...) plays one file with transport control and feeds the analyser; the older `*FileAnalysis` names are aliases.

### Deformers

Mesh deformation is applied to geometry nodes. Available: `ScaleDeformer`, `TwistDeformer`, `BendDeformer`, `WaveDeformer`, `TurbulenceDeformer`, `RandomDeformer`, `FunctionDeformer`.

### Procedural Geometry (generator)

The `generator` library is included via `#include <generator/generator.hpp>` (Z-up; `MeshNode::createFromGenerator()` swaps Y and Z). Generators produce meshes, paths, and shapes that can be composed (repeat, transform, scale, rotate, mirror, subdivide). Use `ivf::MeshNode` to consume generator output.

### Physics

ReactPhysics3D integration is in `examples/rigid_body1/`. The framework does not wrap ReactPhysics3D directly — examples show manual integration.

## Dependencies (vcpkg.json)

GLFW3, OpenGL, ImGui (docking), Assimp, FreeType, stb, nlohmann-json, miniaudio, RtMidi; vcpkg features `examples` (ReactPhysics3D) and `renoise` (miniz, pugixml) are enabled from the CMake options, GLM (header-only, bundled in `include/glm/`).

## Shaders

The library's built-in shaders are embedded as C++ string constants (`stock_shaders.h`, `pbr_shaders.h`, `post_shaders.h`, ...). File-based shaders used by examples live in `bin/shaders/` and are copied next to the executables at build time (`copy_example_assets` / `ivf2_copy_runtime_files`); `ShaderManager::loadProgramFromFiles` resolves them relative to the working directory. The root `shaders/` directory is an older, diverged copy and is not used by the build.

## Documentation

- User guide (Zensical): `docs/docs/`, configured by `docs/zensical.toml`
- API reference (Doxygen): `Doxyfile` at repo root; generated output in `docs/docs/doxygen/html/`
- Build docs: `.readthedocs.yaml` (Python 3.11, doxygen, graphviz, zensical required)
