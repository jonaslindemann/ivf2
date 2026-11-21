# ivf2 Roadmap

A plan for making ivf2 a high-quality, widely usable C++ library for interactive 3D
visualisation and creative coding. Based on a review of the code base in November 2026
(~33k lines of library code in `ivf`, `ivfui`, `ivfmath`, `ivfmusic`; 54 examples).

## Where ivf2 stands

**Strengths**

- Broad feature set for its size: scene graph, ~20 primitives, ~97 procedural generators,
  deformers, PBR/bump/water/glow materials, shadows, planar reflections, ~25 post effects,
  particles, flow fields, tweening/keyframe/spline animation, timeline, music-driven scenes,
  ImGui-based inspectors and an editor.
- Approachable API: `Class::create()` factories, `shared_ptr` everywhere, a single
  `GLFWWindow` subclass gets you a running app.
- Good documentation base (Zensical user guide + Doxygen) and many examples.

**Main weaknesses holding back a wider audience**

| Area | Observation |
|------|-------------|
| Build & packaging | Legacy CMake (global include dirs, outputs in the source tree, no install/export) - *fixed in phase 0* |
| Global state | 9 raw-pointer singletons (`ShaderManager`, `LightManager`, `TextureManager`, ...) - one GL context per process, no deterministic GL resource teardown, hard to test |
| Dependency footprint | Public headers include assimp, FreeType, stb, nlohmann/json, GLFW, ImGui. Every consumer compiles against all of them; none can be made optional |
| Bundled third-party code | glm 1.0, gml, glad and dirent are vendored in `include/`. Two math libraries (glm in ivf, gml in generator) |
| Rendering core | Immediate, recursive draw with a global "current program"; no render queue, sorting, culling or uniform buffers; `glGetUniformLocation` per call with no cache |
| Diagnostics | No `KHR_debug` callback, 4 `glGetError` calls in total; 50 `std::cout`/`std::cerr` calls bypass the existing `Logger` |
| Testing / CI | No tests and no build CI (only CodeQL and docs workflows) - *unit test scaffold added in phase 0* |
| Layering | `ivf` depended on `ivfui` for stb symbols - *fixed in phase 0* |
| Large classes | `glfw_scene_window.cpp` (1.3k lines), `placement_mixin.cpp` (1.1k), `glfw_music_scene_window.cpp` (1k) |

## Phase 0 - Modern build system (done)

- `CMakePresets.json`: `vs`, `ninja`, `debug`, `release`, `lib-only` with build and test presets.
- vcpkg bootstrap moved to `cmake/IvfVcpkgBootstrap.cmake`; `vcpkg.json` pinned with a
  `builtin-baseline` (so the Visual Studio bundled vcpkg works too). Optional manifest
  features `examples` (ReactPhysics3D) and `renoise` (miniz, pugixml) follow the CMake options.
  Unused dependencies removed (GLEW, libpng, libjpeg-turbo, zlib, ImGui OpenGL2 binding).
- Target-based CMake: each library created by `ivf2_add_library()`, links its own
  dependencies, has `ivf2::` aliases, PIC, C++20 as a usage requirement. No global
  `include_directories`.
- Build outputs go to `build/<preset>/{bin,lib}/<Config>` (legacy layout available via
  `IVF2_OUTPUT_IN_SOURCE_TREE`).
- Install + export: `find_package(ivf2)`, `ivf2ConfigVersion.cmake`, headers under
  `include/ivf2/`, runtime data under `share/ivf2/`, helper `ivf2_copy_runtime_files()`.
  Works as a subproject (`FetchContent`) with examples/tests off by default.
- Options: `IVF2_BUILD_EXAMPLES`, `IVF2_BUILD_TESTS`, `IVF2_BUILD_MUSIC`, `IVF_WITH_RENOISE`,
  `IVF2_INSTALL`, `IVF2_USE_PCH`, `IVF2_WARNINGS`.
- `tests/` with a dependency-free harness and CTest tests for `generator` and `ivfmath`.
- stb implementation moved from `ivfui` to `ivf` (`src/ivf/stb_impl.cpp`).

## Phase 1 - Foundations and quality (next 1-3 months)

Goal: make the library trustworthy to depend on.

1. **Continuous integration.** GitHub Actions matrix (Windows/MSVC, Ubuntu/GCC+Clang,
   macOS/AppleClang) using the presets, vcpkg binary caching (`VCPKG_BINARY_SOURCES` with
   GitHub Actions cache), `ctest`, and an install + `find_package` consumer test. Add
   `clang-format --dry-run` and later `clang-tidy` checks.
2. **Replace singletons with a context object.** Introduce `ivf::RenderContext` (owned by
   the window) holding the shader, light, texture, font, mesh and transform managers.
   Keep `XxxManager::instance()` as a deprecated forwarder to the active context for one
   release. Benefits: multiple windows/contexts, GL resources destroyed while the context
   is alive, testable code.
3. **Hide third-party headers.** Pimpl or forward declarations in `model_loader.h`
   (assimp), `font_manager.h` (FreeType), `texture.h` (stb), `scene_serializer.h` /
   `mesh_manager.h` (nlohmann/json). Then make dependencies `PRIVATE` and optional:
   `IVF2_WITH_ASSIMP`, `IVF2_WITH_AUDIO` (miniaudio), `IVF2_WITH_MIDI` (RtMidi),
   exposed as `find_package(ivf2 COMPONENTS ...)`. Faster compiles and a much smaller
   minimal install.
4. **One math library.** Port `generator` from gml to glm (or provide glm adapters) and
   take glm from vcpkg instead of vendoring it; remove `include/gml` and `include/dirent`
   (use `std::filesystem`).
5. **Diagnostics.** Debug-context option on `GLFWWindow`, `glDebugMessageCallback` routed
   to `Logger` (when GL 4.3 / `KHR_debug` is available), `glObjectLabel` for named
   resources, and replace all `std::cout`/`std::cerr` with `IVF_LOG_*`. Add an
   `ivf::Logger` sink API so applications can redirect output.
6. **Error-handling policy.** Decide and document: loaders return
   `std::expected`-style results (or `nullptr` + logged error), never silently continue.
   Apply consistently to textures, models, shaders, fonts.
7. **API polish.** Const-correct getters (`material() const`), `std::string_view`/`const&`
   parameters instead of `const std::string` by value, consistent naming
   (`texture(i)` vs `getTexture(i)`), `[[nodiscard]]` on factories. Generate
   `ivf/version.h` from `project(VERSION)`.
8. **Visual regression tests.** Headless rendering (hidden GLFW window, or EGL on CI)
   rendering each example scene to an FBO and comparing with golden images within a
   tolerance. `FrameWriter` already provides most of the plumbing.
9. **Repository hygiene.** Move runtime data from `bin/` to `data/` and example assets to
   `examples/assets/` (consider Git LFS for the 93 MB of assets); remove `test_span.cpp`,
   `include/ivf/logger.cpp`, the diverged root `shaders/` directory and `_example8`.
   Add `CONTRIBUTING.md`, `CHANGELOG.md`, issue templates and semantic versioning with
   tagged releases.

## Phase 2 - Rendering quality and performance (3-6 months)

1. **Render queue.** Separate scene traversal from drawing: collect draw items
   (mesh, material, transform, flags), frustum-cull with the existing bounding boxes, sort
   opaque front-to-back by program/material and transparent back-to-front. Shadow and
   reflection passes reuse the same list instead of re-traversing.
2. **Uniform handling.** Cache uniform locations in `Program`; move camera, lights and
   fog into std140 uniform buffers shared by all programs; automatic instancing of
   identical meshes.
3. **HDR pipeline.** Float render targets, exposure and tone mapping (ACES/AgX) as the
   final post step, MSAA resolve and FXAA/SMAA option.
4. **Image-based lighting.** Load HDR environment maps (`.hdr`/`.exr`), precompute
   irradiance and prefiltered specular maps for the existing PBR material; skybox from
   the same map.
5. **Better shadows.** Cascaded shadow maps for directional lights, PCF/PCSS soft
   shadows, cube-map shadows for point lights.
6. **SSAO** and screen-space reflections as effects.
7. **glTF 2.0** as the primary model format: PBR materials, node hierarchy, skinning and
   morph-target animation (fastgltf or cgltf; keep assimp for legacy formats).
8. **Resource hot reload** generalized from `ShaderWatcher` to textures and models.
9. **CPU picking** via ray casting against a per-mesh BVH, complementing the existing
   color-buffer selection.

## Phase 3 - Features that set ivf2 apart (6-12 months)

ivf2 sits between general creative-coding toolkits (openFrameworks, Cinder, raylib) and
scientific visualisation toolkits (VTK). Leaning into *interactive visualisation* is the
clearest way to stand out.

1. **Scientific visualisation module (`ivfvis`).**
   - Colormaps (viridis, magma, cividis, diverging, user-defined) and color bars.
   - Scalar fields: isosurfaces (marching cubes), slice planes, volume rendering
     (ray marching on 3D textures).
   - Vector fields: glyphs/arrows (instanced), streamlines and stream tubes (reusing
     `FlowField`, `Extrusion`, `PathFrames`).
   - Point clouds with per-point attributes and size attenuation.
   - 3D axes, grids and tick labels (with `TextNode`), 2D plots in ImGui (ImPlot).
   - Readers for CSV, VTK legacy/VTU and NumPy `.npy`.
2. **Python bindings** (nanobind) for the scene graph, primitives, materials and
   windowing; publish wheels. Opens the library to scientists, students and Jupyter users.
3. **Web target.** Emscripten build (GLES 3 / WebGL2) so examples run in the browser -
   the best possible showcase in the documentation.
4. **Headless and offline rendering.** Render without a visible window (EGL/OSMesa),
   video export through an ffmpeg pipe from `FrameWriter`, deterministic time stepping.
   Useful for figure generation, CI and music videos (`ivfmusic`).
5. **Text and 2D overlay.** SDF/MSDF font rendering for crisp scalable text, a 2D
   drawing layer (thick lines, shapes, images) for HUDs and creative coding.
6. **GPU compute.** Compute-shader particles and fields where GL 4.3 is available,
   transform-feedback fallback for macOS.
7. **Physics module.** Wrap the ReactPhysics3D integration from `examples/rigid_body1`
   into an optional `ivfphysics` library (rigid bodies synced to `TransformNode`).
8. **Editor.** Grow `editor1` into a scene editor with gizmos (ImGuizmo), undo/redo, and
   save/load via `SceneSerializer`.

## Phase 4 - Reach and distribution

- Publish a vcpkg port (and optionally Conan recipe) for `ivf2`.
- A GitHub template repository (`ivf2-starter`) with a minimal app, presets and
  `vcpkg.json`, linked from "Your First App".
- Documentation: examples gallery with screenshots generated in CI, a "concepts" page on
  the render loop and resource lifetime, and a migration guide for each breaking change.
- Long term: evaluate a backend abstraction (OpenGL now; Vulkan or WebGPU later) behind
  the render queue introduced in phase 2. Since OpenGL is deprecated on macOS, WebGPU
  (via Dawn or wgpu-native) would cover desktop and web with one backend.

## Suggested order of work

1. CI (phase 1.1) - everything else becomes safer to change.
2. Hide third-party headers + optional dependencies (1.3) - largest effect on usability.
3. Render context (1.2) and diagnostics (1.5).
4. Render queue + uniform buffers (2.1, 2.2) - the base for HDR, IBL, shadows.
5. Scientific visualisation module and Python bindings (3.1, 3.2) - the main differentiators.
