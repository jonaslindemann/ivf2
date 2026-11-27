# ivf2 Roadmap

A plan for making ivf2 a high-quality, widely usable C++ library for interactive 3D
visualisation and creative coding. Based on a review of the code base in November 2026
(~33k lines of library code in `ivf`, `ivfui`, `ivfmath`, `ivfmusic`; 54 examples).

## Status (26 November 2026)

Latest release: **2.0.0-alpha.3** (26 November 2026): phases 2 and 3.1 (see `CHANGELOG.md`).

| Phase | State |
|-------|-------|
| 0 - Build system | Done |
| 1 - Foundations | Done: CI, render context (step A), private third-party headers and optional dependencies, glm only, diagnostics, `Result<T>` error handling, warnings and API polish, visual regression tests, repository hygiene. Open items listed below |
| 2 - Rendering | **2.1 render queue done** (steps 1-7). **2.2 uniform handling done** (location cache, camera and lights uniform blocks, automatic instancing; one item per mesh for `MeshNode`). **2.3 HDR pipeline done** (exposure, tone mapping, gamma, FXAA, MSAA control; SMAA open). **2.4 image-based lighting done** (`.exr` open). **2.5 better shadows done**. **2.6 SSAO and screen-space reflections done**. **2.7 glTF 2.0 done** (GPU skinning, glTF lights and cameras open). **2.8 resource hot reload done**. **2.9 CPU picking done**. Phase 2 complete|
| 3 - Differentiators | **3.1 `ivfvis` done** (data model, colormaps, colour bars, point clouds, isosurfaces, slices, glyphs, streamlines, volume rendering, axes, readers, 2D plots). **3.3 web target done** (preset `wasm`, GLES compatibility, 67 examples live in the browser at <https://jonaslindemann.github.io/ivf2/web/>, CI smoke test in headless Chrome, guide "Running in the Browser"). **3.7 physics module in progress** (`ivfphysics`: step 1, world, bodies and shapes, done; steps 2, queries, events and interaction, 3, joints and compound bodies, and 4, the sandbox, done; next step 5, documentation). 3.2 Python bindings later |
| 4 - Reach | Not started |

**Open items from finished work**

- Render queue (2.1): materials applied with the renderer's explicit context (completes
  step B; meshes already are); `setMaterialOverride()` (decided, opt-in).
- Render context (1.2): `ivf::Key`/`ivf::MouseButton`; drop the `instance()` forwarders after
  a deprecation period.
- Dependencies (1.3): ImGui out of the core (`DebugDraw::drawUi()` labels).
- Error handling (1.6): `GpuProceduralTexture`, `ShaderWatcher` and `Effect::load()` to
  `Result`; remove deprecated wrappers in the next major release.
- API (1.7): rename virtual `get*` functions in the next major release.
- CI and tests (1.1, 1.8): Ubuntu/Clang, sanitizer, `clang-format` and `clang-tidy` jobs;
  EGL instead of Xvfb; Windows/macOS visual references; Git LFS for assets (1.9).

## Where ivf2 stands

*The review this roadmap started from; rows marked fixed have been addressed since.*

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
| Global state | 9 raw-pointer singletons (`ShaderManager`, `LightManager`, `TextureManager`, ...) - one GL context per process, managers never dropped (GL objects leak or are freed after the context is gone), hard to test - *fixed: `RenderContext` (1.2)* |
| Host coupling | `ivf` makes no GLFW calls, but context bootstrap does: the GL loader runs only in `GLFWWindow`, default shaders/font/light are set up in `GLFWSceneWindow::doSetup`, and `XxxNode::create()` uploads to the GPU immediately, so nodes need a current context. Public input APIs use GLFW key/button codes - *fixed except input codes (1.2, 2.1)* |
| Dependency footprint | Public headers include assimp, FreeType, stb, nlohmann/json, GLFW, ImGui. Every consumer compiles against all of them; none can be made optional - *fixed (1.3)* |
| Bundled third-party code | glm 1.0, gml, glad and dirent are vendored in `include/`. Two math libraries (glm in ivf, gml in generator) - *fixed: only glad/KHR remain, glm from vcpkg* |
| Rendering core | Immediate, recursive draw with a global "current program"; no render queue, sorting, culling or uniform buffers; `glGetUniformLocation` per call with no cache - *render queue, sorting and culling done (2.1); uniforms next (2.2)* |
| Diagnostics | No `KHR_debug` callback, 4 `glGetError` calls in total; 50 `std::cout`/`std::cerr` calls bypass the existing `Logger` - *fixed (1.5)* |
| Testing / CI | No tests and no build CI (only CodeQL and docs workflows) - *fixed: CI on three platforms, unit and visual tests (1.1, 1.8)* |
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
- stb implementation moved from `ivfui` to `ivf`, later made private behind `ivf/image_io.h`.

## Phase 1 - Foundations and quality (next 1-3 months)

Goal: make the library trustworthy to depend on.

1. **Continuous integration.** *In place:* `.github/workflows/build.yml` builds and tests on
   Windows/MSVC, Ubuntu/GCC and macOS/AppleClang using the presets, caches vcpkg binaries,
   runs `ctest`, installs and builds `tests/package` against the installed package. CodeQL
   updated to v3 (`build-mode: none`); Dependabot keeps actions current.
   *Cost split:* a push to `main` builds Linux/GCC only; pull requests, manual runs, a nightly
   run (skipped without new commits) and releases (`release.yml` creates the release only if
   it passes) get the full matrix with visual tests; ccache on Linux/macOS; CodeQL weekly;
   docs only when their sources change.
   *Next:* Ubuntu/Clang job, a sanitizer (ASan/UBSan) job, `clang-format` check on changed
   lines, `clang-tidy`.
2. **Render context, independent of the windowing system.** *Step A done:*
   `ivf::RenderContext` owns the nine GL-bound managers and the fallback textures;
   `GLFWWindow` creates one per window and switches GL, RenderContext and ImGui contexts
   together (also before input callbacks); `setupDefaults()` replaces the setup in
   `GLFWSceneWindow::doSetup`; examples use `renderContext()->...`; new `multi_window1`,
   `embedding1` and `test_render_context`. Finding during implementation:
   `TransformManager` caches uniform locations of the context's program, so it is
   context-bound and lives in the context (the original plan kept it out). *Step B done
   for the renderer and passes* (2.1 step 6); materials and node hooks still use the current
   context. Still open: `ivf::Key`/`ivf::MouseButton` and dropping the `instance()`
   forwarders after a deprecation period. Original plan: replace the GL-bound
   singletons with `ivf::RenderContext`, defined in the core `ivf` library with no
   knowledge of windows. It represents *one OpenGL context and the resources that belong
   to it*; whatever owns a GL context creates one. `ivfui::GLFWWindow` becomes one host
   among several possible (Qt `QOpenGLWidget`, SDL, headless EGL, Emscripten, Python).

   ```
   host (GLFW | Qt | SDL | EGL | web)  -- makes its GL context current, then
       auto ctx = ivf::RenderContext::create(getProcAddress);  // loads GL functions
       ctx->setupDefaults();          // default shaders, font, light (from GLFWSceneWindow::doSetup)
       ...                            // render; destroy ctx before the GL context
   ```

   - **Contents:** only GL-bound state - shaders/programs, textures, meshes, font glyph
     textures, lights (shader uniforms), selection framebuffers, `DebugDraw` (own VAO/VBO/
     program), `ShaderWatcher` (reloads programs) and `TransformManager` (uniform
     locations). Not context-bound and kept out: `Logger`, `TimeController`, tweens,
     `AudioManager`, `MidiController`. Per-context matters because
     VAOs and framebuffers can never be shared between GL contexts.
   - **GL loading** moves from `GLFWWindow` into `RenderContext::create(loaderFn)`
     (`glfwGetProcAddress`, `QOpenGLContext::getProcAddress`, `eglGetProcAddress`, ...).
   - **Default setup** moves from `GLFWSceneWindow::doSetup` to
     `RenderContext::setupDefaults()` so every host starts from the same state.
   - **Lifetime:** the host destroys the context while its GL context is still current,
     so all GL objects are released deterministically.
   - **Finding the context - step A (this phase):** a thread-local *current* context set by
     the host while rendering, mirroring GL's own model. `XxxManager::instance()` becomes a
     deprecated forwarder to it, so existing user code and examples keep working.
     Step B (explicit `RenderContext&` passed through drawing) comes with the phase 2
     render queue.
   - **Input codes:** introduce `ivf::Key` / `ivf::MouseButton` (values identical to GLFW's,
     so existing code keeps compiling); hosts translate their native codes.
   - **Benefits:** multiple windows/contexts, deterministic GL teardown, ivf usable in other
     applications and toolkits, headless rendering for tests and CI (1.8), and the basis for
     the Python, web and offline targets in phase 3.
   - **Impact on examples** (survey of the 53 examples): none *need* to change - step A keeps
     `XxxManager::instance()`/`create()` working because both window classes create and make
     current their context before calling `onSetup`/`onDraw`/`onUpdateUi`.
     - 42 `GLFWSceneWindow` examples: migrate the GL-bound manager calls
       (29 examples, ~40 lines, e.g. `LightManager::instance()` -> `renderContext()->lights()`)
       in the same change, so examples show the new API and don't trigger the deprecation.
     - 10 `GLFWWindow` examples (`hello_ivf`, `extrusions1`, `textures1-3`, `ui1/2`, ...) keep
       their manual setup as the low-level path; one becomes an embedding walkthrough
       (`RenderContext::create()` + `setupDefaults()`), the pattern for Qt/SDL hosts.
     - Unaffected: GLFW key/mouse codes (110 uses in 20 examples - `GLFWWindow::onKey(int ...)`
       keeps its signature; `ivf::Key` only in core APIs, same values), the non-GL globals
       (`TimeController`, `TweenManager`, `TimerManager`, `AudioManager`), `glfwGetTime()`.
     - No example subclasses scene nodes or creates nodes before `onSetup` (the seven
       `doDraw()` overrides are ImGui panels), so step B and lazy upload (2.1) don't affect them.
     - New: a two-window example (one scene per window) as the acceptance test - impossible
       with the current singletons.
3. **Hide third-party headers.** *Done for the optional libraries:* assimp, FreeType,
   miniaudio, RtMidi and stb are private to `ivf` (no public header includes them) and the
   first four are optional via `IVF2_WITH_ASSIMP/FREETYPE/AUDIO/MIDI` (default ON), with
   matching vcpkg features, a generated `ivf/config.h` and `find_package(ivf2 COMPONENTS ...)`.
   nlohmann/json and ImGui are now private to `ivf` too (`SceneSerializer` helpers moved
   into the .cpp, `MeshManager` state no longer stored as JSON; `ivfmusic` links json
   privately). A build-only check (`tests/header_check.cmake`) compiles every public header
   of ivfmath/ivf/ivfui/ivfmusic on its own with only public dependencies, so leaks and
   non-self-contained headers fail CI.
   *Remaining:* ImGui is still a (private) dependency of the core for
   `DebugDraw::drawUi()` text labels - could move to ivfui; gml in public headers (see 4).
   GLFW and ImGui in ivfui headers are intended - they are part of that API.
4. **One math library.** *Done:* the generator library's built-in `GENERATOR_USE_GLM` mode
   is always on, so `gml::` names are aliases for glm; the bundled `include/gml` is removed.
   Verified by dumping 35 generator meshes/paths/shapes with the old (gml) and new (glm)
   build: identical structure, max difference 1e-12. Fixed on the way: `gml::length` alias
   (PolyLinePath) and a comparator for `SubdivideMesh` (glm vectors have no `operator<`).
   glm now comes from vcpkg (`glm::glm`, 1.0.3) instead of the bundled 1.0.0 copy (generator
   output re-verified: identical); `include/dirent` removed - the only user, ImGuiFileDialog in
   `assimp1`, uses `std::filesystem` (`USE_STD_FILESYSTEM`).
5. **Diagnostics.** *Done:* `ivf/gl_debug.h` routes OpenGL debug output (4.3) to the Logger
   (severity mapping, repeat suppression, synchronous for usable stacks); `GLFWWindow`
   requests the newest OpenGL (4.6 -> 4.3 -> 4.1 on macOS) and a debug context in debug
   builds or with `IVF2_GL_DEBUG=1`; GLFW errors are logged; programs, textures and
   framebuffers are labelled; the context's GL version/renderer is logged. Logger is
   thread-safe and has sinks (`addSink`); status/warning `std::cout` output in the libraries
   goes through the Logger (explicit `print()`/dump helpers still write to stdout).
   Automation env vars `IVF2_HIDE_WINDOWS`, `IVF2_EXIT_AFTER_FRAMES`, `IVF2_SCREENSHOT`.
   Running all 52 examples with debug output found and fixed three silent GL errors:
   selection buffer resized without being created (`GL_INVALID_OPERATION`, 40 examples),
   `glLineWidth` > 1 in a forward-compatible context (`GL_INVALID_VALUE`, 7 examples -
   wide lines never worked), `glViewport` with the camera's unset -1 size.
   *Decided:* `GLFW_OPENGL_FORWARD_COMPAT` is only requested on macOS (required there for
   core profiles), so line widths > 1 take effect on Windows and Linux; macOS stays at
   1 pixel (`RenderContext::wideLinesSupported()`).
6. **Error-handling policy.** *Done (policy + loaders):* C++23; `ivf::Result<T>` =
   `std::expected<T, ivf::Error>` (code + message) in `ivf/result.h`; `makeError()` logs and
   returns. Converted: `Texture::fromFile/load/loadHDR`, `Cubemap::load/loadFromDirectory`,
   `FontManager::loadFace`, `ModelLoader::load` (old throwing `loadModel` deprecated),
   `SceneSerializer::loadFile/saveFile` (old `load/save` deprecated). Exceptions only for
   programming errors. Docs: Advanced > Error Handling; tests: `test_result`.
   *Done (shaders):* `ShaderManager::compileProgramFromFiles/FromStrings` and the built-in
   `load*Shader()` return `Result<ProgramPtr>` with the driver's info log in the message
   (`Shader::infoLog()`, `Program::infoLog()`); `loadProgramFrom*` and the global
   `compileLinkErrors()` flag are deprecated. Effects use `Effect::compileProgram()`
   (nullptr on failure, skipped by `addEffect()`); materials and `setupDefaults()` check results.
   *Remaining:* `GpuProceduralTexture` and `ShaderWatcher` still use the low-level
   compile/link calls; `Effect::load()` could return a Result; remove deprecated wrappers
   in the next major release.
7. **API polish.** *Done (warnings):* `IVF2_WARNINGS` is on by default for top-level builds
   (`/W4`, `-Wall -Wextra -Wpedantic`, ivf2's own targets only), plus `IVF2_WARNINGS_AS_ERRORS`.
   MSVC builds everything warning-free at `/W4 /WX`. The cleanup found real bugs: `Capsule`
   ignored its constructor arguments, `KeyframeData` ignored `tension`, `GridLayout` setters and
   getters were empty stubs, `PolyLinePath::setSegments` did nothing (and copies read the
   original's points), `rotateToVector` mixed degrees and radians and skipped vectors with a
   negative component, node click callbacks never fired (a local shadowed
   `GLFWSceneWindow::m_currentNode`), `clearError()` cleared only one error flag.
   Version macros are generated in `ivf/config.h` (`IVF2_VERSION_*`).
   GCC and Clang are warning-free too (initializer order, signed/unsigned, unused fields; missing
   virtual destructors in `PropertyInspectable`, `UiMenuItem`, `UiMenu`, `NodeVisitor`, and
   property pointers that dangled after copying/moving a `PropertyInspectable`), and CI builds
   with `IVF2_WARNINGS_AS_ERRORS=ON` on all platforms.
   *Done (API):* `[[nodiscard]]` on 173 `create()` factories; 78 `const std::string` by-value
   parameters are `const std::string &`; 101 simple getters are `const`; 21 public `get*`
   methods renamed (`getTexture(i)` -> `texture(i)`, `getAmplitude()` -> `amplitude()`, ...)
   with `[[deprecated]]` aliases. Kept on purpose: functions returning through reference
   parameters (`getColor(r, g, b, a)`), lazily-creating accessors (non-const).
   *Next major:* virtual `get*` functions (`Deformer::getOutput`, `ProceduralTexture::getPixel`,
   `getEffect`/`getEffectCount`) - renaming them would silently break user overrides; remove the
   deprecated aliases.
8. **Visual regression tests.** Headless rendering (hidden GLFW window, or EGL on CI)
   rendering each example scene to an FBO and comparing with golden images within a
   tolerance.
   *Done:* `IVF2_VISUAL_TESTS` (`ctest -L visual`, `tests/visual/`) renders 52 examples hidden
   with `IVF2_FIXED_TIMESTEP` and `IVF2_RANDOM_SEED` and compares the last frame with
   references (`image_compare`: per-pixel threshold, fraction of differing pixels, diff image).
   The Linux CI job runs them with Mesa llvmpipe under Xvfb and uploads the rendered images.
   *Later:* EGL/headless contexts instead of Xvfb; Windows/macOS references; music_scene1 via
   its `--export` mode.
9. **Repository hygiene.** Move runtime data from `bin/` to `data/` and example assets to
   `examples/assets/` (consider Git LFS for the 93 MB of assets); remove `test_span.cpp`,
   `include/ivf/logger.cpp`, the diverged root `shaders/` directory and `_example8`.
   Add `CONTRIBUTING.md`, `CHANGELOG.md`, issue templates and semantic versioning with
   tagged releases.
   *Done:* `data/` (`shaders/`, `fonts/`) and `examples/assets/`, `bin/` is build output only;
   obsolete files removed; `CONTRIBUTING.md`, `CHANGELOG.md`, issue and PR templates; the version
   comes from `CMakeLists.txt` (`IVF2_VERSION_PRERELEASE`, `<ivf/version.h>` generated from it)
   and a `v*` tag creates the GitHub release from the changelog (`release.yml`).
   *Later:* Git LFS for `examples/assets/` and `tests/visual/reference/`.

## Phase 2 - Rendering quality and performance (3-6 months)

1. **Render queue.** *Done (steps 1-7 below; open items in Status).* Separate scene
   traversal from drawing: one traversal per frame collects a flat list of draw items, which
   every pass (shadows, reflections, main, selection) then culls, sorts and draws. Design:

   ```
   frame:  scene --collect()--> RenderQueue [DrawItem, ...]
           Renderer(ctx): shadow pass per light | reflection pass | main pass | selection pass
                          each pass: filter -> frustum-cull -> sort -> draw
   ```

   - **Today:** `Node::draw()` applies material and textures in `doPreDraw()` and draws
     while traversing. A parent's material stays applied while its children draw
     (inheritance through leftover GL state). `TransformNode` uses a matrix stack that
     uploads the model matrix on every `translate`/`rotate`/`pop`. Each shadow light and
     each planar reflection does its own `scene->draw()`, and with selection enabled
     `doDrawComplete()` repeats the whole `doDraw()` every frame, shadow maps included.
     No culling, insertion order only (blended nodes must be added last by hand),
     `Material::setAlpha()` has no visible effect (stock materials never enable `GL_BLEND`),
     and constructors/setters upload geometry immediately (`Cube::Cube()` -> `doSetup()`).
   - **DrawItem:** geometry (`Mesh`), world matrix (accumulated during traversal, so
     `InstanceNode` works - world matrices cannot come from `parent()`), effective
     material (inheritance resolved), textures, object id, world bounding box, flags
     (casts shadows, transparent/blend mode, selectable, layer) and a 64-bit sort key.
   - **Collect:** new virtual `Node::collect(collector, state)`. `TransformNode` multiplies
     its matrix into the state, `CompositeNode` recurses, `Node` passes its material down
     explicitly, `MeshNode` emits one item per mesh.
   - **Passes:** shadow map per light (casters only, light frustum), planar reflection
     (all but the mirror, mirrored camera), main (camera frustum), selection (selectable
     items, object ids). Culling tests world bounding boxes against the pass frustum;
     items without a valid box are never culled.
   - **Sorting:** buckets background (skybox) -> opaque (by program, material, then
     front-to-back) -> transparent (`BlendShaderMaterial`, alpha < 1, particles,
     billboards; back-to-front) -> overlay. Program and material change only when they
     differ from the previous item; the model matrix is set once per item.
   - **RenderContext step B:** the renderer and the functions it calls
     (`Material::apply`, `Mesh::draw`) take a `RenderContext&`; `XxxManager::instance()`
     remains for user code.
   - **Lazy GPU upload:** `Mesh` keeps CPU data, `setup()` and shape setters only mark it
     dirty, buffers are created on first draw. Scenes can then be built without a current
     GL context (before the window exists, in GL-free tests, on worker threads).
   - **Compatibility:** `Node::draw()` stays (collect into a temporary queue and execute),
     so examples calling `m_scene->draw()` keep working. Nodes with their own drawing
     (`TextNode`, `Billboard`, `Skybox`, `ParticleSystem`, `InstancedMeshNode`,
     `DeformableMeshNode`, user `doDraw()` overrides) emit a *callback item*: the renderer
     sets matrix and material and calls their `doDraw()`; library nodes get real
     `collect()` implementations over time. Behaviour changes: sorted draw order, no GL
     state carried between nodes, GPU objects not available directly after `create()`.
   - **Extension model:** ivf2 stays a library, not a closed engine. Nodes remain plain
     C++ subclasses with virtual hooks; an extension picks the lowest level that fits:
     1. *Geometry node* - subclass `MeshNode`, fill the mesh in `doSetup()`, set a local
        bounding box (as `Cube` does today; unchanged). Shadows, culling, selection and
        sorting come for free; `mesh()->end()` only marks the mesh dirty, so creating the
        node needs no GL context and calling `refresh()` every frame (animation,
        deformers) costs one upload before the next draw.
     2. *Own GL drawing* - subclass `TransformNode` and override `doDraw()` (as `TextNode`,
        `Skybox`, `ParticleSystem` today; unchanged). The renderer wraps it in a callback
        item: sets the world matrix, applies material and textures, calls `doDraw()`,
        restores state. A bounding box enables culling; `setTransparent()` and
        `setCastsShadows()` choose bucket and passes. Nodes whose drawing depends on the
        pass override `doDrawPass(const DrawContext &)` (pass, `RenderContext`, item,
        view/projection); its default calls `doDraw()`. (Named `doDrawPass` rather than a
        `doDraw` overload: overloading a virtual hides it in every subclass that overrides
        `doDraw()`, which `-Woverloaded-virtual` reports.)
     3. *Own queue items* - override `collect(RenderQueue &, const CollectState &)` to
        emit several items or decide per frame (one item per particle emitter, LOD,
        chunked point clouds). Items carry a mesh or a draw function.

     Beyond nodes: appearance through `ShaderMaterial` (no subclass needed; sorting by
     program makes custom shaders cheap), custom passes and queue inspection through the
     public `Renderer`, and hosts can call collect/execute directly (`embedding1`).
     `scene->draw()` and plain OpenGL in `onDraw()` keep working.
   - **Design rules** (to keep it open):
     1. Nodes are plain subclasses: no registration, no type ids, no `final` classes.
     2. `DrawItem`, `RenderQueue`, `CollectState`, `DrawContext` and `Renderer` are
        public, documented API, not internals.
     3. Built-in nodes use only these public hooks; a library node needing a private
        shortcut means a public hook is missing.
     4. Overriding `doDraw()` is supported permanently (callback items are not deprecated).
     5. The renderer has no hidden global state; everything goes through the
        `RenderContext` it is given.
   - **Steps** (each its own commit, CI and visual tests green):
     1. Lazy GPU upload in `Mesh`. *Done:* CPU data until first draw, one GPU copy per
        `RenderContext` (version counters for full re-upload vs. vertex/normal updates),
        `RenderContext::lifetime()`/`deleteLater()` for freeing copies of other contexts,
        fixed attribute locations; `test_scene_without_gl` and `multi_window1` (floor
        built before any window, shared by both) cover it. Textures, fonts and shader
        materials still need a context when created.
     2. `DrawItem`, `RenderQueue`, `collect()`, main pass only, no culling or sorting -
        pixel-identical to today.
        *Done:* `Node::collect(RenderQueue &, const CollectState &)` (`render_queue.h`) and
        `Renderer::execute()` (`renderer.h`); `Node::draw()` collects and executes, so all
        passes and `scene->draw()` callers use the queue. Containers (`CompositeNode`,
        `InstanceNode`) add no items and pass transform, material and textures down;
        world matrices use the same operations as the matrix stack (bit-identical). Items
        call the nodes' existing `doPreDraw()`/`doDraw()`/`doPostDraw()`, so every node
        type works unchanged; per-mesh items for `MeshNode` and the default material come
        with sorting (step 5). Material inheritance is explicit (siblings no longer leak),
        and `CompositeNode` uses its material by default (it was ignored before). All 51
        visual tests are pixel-identical; `test_render_queue` covers collection.
     3. Shadow, reflection and selection passes from the queue (removes the extra
        traversals; fixes the shadow scene box, whose `ExtentVisitor` result is unused).
        *Done:* `GLFWSceneWindow` collects once per frame; shadow maps
        (`LightManager::renderShadowMaps(const RenderQueue &)`), planar reflections
        (`PlanarReflection::render(const RenderQueue &)`, mirror surfaces left out with a
        `Renderer` skip filter) and the main pass draw the same items; the selection pass
        draws them too instead of re-running `doDraw()` (which re-rendered the shadow maps
        and called `onDraw()` twice). Items carry world-space `bounds`; automatic shadow
        framing uses their union, computed before `apply()` so the shadow and main passes
        use the same light matrix (it was an empty, infinite box before). Images and
        selection buffers pixel-identical. Selection is rendered on demand (mouse movement,
        buttons, resize, camera change); a button press picks the node under the cursor
        afresh, so clicks never reach a node deleted since the last hover update.
     4. Frustum culling. *Done:* `Frustum` (`frustum.h`, planes from a view-projection
        matrix, conservative box test) and `Renderer::execute(queue, frustum)` with
        `stats()`; main and selection passes cull with the camera, shadow maps with the
        light matrix, reflections with the mirrored camera; items without bounds are
        never culled and `Node::draw()` does not cull. Culling exposed wrong bounding
        boxes: `Box`, `RoundedBox` and `Plane` set half-size boxes by hand after
        `createFromGenerator()`, which already computes exact ones - the hand-written
        boxes are gone and `test_render_queue` checks every primitive's box against its
        vertices; `MeshNode::updateVertices()` also updates the box. scene_ui1 draws
        53 of 501 items, transforms1 213 of 251.
     5. Sorting and transparency buckets (intended image changes: update references).
        *Done:* `RenderLayer` (`render_layer.h`: Background, Opaque, Transparent, Overlay)
        from `Node::setRenderLayer()` or the effective material (`Material::renderLayer()`:
        alpha < 1 -> Transparent; `BlendShaderMaterial` Transparent/Background; particles
        and billboards Transparent, skybox Background). `RenderQueue::sortForView()`:
        stable, opaque by program and material (numbered in scene order, deterministic)
        then front to back, transparent back to front. Colour passes blend the
        Transparent layer without depth writes (`Renderer::setTransparencyBlending()`);
        the lit stock shader outputs the material alpha; transparent mesh nodes are drawn
        back faces first, then front faces (each `Mesh` detects its winding from its normals:
        generator meshes are clockwise after the Y/Z swap). Nodes with no material at all get
        a neutral default material; `LineTrace` got its own unlit material (it relied on
        leftover state). New example `transparency1`. Changed images: mip_bloom1 (blend
        order), animation2 (trace colour). Still open: PBR alpha.
        *Done later:* one item per mesh for `MeshNode`. `DrawItem::mesh` names the mesh;
        `MeshNode::collect()` adds an item per enabled mesh (sharing the node's world matrix
        and box), the mesh's own material decides layer and sort order, and
        `MeshNode::doDrawPass()` draws only the item's mesh with the renderer's context. The
        node's hooks still run per item, so subclasses overriding `doPreDraw()`,
        `doPostDraw()` or `doDraw()` (Axis, Grid, LineTrace, DeformableMeshNode) work
        unchanged. `Mesh` undoes its material after drawing (a mesh `ShaderMaterial` left
        its program bound). The model loader gives each further mesh of an Assimp node with
        a different material its own material (all meshes used to share one node material,
        overwritten by the last mesh). All examples pixel-identical (their nodes have one
        mesh each).
     6. Explicit `RenderContext&` through the new path. *Done (renderer and passes):*
        `Renderer(RenderContext &, RenderPass)` uses only its context (transforms, default
        material, statistics); `LightManager::renderShadowMaps(context, queue)`,
        `PlanarReflection::render(context, queue)`, `Node::draw(context)` and
        `Mesh::draw(context)`/`drawInstanced(context, n)` take it explicitly;
        `GLFWSceneWindow` passes its own context. Public `DrawContext` (`draw_context.h`:
        context, `RenderPass`, item, view, projection) reaches nodes through
        `Node::doDrawPass()`, whose default calls `doDraw()`. Materials (`Material::apply()`)
        and the other node hooks keep using the current context: giving them a context
        parameter would stop user overrides of the argument-less hooks from being called.
        That moves with real `collect()` implementations (mesh items drawn by the renderer
        itself) later. All 52 examples pixel-identical.
     7. Docs and a stats readout (draw calls, culled items, state changes). *Done:*
        `Renderer::Stats` adds draw calls, program changes and material applications
        (differences of `RenderContext::counters()`, counted by meshes, text, billboards,
        particles, skybox and `Program::use()`); `LightManager::shadowStats()`,
        `PlanarReflection::stats()`, `GLFWSceneWindow::renderStats()` (`RenderStats` per
        pass), a *Render Statistics* window and `IVF2_RENDER_STATS=n` logging. User guide
        page *Render Queue* (frame, layers, extension levels, own passes, statistics).
   - **Decided:** callback items for custom `doDraw()` nodes are kept permanently (design
     rule 4).
   - **Decided - material inheritance** (inheritance follows the scene, not the drawing
     order). Today stock materials have an empty `unapply()`, so a node without a material
     gets whatever was applied last - often a previous sibling's, not its parent's.
     1. *Nearest material wins:* a node's own material, otherwise the closest ancestor's.
     2. *Default:* a node with no material anywhere in its path gets a neutral default
        (lit, light grey), never leftovers from another node.
     3. *Textures* inherit independently by the same rule.
     4. *Most specific wins:* `Mesh` material > node material > ancestor material.
     5. `setUseMaterial(false)` means "ignore my own material and inherit".
     6. *Instances* resolve per path (an `InstanceNode` takes the material of the group it
        is drawn in).

     Resolved by the collector while walking the tree (no runtime cost); each draw item
     stores its resolved material. Later, opt-in: `setMaterialOverride()` on a parent that
     wins over its children (highlighting a selected group, X-ray views).
   - **Decided - transparency:** materials with alpha < 1 are transparent automatically
     (blending on, transparent bucket, drawn back to front). Today `setAlpha()` has no
     visible effect because stock materials never enable `GL_BLEND`.
   - **Decided - selection on demand:** the selection buffer is re-rendered only when the
     mouse moves, a button is pressed/released or the camera changes (one dirty flag),
     not every frame. Objects animating under a still cursor update their hover state on
     the next mouse event.
   - **Decided - meshes in several windows:** one GPU copy per `RenderContext`, created on
     demand. A mesh keeps its CPU data; the first draw in a window creates that window's
     buffers and VAO, stored in a small per-context table in the mesh and released when
     the context is destroyed (textures get the same mechanism). Any node can be in any
     window's scene without setup and with any host; single-window apps keep exactly one
     copy. Cost: memory and uploads per window that draws the mesh, and dynamic meshes
     are uploaded once per window per change. GL share groups (buffers and textures once,
     VAOs per context) may come later as an optimisation without API changes.
2. **Uniform handling.** *Done (steps 1-4 below).* Cache uniform locations in `Program`;
   move camera, lights and fog into std140 uniform buffers shared by all programs;
   automatic instancing of identical meshes.

   - **Today:** `Program::uniformLoc()` asked the driver on every call (and logged a debug
     line). The managers cache the locations of the stock program, but switching programs
     (`ShaderMaterial::apply()`) re-queries them through `refreshForProgram()` and
     re-uploads camera and all lights (`LightManager::apply()`: about ten
     `"dirLights[i].field"` strings built and looked up per light). Fog is not a scene
     uniform: it exists only as `FogEffect` (post) and in `ParticleSystem`.
   - **Uniform buffers:** one `std140` block per kind of data, owned by the
     `RenderContext`, bound to fixed binding points by `Program::link()` when the program
     declares the block (GLSL 3.30 has no `binding` layout qualifier):
     `IvfCamera` (0: view, projection, view-projection, view position) and `IvfLights`
     (1: counts, point/directional/spot arrays, light-space matrices; shadow samplers stay
     plain uniforms). Unnamed instance blocks keep the member names (`view`,
     `projection`), so shader code using them is unchanged apart from the declaration.
     Changing program then needs only the model matrix and material.
   - **Compatibility:** programs that still declare plain `view`/`projection`/`dirLights[]`
     uniforms (user shaders in `data/shaders`, examples) keep getting them, through cached
     locations; `Program` records once after linking which blocks and legacy uniforms it
     has. `TransformManager::viewMatrix()`/`projectionMatrix()` return mutable references,
     so the camera block is written when the renderer starts a pass (and when matrices are
     set through the manager), not only on change notifications.
   - **Automatic instancing:** opaque mesh items with the same mesh and material and no
     own drawing are merged into one `drawInstanced()` with per-instance model matrices
     (the stock and PBR shaders already have `useInstancing` and instance attributes).
     Builds on the per-mesh items (`DrawItem::mesh`, done). Mesh items still run their
     node's hooks (`doPreDraw()` sets the object id, material, textures and transform), so
     only items whose node keeps the default hooks can be merged; this needs a way to tell
     (for example, a `MeshNode` flag that subclasses with own hooks clear).
   - **Steps:**
     1. Uniform location cache. *Done:* `Program` caches locations per name (also misses,
        as -1) in a map with heterogeneous lookup, cleared by `link()` (shader hot reload);
        no debug line per lookup; `uniformMatrix4Array()` sets the array in one call;
        string views are passed to OpenGL null-terminated. `RenderContext::Counters` and
        `Renderer::Stats` count driver lookups (`uniformLookups`, *Lookups* column):
        shader_material1 70 -> 0 per frame, bump1 49 -> 0, reflection1 44 -> 0. All
        examples pixel-identical (transparency1 and mip_bloom1 vary by 1/255 between runs
        of the same build on this GPU).
     2. Camera uniform block (`IvfCamera`) in the stock, PBR and bump shaders; the
        renderer writes it per pass. *Done:* `uniform_blocks.h` (`CameraBlock`: view,
        projection, viewProjection, viewPos; binding point 0; `IVF_CAMERA_BLOCK_GLSL`);
        `Program::link()` binds the block when declared; `TransformManager` owns the
        buffer and writes it in `syncCameraBlock()` only when the contents changed
        (every view/projection setter, `setViewPos()`, `refreshForProgram()`, and
        `Renderer::execute()` for matrices changed through the references). Stock, PBR,
        bump, glow, mist, water and gradient-sky shaders and the `glow`/`instanced_glow`
        example shaders use the block; `ripple` keeps plain uniforms, which are still set
        through cached locations. `RenderContext::Counters::cameraBlockUploads`. All
        examples pixel-identical, no GL debug messages. `Billboard`, `ParticleSystem`,
        `Skybox` and `DebugDraw` (raw programs with their own `viewProj`) are unchanged.
     3. Lights uniform block (`IvfLights`); `LightManager::apply()` writes the buffer once
        per frame instead of uploading per program; per-program legacy upload only for
        shaders without the block. *Done:* `LightsBlock` (8 point, 4 directional, 8 spot
        lights, `lightSpaceMatrices`, counts; binding point 1; `IVF_LIGHTS_BLOCK_GLSL`
        declares the structs and the block). `LightManager::apply()` builds the block from
        the lights and uploads it only when it changed (`syncLightsBlock()`, also after
        `clear*Lights()`); per program it only binds the shadow maps (samplers cannot be
        in a block) and, for programs that still declare plain light uniforms, uploads
        every light as before. Stock, PBR and bump shaders use the block; their light
        structs dropped unused fields (per-light `lightSpaceMatrix`, point/spot
        `shadowStrength`). Shadow map units and matrices are indexed by light
        (`shadowMaps[i]`, `lightSpaceMatrices[i]` belong to `dirLights[i]`; before they were
        packed by shadow caster, which only matched when the casters came first), and the
        matrices are computed from the same scene box as the shadow pass. PBR now reads at
        most 4 directional lights (was 8; the stock shader always used 4).
        `RenderContext::Counters::lightsBlockUploads`. All examples pixel-identical, also
        with the previous (plain-uniform) stock shader through the legacy path; no GL
        debug messages.
     4. Automatic instancing (per-mesh items are in place). *Done:* the Renderer merges
        consecutive opaque items of the same node, mesh, material and textures
        (`Renderer::instanceable()`; the copies an `InstanceNode` makes) into a batch: the
        node's hooks run once and `MeshNode::doDraw()` passes the world matrices
        (`DrawContext::instances`) to `Mesh::drawInstances()`. That applies the mesh's
        material, then draws one `glDraw*Instanced` call if the current program has
        `useInstancing` (stock, PBR), else one draw per copy (custom shaders), so images
        never change. Requiring the same node answers the hooks question: a node's own
        hooks are the same for all its copies, so no opt-out flag is needed.
        `sortForView()` keeps copies of a (mesh, node) pair that occurs more than once next
        to each other; all other items keep front-to-back order. Per-instance data is shared
        with `InstancedMeshNode` (`instancing.h`: `InstanceAttributes`,
        `bindInstanceAttributes()`); the render context owns one orphaned instance buffer.
        `RenderContext::setAutoInstancing()` / `IVF2_AUTO_INSTANCING=0`, statistics
        `instances`. New example `auto_instancing1` (main pass 359 -> 75 draw calls, 140 ->
        2 program changes, identical image with instancing off; its visual reference comes
        from the CI artifact). Not merged: copies with different materials (lighting1 gives
        every copy its own material), different nodes sharing a mesh, transparent items.
3. **HDR pipeline.** Float render targets, exposure and tone mapping (ACES/AgX) as the
   final post step, MSAA resolve and FXAA/SMAA option.
   *Done (except SMAA):* float targets and the MSAA resolve already existed (RGBA16F scene
   image with a 4x multisampled copy, RGBA16F effect chain); the final copy to the screen
   clamped. The copy shader is now the output step: exposure, tone mapping
   (`ivf::ToneMapping`: None, Reinhard, ACES fit after Stephen Hill, AgX after Troy
   Sobotka/Benjamin Wrensch with its 2.2 display EOTF so all operators output the same
   space), gamma and FXAA (compact FXAA 3.11 on the mapped image), used after the effect
   chain and without effects alike. `GLFWSceneWindow::setToneMapping()`, `setExposure()`,
   `setGamma()`, `setFxaa()` (tone mapping or FXAA turn render to texture on) and
   `setMsaaSamples()` (0 = off, clamped to `GL_MAX_SAMPLES`; was fixed at 4).
   `MipBloomEffect::setToneMapping(false)` leaves the mapping to the window.
   `FrameBuffer::drop()` also frees the multisampled objects when MSAA is turned off. Gamma
   defaults to 1 because materials and lights are authored in display space; a linear
   workflow (sRGB textures, linear lighting) is a separate step. Defaults copy the image
   unchanged: all existing examples pixel-identical. New example `hdr1` and user guide
   page *HDR and Tone Mapping*. Open: SMAA, automatic exposure (eye adaptation).
4. **Image-based lighting.** Load HDR environment maps (`.hdr`/`.exr`), precompute
   irradiance and prefiltered specular maps for the existing PBR material; skybox from
   the same map.
   *Done (except `.exr`):* `EnvironmentMap` (`environment_map.h`) loads an
   equirectangular Radiance `.hdr` (stb), raw float data or a procedural HDR sky
   (`loadSky(SkySettings)`, `skyImage()`; deterministic, so CI needs no HDRI asset) and
   precomputes on the GPU with full-screen passes into cube faces: an RGB16F environment
   cube map with mips, irradiance (32, cosine convolution from a coarse mip), GGX
   prefiltered radiance (128, 5 roughness levels, 256 Hammersley samples reading the
   source mip that matches each sample's solid angle, against fireflies from the sun) and
   the split-sum BRDF LUT (RG16F 256). `GL_TEXTURE_CUBE_MAP_SEAMLESS` is enabled when an
   environment is created. `PBRMaterial::setEnvironment()`, `setEnvironmentIntensity()`
   and `setToneMapping(false)` (linear HDR for the window's tone mapping; the shader's own
   Reinhard + gamma stays the default); the PBR shader's ambient term uses
   `fresnelSchlickRoughness`, irradiance, `textureLod(prefilter, R, roughness * maxLod)`
   and the LUT (units 12-14). `Skybox::setLod()` blurs the background; `Cubemap::allocate()`;
   `writeHdr()` and `imageFlipVerticallyOnLoad()` in `image_io.h`. Orientation checked
   (camera aimed at the sun sees the disc in the centre). New example `ibl1`, unit test
   `test_environment`, guide page *PBR and Image-Based Lighting*. Existing images
   unchanged. Open: `.exr` (needs an EXR library), PBR alpha. (PBR shadows: 2.5.)
5. **Better shadows.** Cascaded shadow maps for directional lights, PCF/PCSS soft
   shadows, cube-map shadows for point lights.
   *Done:* a shadow map is an atlasof tiles in one 2D depth
   texture (GL 3.3, no texture arrays or layered rendering needed): the cascades of a
   directional light (2 x 2) or the six cube faces of a point light (3 x 2, each slightly
   wider than 90 degrees so filters near an edge find depth). Up to `MaxShadowMaps` (4)
   shadow-casting lights share `shadowMaps[4]` on texture units 8-11 (set by
   `Program::link()`; before, units 1-4 clashed with multitexturing and the PBR maps); the
   `IvfLights` block gained `ShadowData shadows[4]` (type, tile count, filter, bias,
   splits, atlas layout, six matrices) and `shadowIndex` per directional and point light,
   replacing `lightSpaceMatrices`. Shared GLSL `shadow_glsl` (`shadow_shaders.h`):
   cascade by view depth or cube face by major axis, normal offset, PCF square
   (`setShadowFilterRadius()`), PCSS (16-tap rotated Poisson blocker search and filter,
   penumbra from the blocker distance, `setShadowSoftness()`) with a receiver-slope bias
   for wide kernels; used by the stock, bump and PBR shaders (PBR now receives shadows).
   `DirectionalLight::setShadowCascades()` (1 = the old single map fitted to the scene,
   still the default and pixel-identical), `setShadowDistance()` (auto: farthest scene
   point), `setCascadeSplitLambda()`; cascades fit the bounding sphere of their frustum
   slice, extend towards the light over the scene's casters and snap to texels
   (`shadow_math.h`, unit test `test_shadow_math`). `PointLight::setShadowRange()`.
   Shadow pass: `ShaderManager::lockProgram()` keeps the stock depth program whatever the
   materials switch to (PBR, bump and `ShaderMaterial` nodes cast correct shadows; before,
   they drew with their own program's camera projection), only opaque items with
   `Node::setCastShadows()` (inherited, `DrawItem::castShadows`) are drawn, and
   `LightManager::apply()` does nothing inside it. `setDebugShadow(5)` tints cascades.
   Shadow maps are created lazily at the needed size. New examples `cascaded_shadows1`,
   `point_shadows1`; guide page *Shadows* rewritten. `auto_instancing1` changed (its
   `ShaderMaterial` copies now cast shadows); other images unchanged. Spot lights
   (`ShadowType::Spot`): one perspective tile covering the outer cone (`spotShadowMatrix()`,
   5% wider for filtering), linear depth compare like point faces, `SpotLight::setShadowRange()`;
   `shadowIndex`/`shadowStrength` use the spot struct's padding (size unchanged).
   `point_shadows1` gained a spot light.
6. **SSAO** and screen-space reflections as effects.
   *Done:* both from the depth buffer only (no G-buffer): positions from `invProjection`,
   normals from neighbouring depths (the side with the smaller step, so silhouettes stay
   sharp). `SsaoEffect`: pre-pass with cosine-weighted golden-spiral hemisphere samples
   turned by interleaved gradient noise and a range check, occlusion in alpha; main pass
   with a depth-aware blur (`setRadius`, `setIntensity`, `setPower`, `setBias`,
   `setSamples`, `setBlurRadius`, `setShowOcclusion`). `ScreenSpaceReflectionEffect`:
   view-space ray march with quadratically growing, jittered steps, binary refinement,
   hits read from the image's mip chain for roughness (`screenTextureLevels`), Fresnel,
   edge and distance fades; reflecting surfaces chosen by world normal
   (`setSurfaceAngle`, `setUpDirection`), since there is no per-material reflectivity
   without a G-buffer. `PostProcessor` passes `projection` and `view` to effects. New
   examples `ssao1`, `ssr1`, guide page *Ambient Occlusion and Reflections*. Existing
   images unchanged. Open: per-material reflectivity and normals (would need a normal /
   material pass), temporal accumulation against noise.
7. **glTF 2.0** as the primary model format: PBR materials, node hierarchy, skinning and
   morph-target animation (fastgltf or cgltf; keep assimp for legacy formats).
   *Done:* cgltf (header-only, MIT, a regular private dependency, so glTF also loads
   without assimp). `GltfModel` (`gltf_model.h`) is a `CompositeNode`: glTF nodes become
   `CompositeNode`s with TRS (new `TransformNode::setRotation(glm::quat)`, applied after
   the other rotations and before the scale, also in `doPreDraw()`), meshes a `MeshNode`
   child with one `Mesh` per primitive (triangles, strips, fans; smooth normals if
   missing), materials `PBRMaterial`s (factors, base colour, packed metallic-roughness,
   normal, occlusion and emissive textures from files, data URIs or the .glb buffer with
   their samplers; alpha modes; vertex colours; KHR_materials_emissive_strength). New PBR
   features: `setMetallicRoughnessMap()`, `setEmissiveMap()`, albedo factor times map,
   `setAlphaMode()` (Opaque/Mask/Blend; Blend sorts as transparent, which closes the PBR
   alpha open item), `setAlphaCutoff()`, `setUseVertexColors()`. Animations: translation,
   rotation (slerp), scale and morph-weight channels with step, linear and cubic spline
   sampling, `setAnimation()`, `setTime()`, looping; skinning (joint matrices relative to
   the mesh node) and morph targets are applied on the CPU into dynamic meshes, so they
   work with every shader, shadows and selection. `ModelLoader::load()` hands .gltf/.glb
   to `GltfModel`. `Texture::loadFromMemory()`, `Texture::setImage()`,
   `loadImageFromMemory()`. Files with required extensions other than
   KHR_materials_emissive_strength and KHR_lights_punctual are refused. Unit test
   `test_gltf` (writes its own glTF files: hierarchy, materials, animation sampling,
   skinning, morphing, errors). New example `gltf1` with a generated showcase model
   (`examples/gltf1/make_showcase.py`, no third-party asset); checked locally with the
   Khronos Fox (skinned) and DamagedHelmet (all texture types) samples. Existing images
   unchanged. Open: GPU skinning for large meshes, glTF lights and cameras, texture
   transforms, Draco/meshopt, alpha-mask shadows.
8. **Resource hot reload** generalized from `ShaderWatcher` to textures and models.
   *Done:* `ResourceWatcher` (`resource_watcher.h`, one per `RenderContext`,
   `resourceWatcher()`; `GLFWSceneWindow::doUpdate()` updates it): watches files with a
   reload function (`watch()`), `watchTexture()` (reloads into the same GL texture;
   `Texture::filename()`), `watchModel()` (a `GltfModel` reloads in place with
   `GltfModel::reload()`, keeping animation by name, time, looping and the node
   transform; other nodes get the children of `ModelLoader::load()`; an onReload callback
   re-applies settings). Polls every 0.5 s; reloads once the modification time has been
   stable for one check (debounce against partial saves); a failed reload keeps the old
   resource and is not retried until the next save (the old ShaderWatcher retried every
   poll). `setOnReload()`, `reloadNow()`, `reloadCount()`, `setEnabled()`. `ShaderWatcher`
   keeps its API but registers with the resource watcher (one polling loop; its `update()`
   forwards). Unit test `test_resource_watcher` (debounce, failures, several files,
   unwatch, model reload in place); new example `hot_reload1` (writes its own live files;
   tested live with edits during a run, also with the GL debug context); guide page
   *Hot Reload*.
9. **CPU picking** via ray casting against a per-mesh BVH, complementing the existing
   color-buffer selection.
   *Done:* `ray.h` (`Ray` with t kept under affine transforms, `rayFromScreen()`,
   `intersectBox()`, `intersectTriangle()` two-sided Möller-Trumbore), `MeshBvh`
   (`mesh_bvh.h`: median split on the longest centroid axis, leaves of up to 4
   triangles, nodes with children side by side, near-first traversal; `intersect()`,
   `occluded()`), `Mesh::bvh()` (lazy, rebuilt when `end()`/`updateVertices()` change the
   versions; triangles, strips, fans, quads; null for lines and points),
   `Mesh::primType()`. `pick()`/`pickAll()` (`picking.h`) over a `RenderQueue` with the
   items' world matrices and bounds (instances, animation and CPU-deformed glTF meshes
   hit where drawn), skipping background/overlay items, `PickOptions` (skip, maxDistance,
   includeTransparent, boundsFallback for meshless nodes), `RayHit` (node, mesh,
   distance, position, normal facing the ray, triangle, barycentrics, exact).
   `GLFWSceneWindow::pick()`, `cameraRay()` (window coordinates, high-DPI safe) and
   `setSelectionMethod(SelectionMethod::Ray)` for hover and click events without the
   selection buffer. Fixed on the way: `BufferSelection::initialize()` left its
   framebuffer bound, so a selection-enabled window drew its first frame into it. Unit
   test `test_picking` (BVH against brute force with 2000 random rays, instances, layers,
   transforms); new example `picking1`; guide section *Ray picking*. Existing images
   unchanged.

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

   **Plan.** A separate static library `ivf2::ivfvis` (`src/ivfvis`, `include/ivfvis`,
   option `IVF2_BUILD_VIS`, like `ivfmusic`) that depends on `ivf` only; `Texture3D` goes
   into core `ivf`. Public headers stay free of third-party headers (`header_check_ivfvis`);
   the VTU reader uses pugixml/miniz privately with a fallback. Examples use analytic data
   (blobs, Lorenz attractor, dipole field) so visual tests stay deterministic; small reader
   samples go in `examples/assets/data/`. One branch per step, each with an example, a
   visual reference and unit tests where GL-free.

   1. *Foundation.* `UniformGrid` (dimensions, origin, spacing, named scalar/vec3 point
      arrays, trilinear `sample()`/`gradient()`), `PointData`, `VectorField` interface
      (grid, `FlowField` and lambda adapters). `Colormap` (viridis, magma, inferno, plasma,
      cividis, turbo, coolwarm, gray; user control points; range, clamp, log, reverse,
      discrete steps; 256-entry table, texture upload). `ScalarColoring` (CPU vertex
      colours from a per-vertex array on any `MeshNode`). `ColorBar` overlay with ticks and
      labels. Example `vis_colormaps1`.
      *Done:* `ivfvis` library (`IVF2_BUILD_VIS`, depends on `ivf` only): `DataSet` (named
      scalar/vector arrays, `fillScalars()`/`fillVectors()`, `addMagnitude()`, `range()`),
      `UniformGrid` (x fastest; trilinear `sample()`/`sampleVector()`, `gradient()`, 2D grids),
      `PointSet`, `VectorField` (`fromGrid`, `fromFunction`, `fromFlowField`), `Colormap` with
      matplotlib's tables (256 entries, generated by `src/ivfvis/make_colormap_data.py`),
      `colorVertices()`/`useVertexColors()`, `ColorBar`, ticks (`niceTicks`, `logTicks`,
      `formatTick`). Core: `OverlayNode` (children drawn in pixels in the overlay layer, main
      pass only, nested render queue), `TextNode::setResolution()`/`setColor()` with glyphs
      cached per font and size in `FontManager::glyphs()`, `Mesh::colors()`/`updateColors()`.
      Fixed on the way: the stock shader's lights ignored vertex colours (only a 0.15 base
      term used them). Unit test `test_ivfvis`; guide section *Visualisation*. Existing images
      unchanged (1/255 on a few pixels). Open: GPU colormap path (scalar attribute + colormap
      texture) for crisp band edges with `setSteps()`; overlay size on high-DPI screens.
2. *Point clouds.* `PointCloud` (distance-attenuated round sprites, colormapped or solid,
      optional size array, bounds for culling and picking). Example `vis_points1`.
      *Done:* `PointCloud` (`TransformNode`): one `GL_POINTS` draw with its own program per
      context (`ivfvis_points`, compiled through the context's `ShaderManager`), buffers and a
      256-entry colormap texture per context (released with `deleteLater()`); colours looked up
      on the GPU (range, log scale, out-of-range and NaN colours as uniforms, nearest sampling
      for steps), world or pixel sizes, `setSizeBy()`, disc/sphere/square shapes, opacity
      (transparent layer), selection-buffer ids, no shadows. Core: `Node::intersectRay()` for
      custom ray tests in `pick()`, `RayHit::point`; `PointCloud` tests every point against
      the ray (radius from the drawn size). Example `vis_points1` (500 000 Lorenz points).
      Open: an acceleration structure for picking very large clouds; shadows of points.
3. *Scalar fields.* `Isosurface` (marching cubes, gradient normals, colour by a second
      array, rebuild on iso change), `SlicePlane` (arbitrary plane resampled to a
      colormapped texture, optional contour lines). Example `vis_scalar1`.
      *Done:* `extractIsosurface()` with marching cubes cases generated at start-up from the
      cube's geometry (no hand-typed table): face segments directed with the inside corners
      on the left chain into oriented loops; ambiguous faces cut off their inside corners
      (the same in both cells, so no cracks); loops are fanned from a vertex whose diagonals
      leave the cube faces, else around a centre vertex. Shared vertices per grid edge,
      gradient normals, values of a second array. `extractSlice()` (plane grid clipped to the
      box, shared interior vertices), `contourLines()`, `SurfaceData`. Nodes `Isosurface`
      and `SlicePlane` (unlit vertex colours instead of a texture: the stock shader samples
      textures only when lit; contour lines as a second mesh lifted off both sides). Tests:
      closed sphere with correct area and orientation, crack-free random fields, slice areas,
      contour length. Example `vis_scalar1`.
4. *Vector fields.* `Glyphs` (instanced arrows oriented, scaled and coloured by the
      vector), `Streamlines` (RK4, point/line/plane seeds, stop criteria; lines or stream
      tubes via `PathFrames`/`Extrusion`). Example `vis_vectors1`.
      *Done:* `Glyphs` builds one merged, lit mesh (arrow: shaft, head and caps with 10
      sides; cone; or unlit lines) instead of `InstancedMeshNode`, which has a single
      context's buffer and no bounds or picking: works in several windows, casts shadows,
      culled, picked per triangle (`glyphIndex()`). Magnitude or constant scaling, tail or
      centre anchor, minimum magnitude, colormap by |v| following its changes.
      `traceStreamline()` steps along the normalised field (RK4, equal step lengths) until
      it leaves the bounds, weakens below `minSpeed` or hits the limits; `Streamlines` draws
      unlit lines or tubes with frames transported along the line (own generator rather than
      `ExtrusionBuilder`, to colour every ring by speed). Seed helpers. Tests: circles of a
      rotation field, wall-to-wall uniform flow, stopping, node geometry. Example
      `vis_vectors1` (dipole with log colormap). Open: instanced glyphs for very large
      counts.
5. *Volume rendering.* `Texture3D` in core; `VolumeNode` (box proxy, ray marching in the
      transparent layer, transfer function = colormap + opacity curve, jittered start,
      early termination, stops at opaque scene depth). Example `vis_volume1`.
      *Done:* `ivf::Texture3D` (CPU data, per-context upload like `Mesh`, R8/R16F/R32F).
      `Volume` draws the back faces of the grid's box (works with the camera inside) with
      its own program per context; the fragment shader intersects the ray with the box,
      marches front to back (opacity `1 - exp(-a * extinction * step)`, extinction scaled by
      the box diagonal, per-pixel jitter, stop at 99 %), optional gradient shading. Opaque
      occlusion: in the main pass the current depth buffer is blitted into a depth texture of
      the same format (queried from the framebuffer: 24-bit, 24+8 stencil, 32F; falls back
      to no occlusion with a warning) and the ray ends at that depth. Transfer function as a
      256-entry RGBA texture. Visual test with 3 frames (llvmpipe is slow at ray marching).
      Open: transparent objects inside volumes, empty-space skipping, a transfer function
      editor widget.
6. *Axes and annotation.* `Axes3D` (bounding box with nice-number ticks, camera-facing
      labels, titles, back-face grid planes; sized from a node's bounds).
      *Done:* `Axes3D` (`CompositeNode`): six walls (grid lines and outline) that draw only
      while facing away from the camera, twelve tick edges of which each axis draws the one
      chosen by `axisEdge()` (outline edges, lowest on screen for x/z, leftmost for y), all
      deciding in `doDrawPass()` from the pass's camera (no frame lag), and a labels
      `OverlayNode` that projects tick points to the screen and pushes labels and titles
      outward. Core: `OverlayNode` keeps the scene camera and parent matrix of its draw
      (`project()`). Children are built after construction (`create()` or first collect),
      because `CompositeNode::add()` needs `shared_from_this()`. Example `vis_axes1`.
      Open: labels hidden behind data are still drawn; minor ticks; log axes.
7. *Readers.* CSV, `.npy`, VTK legacy (structured points, polydata, unstructured grid),
      VTU (ASCII/base64/zlib; optional). All return `ivf::Result<T>`; unit tests.
      *Done:* `readers.h`: `readCsv()` (delimiter and header detection, quotes, comments;
      `Table`, `pointsFromTable()`), `readNpy()` (v1-3, b/i/u/f, both byte orders, C and
      Fortran order; `gridFromNpy()` incl. vector arrays, `pointsFromNpy()`), `readVtk()`:
      legacy 2.0-5.1 ASCII/binary (structured points, polydata, unstructured grids,
      structured/rectilinear grids as points; METADATA and FIELD blocks skipped) and XML
      .vtu/.vtp/.vti (ASCII, inline base64, appended raw/base64, UInt32/UInt64 headers,
      zlib blocks; appended data cut out before parsing with pugixml). Cells to triangles
      (polygons, strips, quads, pixels; outer faces of volume cells by face counting) and
      lines. Option `IVF2_WITH_VTK_XML` (vcpkg feature `vtk-xml`: pugixml, miniz; off in the
      minimal preset, where XML gives `NotSupported`). Test files written by VTK 9.6 and
      numpy (`tests/data/make_test_data.py`), `test_ivfvis_readers` checks every encoding.
      Example `vis_readers1` (data from `make_data.py`). Open: cell data, `.vts`/`.vtr`,
      multi-block `.vtm`, NumPy `.npz`.
   8. *2D plots.* ImPlot as optional `IVF2_WITH_IMPLOT` in `ivfui` (context per ImGui
      context), helpers for grid lines and histograms. Example `vis_dashboard1`.
      *Done:* ImPlot 1.0 through the vcpkg feature `implot`, linked publicly by ivfui (like
      ImGui; `IVF2_HAS_IMPLOT`); `UiRenderer` creates, switches and destroys one ImPlot
      context per window with its ImGui context. `plot_data.h`: `sampleLine()`,
      `histogram()` (GL- and UI-free, tested). Example `vis_dashboard1`: probe profile and
      histogram with drag lines setting the colormap range (fallback without ImPlot).
   9. *Wrap-up.* Guide section "Scientific visualisation", gallery, changelog.
      *Done:* each step updated the guide, gallery and changelog as it went; the wrap-up
      added a first complete program to the overview, the Visualisation Examples page and
      a section in the example index, ivfvis highlights on the landing page and in the
      README, and cleaned up the API reference (examples grouped by topic, wrong `@param`
      names and duplicate `@example` names fixed; 100 Doxygen warnings down to 15).

   Decisions: CPU vertex colours first (work with every material, shadows and picking; a
   GPU scalar attribute + colormap texture path later if needed); uniform grids only at
   first (rectilinear/unstructured data load as points or meshes).
2. **Python bindings** (nanobind) for the scene graph, primitives, materials and
   windowing; publish wheels. Opens the library to scientists, students and Jupyter users.
3. **Web target.** Emscripten build (GLES 3 / WebGL2) so examples run in the browser -
   the best possible showcase in the documentation.

   **Plan.** The same libraries compiled to WebAssembly with Emscripten, rendering through
   WebGL2 (OpenGL ES 3.0); no separate renderer. A `wasm` CMake preset uses vcpkg's
   `wasm32-emscripten` triplet (GLFW from Emscripten itself; ImGui, glm, FreeType, stb,
   cgltf from vcpkg) with assimp, audio, MIDI and ivfmusic off. glad stays: on the web
   `RenderContext::create()` loads it through a shim that reports GL 3.3 and forwards to
   Emscripten, so headers and call sites stay unchanged; the few desktop-only calls
   (`glPolygonMode`, multisampled textures, border clamping, debug output) check
   `RenderContext::isGles()`. Shaders keep `#version 330 core`; `Shader` rewrites the
   version line to `#version 300 es` with default precisions when compiling on GLES, so
   one source serves both. One window per page (the canvas), no threads (so GitHub Pages
   can host the builds without special headers). One branch per step, like `ivfvis`.

   1. *Toolchain and first example.* `wasm` preset and vcpkg platform filters; GL loader
      shim and `isGles()`; shader version translation; `GLFWApplication::run()` split into
      a per-frame step driven by `emscripten_set_main_loop`; `ivf2_add_example()` builds
      `.html`/`.js`/`.wasm` with shaders, fonts and assets preloaded and a page template.
      `hello_ivf` runs in the browser.
      *Done:* preset `wasm` (vcpkg triplet `wasm32-emscripten`; `glfw3`/`opengl` filtered
      out, Emscripten's GLFW and WebGL 2 as `glfw`/`OpenGL::GL` targets). `RenderContext`
      loads glad through a shim reporting GL 3.3 on OpenGL ES; `isGles()`. `ivf/glsl_es.h`:
      `translateToGlslEs()` (version line first, precisions, `#line`; uniform initialisers
      removed and set after linking as `UniformDefault`s, since ES has none) used by
      `Shader`/`Program` and the classes compiling their own shaders; GL-free tests.
      `GLFWApplication::frame()` runs under `emscripten_set_main_loop`, the canvas follows
      its CSS size. `ivf2_web_executable()` (page template `cmake/ivf2_web_shell.html`,
      preloaded `shaders/`, `fonts/` and listed assets, function names kept);
      `add_ivf2_example(... WEB_ASSETS ...)`. Guarded for GLES: polygon modes, GPU timer
      queries, debug output. Fixed on the way: constant sampler indices for multitexturing,
      `SubdivideShape` with newer Clang. `hello_ivf` runs in Chrome (headless, SwiftShader),
      camera control included.
   2. *GLES compatibility.* Desktop-only GL replaced or guarded: MSAA through multisampled
      renderbuffers (also on desktop), shadow map borders in the shader, wireframe,
      float render targets (`EXT_color_buffer_float`), depth blits; every built-in shader
      compiles as GLSL ES 3.00 (a GL-free test translates and validates them with
      glslang where available). Lighting, shadows, PBR, HDR and effects examples run.
      *Done:* MSAA through multisampled renderbuffers on every platform (sample count limited
      by the RGBA16F format on ES); shadow maps clamp to edge on ES (the shader already
      rejects coordinates outside the map), `glDrawBuffers` instead of `glDrawBuffer`; no
      seamless-cube-map switch, `GL_PROGRAM_POINT_SIZE` or `GL_POLYGON_OFFSET_LINE` on ES;
      wireframe on ES as `GL_LINES` from a per-copy edge index buffer; colour picking buffer
      RGBA8 (ES reads back RGBA only). WebGL refuses draws into a texture a sampler can see
      (desktop drivers tolerate it): render targets, the output step, the effect chain and
      the shadow pass now unbind their textures. Shaders fixed for GLSL ES (`textureSize()`
      as `ivec2`, integer `pow()` exponents, interface blocks). Instead of glslang, which
      is not always installed, `tests/web/shader_check` compiles all 58 built-in and file
      shaders with the browser's WebGL 2 compiler. 22 examples run in Chrome without WebGL
      errors: shadows (cascaded, point, spot, PCSS), PBR/IBL, glTF, HDR/FXAA, reflections,
      SSAO, SSR, depth of field, bloom, fog, transparency, textures, bump mapping, skybox.
      Float render targets rely on `EXT_color_buffer_float` (enabled automatically; all
      current desktop browsers have it).
   3. *Examples.* The examples build for the web (a list of exclusions: threads, MIDI,
      audio, several windows, file dialogs); ImGui and ImPlot; ivfvis (point clouds,
      volumes); resizing with the page and high-DPI canvases; environment variables
      (`IVF2_FIXED_TIMESTEP`, ...) from the page URL.
      *Done:* the web build uses the normal example list minus `IVF2_WEB_EXCLUDED_EXAMPLES`
      (`multi_window1`, `embedding1`: several windows or a host-owned loop; `audio_reactive1`,
      `music_scene1`: audio; `assimp1`: assimp and a native file dialog), so new examples
      build for the browser by default; examples that load files list them in `WEB_ASSETS`.
      67 examples run in Chrome without WebGL errors, among them ImGui and ImPlot panels,
      every `ivfvis` example (point clouds, volumes, VTK/CSV/`.npy` readers), text, picking,
      hot reload (in the page's memory file system) and ReactPhysics3D (`rigid_body1`).
      Fixed for OpenGL ES: sized `GL_R8`/`GL_RG8` formats for glyphs and one/two-channel
      images, no `GL_CONTEXT_FLAGS` query, no `glPolygonMode` in two examples. The canvas
      follows its parent element (`#viewport` in the page template) and Emscripten's GLFW
      scales it to device pixels (`GLFW_SCALE_TO_MONITOR`); `RenderContext::pixelRatio()`
      lets `OverlayNode` (colour bars, axis labels) and pixel-sized point clouds keep their
      size and stay sharp on high-DPI screens, which also fixes them on macOS Retina.
      `ivf2_web_pre.js` (`--pre-js`) turns `IVF2_*` URL query parameters into environment
      variables before `main()`.
   4. *CI and documentation.* A CI job builds the web examples (emsdk cached) and smoke
      tests them in headless Chrome; the docs workflow publishes them under `web/`, guide
      pages embed or link the live examples, and a "Web" guide page explains building
      your own app for the browser.
      *Done:* `.github/workflows/web.yml` (reusable; from `build.yml` with the full matrix and
      from `docs.yml`) installs the emsdk (cached with its system libraries), builds preset
      `wasm` with ccache and a separate vcpkg cache, opens every page in headless Chrome
      (`tests/web/smoke_test.mjs`: starts, renders frames, no errors or WebGL warnings;
      `shader_check` reports 0 failures) and uploads the pages and screenshots. `docs.yml`
      deploys only if that passes and publishes the pages under `web/` on GitHub Pages
      (<https://jonaslindemann.github.io/ivf2/web/>), with a start page from
      `make_gallery.py --web`; the script also keeps a generated "Run in the browser" box on
      the 34 guide pages the Example Index links examples to. Guide page "Running in the
      Browser" (building, URL settings, hosting, portable code, testing). The installed package
      works in Emscripten projects (`ivf2Emscripten.cmake` replaces the OpenGL/GLFW/Threads
      lookups), tested in CI by building `tests/package/web_app.cpp` as a page. Found on the
      way: with ANGLE's D3D backend (WebGL on Windows) the stock shader took ~25 s to link,
      because the shadow filter was inlined once per shadow map and sampled with implicit
      derivatives in loops; now ~7 s (only the fetch branches on the map, `textureLod()`,
      `textureGrad()` for multitexturing), and SwiftShader starts the shadow examples 4x faster.
      Open: evaluating the shadow filter once per map instead of once per light loop would
      bring the link time to about 2 s (needs a shader API change: shadows computed in `main()`).
   5. *Wrap-up.* Gallery with live links, changelog, README.
      *Done:* the Example Gallery has a "Run" badge on each of the 67 web examples
      (`make_gallery.py`), the landing page and the Example Index link the live examples, the
      README has a web section, a link at the top and the docs badge. The web target is complete;
      open follow-ups: shadows evaluated once per map (shader link time on Windows), ccache hits
      for the web build (precompiled headers), mobile browsers not yet tested.
4. **Headless and offline rendering.** Render without a visible window (EGL/OSMesa),
   video export through an ffmpeg pipe from `FrameWriter`, deterministic time stepping.
   Useful for figure generation, CI and music videos (`ivfmusic`).
5. **Text and 2D overlay.** SDF/MSDF font rendering for crisp scalable text, a 2D
   drawing layer (thick lines, shapes, images) for HUDs and creative coding.
6. **GPU compute.** Compute-shader particles and fields where GL 4.3 is available,
   transform-feedback fallback for macOS.
7. **Physics module.** Wrap the ReactPhysics3D integration from `examples/rigid_body1`
   into an optional `ivfphysics` library (rigid bodies synced to `TransformNode`), built
   together with a larger example: an interactive physics sandbox, the first "complete
   application" among the examples and a showcase for the live web examples.

   **Plan.** A library `ivf2::ivfphysics` (namespace `ivfphysics`, option
   `IVF2_BUILD_PHYSICS`, vcpkg feature `physics` with ReactPhysics3D, which then leaves the
   `examples` feature), built like `ivfvis`: it depends on `ivf` only (not `ivfui`), its
   examples are named `physics_*`, and it builds for the web too (ReactPhysics3D already does,
   see `rigid_body1`). Public headers do not include ReactPhysics3D: bodies, shapes and joints
   are ivf2 types implemented over it (pimpl), so applications need no physics headers and
   the engine could be replaced later (Jolt) without changing the API. The physics state is
   GL-free, so unit tests can step worlds without a window.

   - `PhysicsWorld::create(settings)`: gravity, solver iterations, sleeping; owns the engine.
     `step(dt)` runs fixed substeps (default 1/240 s) from an accumulator and interpolates the
     node transforms between them, so motion is smooth at any frame rate and identical for
     the same inputs (deterministic: required by the visual tests and `IVF2_FIXED_TIMESTEP`).
     `pause()`, `stepOnce()`, `reset()` to the state the bodies were added in.
   - Bodies are attached to nodes: `world->addBody(node, shape, {mass, friction, bounciness,
     type})` returns a `RigidBodyPtr`; dynamic bodies drive the node's position and rotation,
     kinematic bodies follow the node, static bodies are fixed. `Shape::box/sphere/capsule/
     plane/convexHull(mesh)/triangleMesh(mesh)` and `Shape::fromNode(node)`, which derives
     the shape from the primitive (`Cube`, `Box`, `Sphere`, `Capsule`, `Cylinder` as a convex
     hull, a `MeshNode`'s meshes). Compound bodies from several shapes.
   - Forces, impulses, velocities; collision categories and masks; triggers.
   - Queries and events: `raycast()` (closest hit or all, with body, point, normal) and
     contact callbacks (begin, stay, end; contact points and impulse), queued during the step
     and delivered after it so callbacks may add and remove bodies.
   - Joints: ball and socket, hinge (limits, motor), slider (limits, motor), fixed; breakable
     above a force.
   - Interaction (in `ivfphysics`, GL-free; a camera ray in): `DragController` pulls a body
     at the grabbed point with a damped spring (the usual "mouse spring"), `throwBody()`
     launches a body along a ray.
   - Debug view: colliders, contact points and normals, joint axes, sleeping bodies through
     `DebugDraw`; statistics (bodies, awake bodies, contacts, step time).

   **The example, `physics_sandbox1`:** presets selected in an ImGui panel (brick wall,
   domino run, tower to knock over, Newton's cradle and a pendulum chain with joints, a ramp
   course); click to throw spheres along the view direction, drag bodies with the mouse,
   pause / step / reset, sliders for gravity, friction, bounciness and simulation speed. Bodies
   are coloured by speed with an `ivfvis` colormap and colour bar, with optional velocity
   arrows and contact points. Shadows, PBR materials and an environment map. Deterministic
   (fixed step, seeded randomness), so it gets a visual test, and it runs in the browser.

   1. *Library and bodies.* Target, option, vcpkg feature, package component `Physics`,
      `header_check_ivfphysics`; `PhysicsWorld` with fixed substeps and interpolation; bodies
      on nodes; box, sphere, capsule, plane, convex hull and triangle mesh shapes,
      `Shape::fromNode()`; forces and impulses. `rigid_body1` rewritten on `ivfphysics` (as
      the minimal example). GL-free tests: a falling box lands where it should, node sync,
      the same run twice gives the same transforms, reset.
      *Done:* `include/ivfphysics/` (`PhysicsWorld`, `RigidBody`, `Shape`; engine data behind
      `Impl` structs in `src/ivfphysics/engine.h`). The simulation runs at most one step ahead
      of the shown time and nodes are interpolated to it, so whole multiples of the step show
      the exact state. Convex hulls are built by ivfphysics (`convex_hull.cpp`: incremental, double
      precision, coplanar faces merged into polygons) and handed to the engine as polyhedra:
      ReactPhysics3D's own QuickHull asserts on the coplanar points of capped cylinders and
      boxes. A plane is a 1-unit thick box (the engine has no plane shape). Bodies use world
      coordinates (nodes without transformed parents); Euler angles and axis rotations of a node
      are folded into its quaternion when its body is added. `test_ivfphysics` (12 cases),
      `header_check_ivfphysics`, package consumer `package_test_physics`; `minimal` builds
      without physics.
   2. *Queries, events and interaction.* Raycasts, contact events, triggers, collision
      filtering, `DragController`, `throwBody()`, debug view and statistics; tests.
      *Done:* `events.h` (`Contact`, `TriggerEvent`, `RaycastHit`, `PhysicsStats`),
      `interaction.h` (`DragController`, `throwBody()`), `debug_view.h` (`drawDebug()`).
      The engine's reports are queued during a step and delivered after it; active pairs are
      tracked by ivfphysics, so removing a body reports End for its contacts (the engine does
      not) and the order of events is deterministic. Contacts begin one step after the bodies
      meet (the first step finds the pair) and are not listed while both bodies sleep; falling
      asleep ends nothing. `PhysicsWorld::addStepCallback()` runs before every fixed step: the
      drag spring (acceleration-based, so every body follows alike) uses it. ReactPhysics3D
      0.10.2 crashes when filter bits change on a collider of a sleeping pair, so
      `setCollisionFilter()` wakes the dynamic bodies first. `rigid_body1` gained mouse dragging,
      throwing and the debug view. `test_ivfphysics`: 20 cases.
   3. *Joints and compound bodies.* The four joint types with limits and motors, breaking,
      compound shapes; tests (a pendulum keeps its length, hinge limits hold).
      *Done:* `joint.h` (`Joint`, `JointDesc`; `PhysicsWorld::add{BallSocket,Hinge,Slider,Fixed}Joint()`,
      `removeJoint()`, joint break callback), `Shape::compound()` (one collider per part; contacts
      count collider pairs so a compound body touching with several parts reports one Begin/End).
      Joints keep their anchor and axis in the first body's coordinates, so a broken joint is rebuilt
      by `reset()`. A body held to the world is the engine's second body after a hidden static body
      mirrored through the anchor: ReactPhysics3D measures angles, translations, motors and the cone
      of its second body relative to its first, and takes the cone axis from each body's position
      through the anchor. Its hinge motor diverges in 0.10.2 (the body spins up at full torque), so
      hinge motors are applied by `PhysicsWorld` before each step (the torque that reaches the
      motor speed in one step, capped by the maximum torque); the slider motor is the engine's.
      New example `physics_joints1` (visual test). `test_ivfphysics`: 27 cases.
   4. *The sandbox.* `physics_sandbox1` with presets, interaction, panels and the `ivfvis`
      colouring; visual test reference; runs in the browser (smoke test).
      *Done:* five presets (brick wall, domino run, log tower and wrecking ball, Newton's cradle
      and a chain, ramp course of compound static bodies), each starting in motion; dragging
      (wheel: distance), throwing (Ctrl + click, Space), sliders for gravity, friction,
      bounciness, time scale and throw speed; per-body `PBRMaterial`s coloured from speed by an
      `ivfvis` colormap with a `ColorBar`; procedural sky environment, shadows, ACES.
      `IVF2_SANDBOX_PRESET` selects the preset (a page URL parameter in the browser). Findings:
      a Newton's cradle needs small gaps between the balls (touching balls are solved as one
      contact group and the first ball bounces back); bodies wedge where the gap under a ramp's
      end is about their size, so the ramps are 3.5 apart with back walls above the ramp before.
      The physics examples moved off the scene window's T, S and V shortcuts (Space, N, L).
   5. *Documentation and wrap-up.* Guide section "Physics" (world and bodies, shapes, events,
      joints, interaction), example pages and gallery, changelog, README.
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

1. CI (phase 1.1) - everything else becomes safer to change. *Done.*
2. Hide third-party headers + optional dependencies (1.3) - largest effect on usability. *Done.*
3. Render context (1.2) and diagnostics (1.5). *Done.*
4. Render queue + uniform buffers (2.1, 2.2) - the base for HDR, IBL, shadows. *Render queue
   done; uniform handling is next.*
5. Scientific visualisation module and Python bindings (3.1, 3.2) - the main differentiators.
   *Visualisation module and web target (3.3) done.*
6. Physics module with an interactive sandbox example (3.7) - completes the base
   functionality before the Python bindings (3.2), which follow it.
