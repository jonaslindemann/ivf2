# Contributing to ivf2

Thank you for your interest in ivf2! Bug reports, feature ideas, documentation fixes and code
are all welcome.

- **Questions and ideas:** open a [GitHub issue](https://github.com/jonaslindemann/ivf2/issues)
  using the feature request template.
- **Bugs:** use the bug report template and include your OS, compiler, GPU/driver and the log
  output (ivf2 logs OpenGL driver messages in Debug builds).
- **Security issues:** see [SECURITY.md](SECURITY.md) instead of opening a public issue.

## Building

You need CMake 3.24+, a C++23 compiler (MSVC 2022, GCC 13, Clang 17 or Xcode 16 or newer) and
[vcpkg](https://vcpkg.io), which CMake finds through `VCPKG_ROOT`.

```
cmake --preset vs                 # Windows; Linux/macOS: cmake --preset debug
cmake --build --preset vs-debug   #                       cmake --build --preset debug
ctest --preset vs-debug           #                       ctest --preset debug
```

Executables and their runtime data end up in `build/<preset>/bin/` (`build/vs/bin/Debug/` with Visual Studio). See the
[installation guide](https://interactive-visualisation-framework-2.readthedocs.io/) for all
options.

## Repository layout

| Directory | Contents |
|-----------|----------|
| `include/<lib>/`, `src/<lib>/` | Libraries: `ivf` (scene graph, rendering), `ivfui` (GLFW windows, ImGui), `ivfmath`, `ivfmusic`, `generator` |
| `data/` | Runtime data shipped with the library (`shaders/`, `fonts/`) |
| `examples/` | Example programs, one directory each; shared models and textures in `examples/assets/` |
| `tests/` | Unit tests (GL-free), header checks, package consumer test, visual tests (`tests/visual/`) |
| `docs/` | User guide (Zensical) and Doxygen configuration |
| `cmake/` | Build modules, install rules, package config |

## Making a change

1. Fork the repository and create a branch for your change.
2. Keep the change focused: one feature or fix per pull request.
3. Build with warnings on (the default) and keep your code warning-free; CI builds with
   warnings as errors on MSVC, GCC and AppleClang.
4. Run the unit tests (`ctest`) and the examples your change affects. Examples can run
   without showing windows: `IVF2_HIDE_WINDOWS=1 IVF2_EXIT_AFTER_FRAMES=90`.
5. If the change alters what examples look like, the visual tests in CI will fail; download the
   `visual-tests` artifact and update `tests/visual/reference/` (see the
   [diagnostics guide](docs/docs/advanced/diagnostics.md#visual-regression-tests)).
6. Add an entry to the `[Unreleased]` section of [CHANGELOG.md](CHANGELOG.md) for changes
   users will notice.
7. Open a pull request and describe what changed and how you tested it.

## Coding conventions

- Format with the repository's `.clang-format` (Microsoft style, 4 spaces).
- Classes are created through `ClassName::create(...)` returning `std::shared_ptr`
  (`using ClassNamePtr = std::shared_ptr<ClassName>`); factories are `[[nodiscard]]`.
- Getters are named after the property and are `const` when they only read (`texture()`),
  setters are `setX()`. Strings are passed as `const std::string &` or `std::string_view`.
- Expected failures (missing files, bad formats, shader compile errors) return
  `ivf::Result<T>` and are created with `ivf::makeError(...)`; exceptions are only for
  programming errors.
- Use the Logger (`ivf::logInfo`, `logWarning`, `logError`) instead of `std::cout`.
- GL state that belongs to one OpenGL context goes into `ivf::RenderContext`, not into
  function-local statics.
- Public headers must not include optional or private dependencies (assimp, FreeType, miniaudio,
  RtMidi, stb, nlohmann-json, ImGui in `ivf`); CI checks this. Guard optional features with the
  `IVF2_HAS_*` macros from `<ivf/config.h>`.
- Don't break existing code: when renaming a public function, keep the old name as a
  `[[deprecated("Use x() instead")]]` alias.
- Examples must not depend on the wall clock or on unseeded random numbers (use
  `frameTime()`/`elapsedTime()` and `ivf::automaticSeed()`), so the visual tests stay repeatable.

## Versioning and releases

ivf2 uses [semantic versioning](https://semver.org/): `MAJOR.MINOR.PATCH`, with a pre-release
label such as `-alpha.2` or `-rc.1` before a final release.

- **MAJOR**: incompatible API changes (deprecated aliases are removed only here).
- **MINOR**: new features that keep existing code compiling.
- **PATCH**: bug fixes only.

The version is set in one place, the top-level `CMakeLists.txt` (`project(ivf2 VERSION ...)`
and `IVF2_VERSION_PRERELEASE`); `<ivf/config.h>` and `<ivf/version.h>` are generated from it.

To make a release:

1. Update the version in `CMakeLists.txt` and `vcpkg.json`.
2. In `CHANGELOG.md`, rename `[Unreleased]` to the new version with today's date, add an empty
   `[Unreleased]` section and update the comparison links at the bottom.
3. Commit, wait for CI to pass, then tag the commit `v<version>` (for example
   `git tag -a v2.0.0-alpha.2 -m "ivf2 2.0.0-alpha.2"`) and push the tag. The release workflow
   checks that the tag matches the version in `CMakeLists.txt` and creates the GitHub release
   from the changelog section.

## License

By contributing you agree that your contributions are licensed under the project's
[license](LICENSE).
