# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**ivf2** (Interactive Visualisation Framework 2) is a modern C++23 scene graph-based graphics and visualization library built on OpenGL 3.3+. It is a complete rewrite of the original ivfplusplus library. Documentation: https://interactive-visualisation-framework-2.readthedocs.io/

## Status

Latest release 2.0.0-alpha.2; `main` adds the render queue (see `CHANGELOG.md` `[Unreleased]`). `ROADMAP.md` tracks the plan: phases 0 (build) and 1 (foundations) are done apart from listed open items, phase 2 item 1 (render queue, steps 1-7) is done, and phase 2 item 2 (uniform handling: uniform location cache, `IvfCamera`/`IvfLights` uniform blocks, one draw item per mesh, automatic instancing) is done, phase 2 item 3 (HDR pipeline: output step with exposure, tone mapping, gamma, FXAA; MSAA samples) is done, phase 2 item 4 (image-based lighting: `EnvironmentMap`, `PBRMaterial::setEnvironment()`) is done, phase 2 item 5 (better shadows: cascades, PCF/PCSS, point and spot light shadows) is done, phase 2 item 6 (`SsaoEffect`, `ScreenSpaceReflectionEffect`) is done, phase 2 item 7 (`GltfModel`: glTF 2.0 with PBR materials, skinning, morph targets, animations) is done, phase 2 item 8 (`ResourceWatcher`: hot reload of textures, models and shaders) is done, and **phase 2 item 9 (CPU picking) is next**. The "Status" section at the top of `ROADMAP.md` lists what is open; when finishing roadmap work, update that section and add a *Done:* note to the item.

## Build Commands

**Requirements**: CMake 3.24+, C++23 compiler (MSVC 2022+, GCC 13+, Clang 17+, Xcode 16+), vcpkg (auto-detected: `CMAKE_TOOLCHAIN_FILE` > `VCPKG_ROOT` > common paths > VS-bundled), C++20 compiler. Build files live in `cmake/` (`IvfVcpkgBootstrap`, `IvfTargets`, `IvfInstall`, `ivf2Helpers`, `ivfmusic`).

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

Everything is built under `build/<preset>/`: libraries in `lib/`, executables plus copied runtime data (`shaders/`, `fonts/` from `data/`, `assets/` from `examples/assets/`) in `bin/` (`build/release/bin/`); multi-config generators (Visual Studio, Xcode, Ninja Multi-Config) add a `<Config>` level (`build/vs/bin/Debug/`). `-DIVF2_OUTPUT_IN_SOURCE_TREE=ON` puts `bin/` and `lib/` in the source tree instead.

Library targets are static, created with `ivf2_add_library()` and exported as `ivf2::<name>`; dependencies are linked per target (no global include dirs). New `.cpp`/`.h` files in `src/<lib>/` and `include/<lib>/` are picked up automatically (`CONFIGURE_DEPENDS` globs), except `src/generator`, which lists its files explicitly. Unit tests (GL-free) live in `tests/`. Builds use `/W4` (MSVC) or `-Wall -Wextra -Wpedantic` (`IVF2_WARNINGS`, on for top-level builds); keep new code warning-free (unused parameters: comment out the name; narrowing: `static_cast`). Optional dependencies (`IVF2_WITH_ASSIMP`, `IVF2_WITH_FREETYPE`, `IVF2_WITH_AUDIO`, `IVF2_WITH_MIDI`, all default ON) are private to `ivf`: public headers must not include assimp/FreeType/miniaudio/RtMidi/stb/nlohmann-json/ImGui (ivf core) — the `header_check_<lib>` targets built with the tests enforce this and header self-containment; guard implementations with the `IVF2_HAS_*` macros from the generated `<ivf/config.h>` and keep a no-op fallback. The `minimal` preset (all off) runs in CI.

**CI**: `.github/workflows/build.yml` runs the same presets on Windows (`vs`/`vs-release`), Ubuntu and macOS (`release`), then `ctest` and a package consumer build (`tests/package/`, a standalone project using `find_package(ivf2)`). To save runner minutes its `plan` job builds only Linux (GCC, no visual tests) on pushes to `main`; pull requests, the nightly schedule (skipped without new commits), manual runs and `release.yml` (version tags; creates the release only if the build passes) get the full matrix. Linux/macOS use ccache. CodeQL runs weekly, docs only when their sources change. vcpkg is pinned to `builtin-baseline` from `vcpkg.json`; keep code portable (GCC 13 / AppleClang, case-sensitive includes).

## Error handling

Expected failures (files, formats, compile errors) return `ivf::Result<T>` (`std::expected<T, ivf::Error>`, `ivf/result.h`); use `return ivf::makeError(ErrorCode::..., "message incl. file name", "Context");`, which also logs. Exceptions only for programming errors. Results are `[[nodiscard]]` — callers check them (`if (!tex->load(...)) return -1;`) or `static_cast<void>(...)` with a comment. Old APIs kept as `[[deprecated]]` wrappers (MSVC shows them via `/w14996`).

## Diagnostics and unattended runs

- Debug builds (or `IVF2_GL_DEBUG=1`) get an OpenGL debug context; driver messages are logged with context `OpenGL` (`ivf/gl_debug.h`). Breakpoint on `ivf::logError` gives the stack of the offending GL call. Expect zero `ERROR`/`WARN` lines from examples. Log line format: `HH:MM:SS.mmm LEVEL Context16 message` (see `Logger::formatMessage`; `setShowDate`, `setContextWidth`, `setWrapColumn` - wraps at column 100 by default).
- Run examples without showing windows: `IVF2_HIDE_WINDOWS=1 IVF2_EXIT_AFTER_FRAMES=90 IVF2_SCREENSHOT=out.png` (PowerShell: set `$env:...`). `embedding1` uses raw GLFW and ignores these. `IVF2_FIXED_TIMESTEP`, `IVF2_RANDOM_SEED` and `IVF2_SCREENSHOT_UI=0` make runs repeatable.
- Visual regression tests (`-DIVF2_VISUAL_TESTS=ON`, `ctest -L visual`, `tests/visual/`): every example except `embedding1`/`music_scene1` is compared with `tests/visual/reference/`, rendered by CI with Mesa llvmpipe (Linux job, artifact `visual-tests`). Examples must not use the wall clock (`glfwGetTime`) or unseeded randomness (use `ivf::automaticSeed()`); intended visual changes need updated references from that artifact.
- Use the Logger (`ivf::logInfo/logWarning/logError/logDebug`, `ivf::logStr(...)` for mixed values) instead of `std::cout`; `std::cout` is only for explicit `print()`/dump helpers.
- Label new GL objects with `ivf::glLabel(GL_TEXTURE, id, "name")` after they are first bound.

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

Drawing goes through a render queue (`render_queue.h`, `renderer.h`): `Node::draw()` calls `collect()` (one `DrawItem` per drawable node; `CompositeNode`/`InstanceNode` pass transform, material and textures down via `CollectState`) and `Renderer::execute()`, which sets each item's parent matrix, applies the inherited material/textures if the node has none, and calls the node's `doPreDraw()`/`doDraw()`/`doPostDraw()`. `GLFWSceneWindow` collects once per frame and draws that queue in the shadow, reflection, main and selection passes (selection only after mouse input or camera changes); items carry world-space `bounds`, and each pass culls them with its frustum (`frustum.h`). Bounding boxes must enclose what a node draws: generator primitives get them from `createFromGenerator()`; nodes that change vertices call `MeshNode::updateVertices()` (or `updateBoundingBox()`); nodes without a box are never culled. Materials inherit from the nearest ancestor (a group's material applies to children without their own); a mesh's own material (`Mesh::setMaterial()`) wins over its node's. `MeshNode` adds one item per enabled mesh (`DrawItem::mesh`); the node's hooks run per item and `MeshNode::doDraw()` draws only that mesh, so subclasses overriding `doDraw()` should call `MeshNode::doDraw()` to draw. Automatic instancing: consecutive opaque items of the same node, mesh, material and textures (`InstanceNode` copies; `Renderer::instanceable()`) are drawn as one batch through `DrawContext::instances` and `Mesh::drawInstances()` (falls back to one draw per copy for shaders without `useInstancing`); `RenderContext::setAutoInstancing()`, `IVF2_AUTO_INSTANCING=0`. Per-instance attributes (locations 4-9) are defined in `instancing.h`. See ROADMAP phase 2.1 for the design and next steps.

### Render Context and Managers

GL-bound state lives in an `ivf::RenderContext` (`include/ivf/render_context.h`) — one per OpenGL context, defined in `ivf` with no windowing dependency. It owns, created lazily on first use: `ShaderManager` (`shaders()`), `LightManager` (`lights()`), `TextureManager` (`textures()`), `FontManager` (`fonts()`), `MeshManager` (`meshes()`), `SelectionManager` (`selection()`), `TransformManager` (`transforms()`, caches uniform locations of the context's program), `DebugDraw`, `ResourceWatcher` (`resourceWatcher()`: hot reload of textures, models, shaders; `ShaderWatcher` registers with it), and the fallback textures.

- `GLFWWindow` creates its context (loading GL via `RenderContext::create(glfwGetProcAddress)`), and `makeCurrent()` switches GL, RenderContext and ImGui context together — before drawing and before input callbacks. `GLFWSceneWindow` calls `renderContext()->setupDefaults()` (default shaders, font, light).
- In window code use `renderContext()->lights()` etc. `XxxManager::instance()`/`create()` still work: they forward to `RenderContext::current()` (thread-local), or to a leaked process-wide fallback with a warning if none is current. `drop()` is a no-op. The render queue path takes the context explicitly (`Renderer(ctx, pass)`, `renderShadowMaps(ctx, queue)`, `PlanarReflection::render(ctx, queue)`, `Node::draw(ctx)`, `Mesh::draw(ctx)`, `DrawContext` in `Node::doDrawPass()`); materials and other library internals still use `instance()`. Render statistics: `Renderer::stats()`, `GLFWSceneWindow::renderStats()`, `IVF2_RENDER_STATS=n`.
- Non-GL globals stay singletons: `Logger`, `TimeController`, `TweenManager`, `TimerManager`, `AudioManager`.
- Don't add function-local `static` GL objects or binding caches — they break with several contexts; put per-context GL state in `RenderContext`.
- `Mesh` holds CPU data only until drawn: building or changing geometry (`end()`, `updateVertices()`, `updateNormals()`, primitive setters) needs no GL context, and each `RenderContext` that draws a mesh gets its own GPU copy (so nodes can be shared between windows). Attribute locations are fixed (`aPos` 0, `aColor` 1, `aTex` 2, `aNormal` 3, bound by `ShaderManager`). GL objects of a context that is not current are freed with `RenderContext::deleteLater()`; `RenderContext::lifetime()` tells whether a context still exists. `tests/test_scene_without_gl.cpp` builds scenes with no GL functions loaded.
- Other hosts (Qt, SDL, EGL): see `examples/embedding1`; several windows: `examples/multi_window1`.

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

Effects belong to the `GLFWSceneWindow`: call `setRenderToTexture(true)`, create the effect, call `load()`, then `addEffect()`; they run in the order added. The scene image and effect chain are RGBA16F (HDR); the final output step (the `render_to_texture` program, `post_shaders.h`) applies `setExposure()`, `setToneMapping()` (`ivf::ToneMapping`, `tone_mapping.h`), `setGamma()` and `setFxaa()` after all effects (defaults copy unchanged; tone mapping or FXAA turn render to texture on); `setMsaaSamples()` sets the offscreen multisampling (default 4). With window tone mapping use `MipBloomEffect::setToneMapping(false)`. Image-based lighting: `EnvironmentMap` (`.hdr` file, float data or `loadSky()`) precomputes environment, irradiance and prefiltered cube maps and the BRDF LUT on the GPU (needs a current context); `PBRMaterial::setEnvironment()` uses them (texture units 12-14), `Skybox::create(env->cubemap())` shows it. PBR materials tone map in their shader unless `setToneMapping(false)` (then use window tone mapping with `setGamma(2.2f)`). Effects get `screenTexture`, `previousFrame`, `depthTexture`, `projection`, `view`, `invProjection`, `invView` and `time`; `SsaoEffect` and `ScreenSpaceReflectionEffect` (`screen_space_shaders.h`) work from depth alone. Available effects include: `BlurEffect`,`BloomEffect`, `MipBloomEffect` (wide bloom from the mip chain with tone mapping; place last), `ChromaticEffect`, `DitheringEffect`, `FilmGrainEffect`, `FogEffect`, `PixelationEffect`, `TintEffect`, `VignetteEffect`, `EdgeDetectionEffect`.

Custom shaders on nodes use `ShaderMaterial`; `BlendShaderMaterial` adds additive, alpha or background blending without depth writes (the render queue draws blended and alpha < 1 nodes after opaque ones, back to front; no need to order them by hand); `GlowMaterial`, `MistMaterial` and `GradientSkyMaterial` are ready-made subclasses with built-in shaders and typed setters (same pattern as `WaterMaterial`). `ivfui::PathCameraManipulator` lets the viewer look around an authored camera path (`ivf::TimelineCamera`) and returns to it.

### Animation and Time

`ivf::Easing` (in `tween.h`, with `easingName()`/`EasingCount`) is shared by `Tween` and `ivfmusic::Automation`. Use `Automation` for seekable, song-synchronised parameters, `SceneTimeline` for sequencing scenes, `Tween` for event-driven transitions. `AudioManager`'s music stream (`startMusic`, `seekMusic`, `musicPosition`, ...) plays one file with transport control and feeds the analyser; the older `*FileAnalysis` names are aliases.

### Deformers

Mesh deformation is applied to geometry nodes. Available: `ScaleDeformer`, `TwistDeformer`, `BendDeformer`, `WaveDeformer`, `TurbulenceDeformer`, `RandomDeformer`, `FunctionDeformer`.

### Procedural Geometry (generator)

The generator always uses glm (`GENERATOR_USE_GLM` is forced in `generator/math.hpp`; `gml::` names are glm aliases).

The `generator` library is included via `#include <generator/generator.hpp>` (Z-up; `MeshNode::createFromGenerator()` swaps Y and Z). Generators produce meshes, paths, and shapes that can be composed (repeat, transform, scale, rotate, mirror, subdivide). Use `ivf::MeshNode` to consume generator output.

### Physics

ReactPhysics3D integration is in `examples/rigid_body1/`. The framework does not wrap ReactPhysics3D directly — examples show manual integration.

## Dependencies (vcpkg.json)

GLFW3, GLM (`glm::glm`, the only math library — also used by the generator), OpenGL, ImGui (docking), cgltf (glTF, header-only, private to `gltf_model.cpp`), Assimp,FreeType, stb, nlohmann-json, miniaudio, RtMidi; vcpkg features `examples` (ReactPhysics3D) and `renoise` (miniz, pugixml) are enabled from the CMake options. Only glad/KHR are still vendored in `include/`.

## Shaders

Camera data reaches shaders through the std140 uniform block `IvfCamera` (`view`, `projection`, `viewProjection`, `viewPos`; `include/ivf/uniform_blocks.h`, `IVF_CAMERA_BLOCK_GLSL` for C++ shader strings), bound by `Program::link()` and written by `TransformManager::syncCameraBlock()`. New shaders declare the block instead of plain `view`/`projection`/`viewPos` uniforms (those still work for old shaders, but a program can't mix both); don't set camera uniforms by name on the built-in programs. Lights likewise come from `IvfLights` (`IVF_LIGHTS_BLOCK_GLSL`: light structs, `pointLights`/`dirLights`/`spotLights`, `shadows`, counts; binding 1), written by `LightManager::apply()`/`syncLightsBlock()` when lights changed. Programswith plain light uniforms still get the old per-light upload. Shadows: up to four shadow-casting directional, point or spot lights; each has a `ShadowData` entry in the block (`shadowIndex` on the light) and a 2D depth atlas (cascades, six cube faces or one spot view as tiles)sampled through `shadowMaps[i]` on texture unit 8 + i (`ShadowMapUnit`, set by `Program::link()`); shaders append `ivf::shadow_glsl` (`shadow_shaders.h`) and call `ivfShadowFactor()`. The shadow pass draws every opaque item with `Node::castsShadows()` using the stock program in `shadowPass` mode (`ShaderManager::lockProgram()`), whatever its material.

The library's built-in shaders are embedded as C++ string constants (`stock_shaders.h`, `pbr_shaders.h`, `post_shaders.h`, ...). File-based shaders used by examples live in `data/shaders/` and are copied next to the executables at build time (`copy_example_assets` / `ivf2_copy_runtime_files`) and installed with the library; `ShaderManager::compileProgramFromFiles` (returns `Result<ProgramPtr>`) resolves them relative to the working directory. Example models, textures and sounds are in `examples/assets/`.

## Documentation

- User guide (Zensical): `docs/docs/`, configured by `docs/zensical.toml`
- API reference (Doxygen): `Doxyfile` at repo root; generated output in `docs/docs/doxygen/html/`
- Build docs: `.readthedocs.yaml` (Python 3.11, doxygen, graphviz, zensical required)
