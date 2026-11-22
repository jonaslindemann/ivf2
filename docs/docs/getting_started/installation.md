# Installation

This guide will help you install and set up the ivf2 library.

## Prerequisites

- **CMake** 3.24 or higher
- **vcpkg** package manager (handles all library dependencies automatically)
- **Compiler**:
  - Windows: Visual Studio 2022 (17.x) or 2026
  - Linux: GCC 13+ or Clang 17+
  - macOS: Xcode 16+ (Apple Clang)

  ivf2 uses C++23 (`std::expected`); applications using ivf2 are compiled as C++23 too.
- **OpenGL** 3.3 or higher

All library dependencies (GLFW3, ImGui, Assimp, FreeType, stb, nlohmann-json, miniaudio, RtMidi, ReactPhysics3D, etc.) are managed automatically by vcpkg and do not need to be installed manually.

## 1. Clone the Repository

```bash
git clone https://github.com/jonaslindemann/ivf2.git
cd ivf2
```

## 2. Install vcpkg

ivf2 uses vcpkg for dependency management. Instructions for installing vcpkg can be found in the [vcpkg getting started guide](https://learn.microsoft.com/en-us/vcpkg/get_started/overview).

The build system detects vcpkg automatically in the following locations (in order):

1. An explicit `-DCMAKE_TOOLCHAIN_FILE=...`
2. `VCPKG_ROOT` environment variable *(recommended)*
3. Windows: `c:/vcpkg` or `e:/vcpkg`; Linux/macOS: `$HOME/vcpkg`, `/usr/local/vcpkg` or `/opt/vcpkg`
4. The vcpkg bundled with Visual Studio (last resort)

Dependency versions are pinned by the `builtin-baseline` in `vcpkg.json`.

Setting `VCPKG_ROOT` is the most portable approach:

**Windows (PowerShell):**

```powershell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
```

**Linux/macOS:**

```bash
export VCPKG_ROOT=/path/to/vcpkg
```

If vcpkg is in a non-standard location you can also pass the toolchain file directly:

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## 3. Configure and Build

The repository ships `CMakePresets.json`. Each preset builds into `build/<preset>/`.

### Windows (Visual Studio)

```powershell
cmake --preset vs                 # newest installed Visual Studio, x64
cmake --build --preset vs-debug
cmake --build --preset vs-release
ctest --preset vs-debug           # run the unit tests
```

Open `build/vs/ivf2.sln` to work in the IDE.

### Linux / macOS (Ninja)

```bash
cmake --preset release            # or: debug, ninja (multi-config), lib-only, minimal
cmake --build --preset release
ctest --preset release
```

Without presets the usual commands work as well:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

vcpkg will download and compile all required dependencies on the first run. This may take several minutes depending on your internet connection and machine.

### Build options

| Option | Default | Description |
|--------|---------|-------------|
| `IVF2_BUILD_EXAMPLES` | ON (top level) | Build the example programs |
| `IVF2_BUILD_TESTS` | ON (top level) | Build the CTest unit tests |
| `IVF2_VISUAL_TESTS` | OFF | Visual regression tests: render the examples and compare with reference images (`ctest -L visual`) |
| `IVF2_BUILD_MUSIC` | ON | Build `ivfmusic` |
| `IVF_WITH_RENOISE` | ON | Renoise (.xrns) support in ivfmusic |
| `IVF2_WITH_ASSIMP` | ON | Model loading (`ivf::ModelLoader`) with assimp |
| `IVF2_WITH_FREETYPE` | ON | Text rendering (`ivf::TextNode`) with FreeType |
| `IVF2_WITH_AUDIO` | ON | Audio playback/analysis (`ivf::AudioManager`) with miniaudio; required by ivfmusic |
| `IVF2_WITH_MIDI` | ON | MIDI input (`ivf::MidiController`) with RtMidi |
| `IVF2_INSTALL` | ON (top level) | Install rules and CMake package |
| `IVF2_USE_PCH` | ON | Precompiled headers |
| `IVF2_WARNINGS` | ON (top level) | Warnings for ivf2's own code: `/W4` (MSVC), `-Wall -Wextra -Wpedantic` |
| `IVF2_WARNINGS_AS_ERRORS` | OFF | Treat those warnings as errors (`/WX`, `-Werror`) |
| `IVF2_OUTPUT_IN_SOURCE_TREE` | OFF | Legacy layout: binaries in `<source>/bin/<config>` |

The `IVF2_WITH_*` options switch optional dependencies. Each also selects the matching vcpkg
feature, so a disabled library is not downloaded or built. With a feature off, its classes still
exist but log an error and do nothing (`ModelLoader::loadModel` throws). Check what a build
supports with the macros in the generated `<ivf/config.h>` (`IVF2_HAS_ASSIMP`, ...) or with
`find_package(ivf2 REQUIRED COMPONENTS Assimp FreeType Audio Midi Music)`. The `minimal`
preset turns all of them off.

## Build Output

| Artifact     | Location                          |
| ------------ | --------------------------------- |
| Libraries    | `build/<preset>/lib/<Config>/`    |
| Executables  | `build/<preset>/bin/<Config>/`    |

For single-config generators `<Config>` is the `CMAKE_BUILD_TYPE` (e.g. `build/release/bin/Release/`).

Debug library names carry a `d` suffix (e.g. `ivfd.lib` vs `ivf.lib`).

Shaders and fonts from `data/shaders/` and `data/fonts/`, and the example assets from `examples/assets/`, are copied next to the examples by CMake so they work out of the box.

## Integrating with Your Project

All libraries are exported as `ivf2::` targets; `ivf2::ivf2` links everything except `ivf2::ivfmusic`.

**Installed package:**

```bash
cmake --install build/release --prefix /path/to/ivf2-install
```

```cmake
find_package(ivf2 CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ivf2::ivf2)
ivf2_copy_runtime_files(my_app)   # shaders/ and fonts/ next to the executable
```

**FetchContent:**

```cmake
include(FetchContent)
FetchContent_Declare(ivf2 GIT_REPOSITORY https://github.com/jonaslindemann/ivf2.git GIT_TAG main)
FetchContent_MakeAvailable(ivf2)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ivf2::ivf2)
ivf2_copy_runtime_files(my_app)
```

In both cases your project needs the same dependencies available (use the same vcpkg toolchain and add ivf2's dependencies to your own `vcpkg.json`).

See [Your First App](your_first_app.md) for a complete getting-started example.
