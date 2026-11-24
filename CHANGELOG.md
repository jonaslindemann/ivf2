# Changelog

All notable changes to ivf2 are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and ivf2 uses
[semantic versioning](https://semver.org/) (see [CONTRIBUTING.md](CONTRIBUTING.md#versioning-and-releases)).
While the version is `2.0.0-alpha.*`, the API may still change between pre-releases;
renamed functions keep a `[[deprecated]]` alias for at least one release.

## [Unreleased]

### Changed (may require changes in your code)

- Meshes are uploaded to the GPU when first drawn instead of in `end()`: creating and changing
  geometry (primitives, `MeshNode`, `Mesh::updateVertices()`/`updateNormals()`) needs no OpenGL
  context, so scenes can be built before the window exists. `Mesh::vertexArray()` uploads on
  demand and returns the vertex array of the current context.
- Mesh vertex attributes use the fixed locations bound by `ShaderManager` (`aPos` 0, `aColor` 1,
  `aTex` 2, `aNormal` 3) instead of querying the program that was current at creation.
- Selection (`setSelectionEnabled(true)`) is rendered only after mouse movement, button presses,
  resizing or camera changes instead of every frame, so `onOverNode()` is no longer called every
  frame while the cursor rests on a node. Clicks pick the node under the cursor afresh.
- `GLFWSceneWindow` traverses the scene once per frame; the shadow, reflection, main and selection
  passes draw the same items (`LightManager::renderShadowMaps(const RenderQueue &)`,
  `PlanarReflection::render(const RenderQueue &)`).
- Scenes are drawn through a render queue: `Node::draw()` collects draw items
  (`Node::collect()`, `RenderQueue`) and draws them with a `Renderer`. Nodes keep their
  `doPreDraw()`/`doDraw()`/`doPostDraw()` hooks.
- Opaque objects are drawn sorted by shader program and material, then front to back (fewer
  state changes); `RenderQueue::sortForView()`.
- Nodes without any material (own or inherited) are drawn with a neutral default material instead
  of the material of the previously drawn node. `LineTrace` has its own unlit material.
- Material inheritance follows the scene: a node without a material uses its nearest ancestor's.
  Before, it used whatever material was applied last (often a previous sibling's).
- `CompositeNode` uses a material set on it (`setMaterial()` on a group used to be ignored
  because groups had `setUseMaterial(false)` by default).

### Added

- Transparency: materials with alpha < 1 are drawn after all opaque objects, back to front, with
  alpha blending (`setAlpha()` had no visible effect before). Blended materials (`GlowMaterial`,
  `MistMaterial`, ...), particles and billboards are ordered the same way, so blended nodes no
  longer need to be added last. `Node::setRenderLayer()` chooses a layer explicitly
  (`RenderLayer::Background`, `Opaque`, `Transparent`, `Overlay`). New example `transparency1`.
  Transparent meshes are drawn in two passes (back faces, then front faces), so their own
  triangles blend in the right order; `Mesh::clockwise()` tells which winding faces outwards.
- Frustum culling: `GLFWSceneWindow` skips items outside the camera (main and selection passes),
  the light (shadow maps) or the mirrored camera (reflections). `ivf::Frustum`,
  `Renderer::execute(queue, frustum)` and `Renderer::stats()` are public.
- A node can be shown in several windows: each OpenGL context gets its own GPU copy of a mesh
  (`multi_window1` shares a floor between two windows).
- `RenderContext::lifetime()` and `RenderContext::deleteLater()` for objects that hold GL
  resources in several contexts.
- The renderer draws into an explicit render context: `Renderer(RenderContext &, RenderPass)`,
  `LightManager::renderShadowMaps(context, queue)`, `PlanarReflection::render(context, queue)`,
  `Node::draw(context)`, `Mesh::draw(context)`. Nodes can override `Node::doDrawPass(const
  DrawContext &)` to see the pass (`RenderPass::Main`, `Shadow`, `Reflection`, `Selection`),
  the render context and the camera; its default calls `doDraw()`.
- Render statistics: `Renderer::stats()` also counts draw calls, shader program changes and
  material applications; `GLFWSceneWindow::renderStats()` reports them per pass, the
  *Windows > Render Statistics* window shows them, and `IVF2_RENDER_STATS=n` logs them every n
  frames. `RenderContext::counters()` gives the running totals. New user guide page
  *Render Queue*.

### Fixed

- Bounding boxes of `Box`, `RoundedBox` and `Plane` were half their real size (and those of the
  other generator primitives were approximations): primitives now use the box computed from their
  vertices. Code using scene extents (camera framing, `Extents`) sees the correct size.
- `MeshNode::updateVertices()` updates the bounding box.
- Selection on high-DPI displays (macOS Retina): the selection buffer and its viewport now use window
  coordinates like the mouse position, so hovering and clicking pick the object under the cursor.
- Hovering only reports a node while the cursor is over the window; outside it (or in hidden
  windows) the selection buffer was read at undefined pixels.
- Automatic shadow framing (`LightManager::setAutoCalcBBox(true)`, the default) used an empty,
  infinitely large scene box; it now fits the shadow maps to the scene's bounding boxes, and the
  shadow and main passes use the same light matrix.
- With selection enabled, every frame rendered the shadow maps twice and called `onDraw()` twice;
  the selection pass now draws the frame's collected items only.
- Line widths above 1 (`Mesh::setLineWidth`) now work on Windows and Linux: windows no longer
  request a forward-compatible OpenGL context there (macOS still needs one and keeps
  1-pixel lines). Scenes with wide lines look different from before.

## [2.0.0-alpha.2] - 2026-11-22

### Changed (may require changes in your code)

- C++23 is required (MSVC 2022, GCC 13, Clang 17, Xcode 16 or newer).
- Loaders and shader compilation report failures as `ivf::Result<T>`
  (`std::expected<T, ivf::Error>`): `ShaderManager::compileProgramFromFiles`/`FromStrings`,
  `load*Shader()`, texture, font and model loading. The old functions remain as `[[deprecated]]`
  wrappers. Results are `[[nodiscard]]`.
- GL state lives in a per-context `ivf::RenderContext` (`renderContext()->lights()`, ...).
  `XxxManager::instance()` still works and forwards to the current context.
- glm is the only math library (also in the generator); the bundled glm and dirent are gone.
- Getters are named after the property: `getTexture(i)` -> `texture(i)`, `getAmplitude()` ->
  `amplitude()`, `getProperties()` -> `properties()` and 18 more, with deprecated aliases.
  101 getters are now `const`, `create()` factories are `[[nodiscard]]`, and string parameters are
  `const std::string &`.
- Public headers no longer include assimp, FreeType, miniaudio, RtMidi, stb, nlohmann-json or ImGui
  (ivf core); stb is available through `ivf/image_io.h`.
- Repository layout: library runtime data moved from `bin/` to `data/` (`shaders/`, `fonts/`),
  example assets to `examples/assets/`. Build output goes to `build/<preset>/bin/` and `lib/`, with a
  `<Config>` level only for multi-config generators such as Visual Studio
  (`IVF2_OUTPUT_IN_SOURCE_TREE=ON` puts them in the source tree).

### Added

- Modern CMake build: presets, static targets exported as `ivf2::<name>`, install rules and
  `find_package(ivf2)`, vcpkg auto-detection, precompiled headers, `ivf2_copy_runtime_files()`.
- Optional dependencies `IVF2_WITH_ASSIMP`, `IVF2_WITH_FREETYPE`, `IVF2_WITH_AUDIO`,
  `IVF2_WITH_MIDI` (default ON) with `IVF2_HAS_*` macros in the generated `<ivf/config.h>`.
- CI on Windows, Linux and macOS with warnings as errors, unit tests, a package consumer build,
  a minimal (all options off) build, CodeQL and visual regression tests (`IVF2_VISUAL_TESTS`).
- Diagnostics: OpenGL debug output routed to the Logger, `ivf::glLabel()`, logger sinks, compact
  log format with wrapping.
- Unattended runs: `IVF2_HIDE_WINDOWS`, `IVF2_EXIT_AFTER_FRAMES`, `IVF2_SCREENSHOT`,
  `IVF2_SCREENSHOT_UI`, `IVF2_FIXED_TIMESTEP`, `IVF2_RANDOM_SEED` (`ivf::automaticSeed()`).
- `ivfmusic`: song-driven scenes (`GLFWMusicSceneWindow`, `Automation`, Renoise `.xrns` scores,
  MIDI `LiveControls`, `AutomationRecorder`), blend materials (`GlowMaterial`, `MistMaterial`,
  `GradientSkyMaterial`), `PathCameraManipulator`.
- Effects framework and post-processing effects (bloom, mip bloom, fog, depth of field, ...),
  planar reflections, procedural and GPU-generated textures, multitexturing, bump mapping,
  frame export, particle systems, flow fields, tweens and timers.
- `SECURITY.md`, `CONTRIBUTING.md`, this changelog, issue and pull request templates.

### Fixed

- Crash on exit on Linux: windows and their scenes are now destroyed before `glfwTerminate()`.
- High-DPI displays (macOS Retina): the camera manipulator mixed window and framebuffer sizes,
  so the mouse ray for the 3D cursor was wrong and the projection was recomputed every frame
  while a mouse button was held.
- Bugs found by enabling `/W4` and `-Wall -Wextra`: `Capsule` ignored its constructor
  arguments, keyframe tension was ignored, `GridLayout` setters were stubs,
  `PolyLinePath::setSegments` did nothing, `TransformManager::rotateToVector` used the wrong axis,
  click callbacks in `GLFWSceneWindow` never fired, missing virtual destructors.
- A window whose `onSetup()` fails is closed instead of crashing in `onDraw()`.

### Removed

- Unused files: `test_span.cpp`, `include/ivf/logger.cpp`, the outdated root `shaders/`
  directory, `examples/_example8`, the embedded font `ivf_font.cpp` and the `python/` scripts.

### Security

- Vulnerabilities are reported privately through GitHub (see [SECURITY.md](SECURITY.md)).

## [2.0.0-alpha] - 2025-06-22

First public pre-release of ivf2: scene graph, primitives, materials and lighting, textures,
text rendering, model loading with assimp, keyframe animation, procedural geometry (generator),
ImGui-based user interface and around 20 examples.

[Unreleased]: https://github.com/jonaslindemann/ivf2/compare/v2.0.0-alpha.2...HEAD
[2.0.0-alpha.2]: https://github.com/jonaslindemann/ivf2/compare/2.0.0-alpha...v2.0.0-alpha.2
[2.0.0-alpha]: https://github.com/jonaslindemann/ivf2/releases/tag/2.0.0-alpha
