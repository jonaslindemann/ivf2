# ivf2 Roadmap

A plan for making ivf2 a high-quality, widely usable C++ library for interactive 3D
visualisation and creative coding. Based on a review of the code base in November 2026
(~33k lines of library code in `ivf`, `ivfui`, `ivfmath`, `ivfmusic`; 54 examples).

## Status (24 November 2026)

Latest release: **2.0.0-alpha.2** (22 November 2026). `main` adds the render queue (unreleased,
see `CHANGELOG.md`).

| Phase | State |
|-------|-------|
| 0 - Build system | Done |
| 1 - Foundations | Done: CI, render context (step A), private third-party headers and optional dependencies, glm only, diagnostics, `Result<T>` error handling, warnings and API polish, visual regression tests, repository hygiene. Open items listed below |
| 2 - Rendering | **2.1 render queue done** (steps 1-7). **2.2 uniform handling in progress** (steps 1-3, location cache, camera and lights blocks, done; one item per mesh for `MeshNode` done; next: automatic instancing). 2.3-2.9 not started |
| 3 - Differentiators | Not started |
| 4 - Reach | Not started |

**Open items from finished work**

- Render queue (2.1): materials applied with the renderer's explicit context (completes
  step B; meshes already are); alpha for the PBR material; `setCastsShadows()` per node;
  `setMaterialOverride()` (decided, opt-in).
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
2. **Uniform handling.** Cache uniform locations in `Program`; move camera, lights and
   fog into std140 uniform buffers shared by all programs; automatic instancing of
   identical meshes.

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
     4. Automatic instancing (per-mesh items are in place).
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

1. CI (phase 1.1) - everything else becomes safer to change. *Done.*
2. Hide third-party headers + optional dependencies (1.3) - largest effect on usability. *Done.*
3. Render context (1.2) and diagnostics (1.5). *Done.*
4. Render queue + uniform buffers (2.1, 2.2) - the base for HDR, IBL, shadows. *Render queue
   done; uniform handling is next.*
5. Scientific visualisation module and Python bindings (3.1, 3.2) - the main differentiators.
