# AGENTS.md

This file provides guidance to Codex (claude.ai/code) when working with code in this repository.

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

Everything is built under `build/<preset>/`: libraries in `lib/<Config>/`, executables plus copied runtime data (`shaders/`, `fonts/` from `data/`, `assets/` from `examples/assets/`) in `bin/<Config>/`. `-DIVF2_OUTPUT_IN_SOURCE_TREE=ON` restores the old `<source>/bin/<Config>` layout.

Library targets are static, created with `ivf2_add_library()` and exported as `ivf2::<name>`; dependencies are linked per target (no global include dirs). New `.cpp`/`.h` files in `src/<lib>/` and `include/<lib>/` are picked up automatically (`CONFIGURE_DEPENDS` globs), except `src/generator`, which lists its files explicitly. Unit tests (GL-free) live in `tests/`. Builds use `/W4` (MSVC) or `-Wall -Wextra -Wpedantic` (`IVF2_WARNINGS`, on for top-level builds); keep new code warning-free (unused parameters: comment out the name; narrowing: `static_cast`).

## Running Examples

Examples are the primary way to validate functionality. Each example is a standalone executable built alongside the library:

```
build/vs/bin/Debug/hello_ivfd       # Basic primitives and scene setup
build/vs/bin/Debug/lighting1d       # Lighting configuration
build/vs/bin/Debug/textures1d       # Texture mapping
build/vs/bin/Debug/animation1d      # Keyframe animation
build/vs/bin/Debug/procedural1d     # Geometry generation
build/vs/bin/Debug/music_scene1d  # ivfmusic: song-driven scene (--help for options)
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

### Manager Singletons

Global state is managed via singleton managers, accessed through static `instance()`:

- `ShaderManager` — GLSL program lifecycle
- `LightManager` — point/directional/spot lights (max defined in shaders)
- `TextureManager` — texture loading and binding
- `FontManager` — FreeType font loading

### Key Patterns

- **Factory methods**: All major classes use `ClassName::create(...)` returning `std::shared_ptr<ClassName>`. Never use raw `new`.
- **Naming**: getters are named after the property and are `const` where they only read (`texture()`, `material() const`), setters `setX()`; a `get` prefix only for functions that return through reference parameters (`getColor(r, g, b, a)`). Factories are `[[nodiscard]]`; strings are passed as `const std::string &` or `std::string_view`. Renamed APIs keep a `[[deprecated]]` alias.
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
    void onInit() override { /* scene setup */ }
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

The library's built-in shaders are embedded as C++ string constants (`stock_shaders.h`, `pbr_shaders.h`, `post_shaders.h`, ...). File-based shaders used by examples live in `data/shaders/` and are copied next to the executables at build time (`copy_example_assets` / `ivf2_copy_runtime_files`) and installed with the library; `ShaderManager::compileProgramFromFiles` (returns `Result<ProgramPtr>`) resolves them relative to the working directory. Example models, textures and sounds are in `examples/assets/`.

## Documentation

- User guide (Zensical): `docs/docs/`, configured by `docs/zensical.toml`
- API reference (Doxygen): `Doxyfile` at repo root; generated output in `docs/docs/doxygen/html/`
- Build docs: `.readthedocs.yaml` (Python 3.11, doxygen, graphviz, zensical required)
