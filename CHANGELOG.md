# Changelog

All notable changes to ivf2 are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and ivf2 uses
[semantic versioning](https://semver.org/) (see [CONTRIBUTING.md](CONTRIBUTING.md#versioning-and-releases)).
While the version is `2.0.0-alpha.*`, the API may still change between pre-releases;
renamed functions keep a `[[deprecated]]` alias for at least one release.

## [Unreleased]

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
  example assets to `examples/assets/`. Build output goes to `build/<preset>/bin/<Config>/`
  (`IVF2_OUTPUT_IN_SOURCE_TREE=ON` restores the old layout).

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

[Unreleased]: https://github.com/jonaslindemann/ivf2/compare/2.0.0-alpha...HEAD
[2.0.0-alpha]: https://github.com/jonaslindemann/ivf2/releases/tag/2.0.0-alpha
