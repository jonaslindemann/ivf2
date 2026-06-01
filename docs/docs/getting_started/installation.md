# Installation

This guide will help you install and set up the ivf2 library.

## Prerequisites

- **CMake** 3.24 or higher
- **vcpkg** package manager (handles all library dependencies automatically)
- **Compiler**:
  - Windows: Visual Studio 2022 (MSVC with C++20 support)
  - Linux: GCC or Clang with C++20 support
  - macOS: Xcode with C++20 support
- **OpenGL** 3.3 or higher

All library dependencies (GLFW3, GLEW, ImGui, Assimp, FreeType, stb, GLM, ReactPhysics3D, etc.) are managed automatically by vcpkg and do not need to be installed manually.

## 1. Clone the Repository

```bash
git clone https://github.com/jonaslindemann/ivf2.git
cd ivf2
```

## 2. Install vcpkg

ivf2 uses vcpkg for dependency management. Instructions for installing vcpkg can be found in the [vcpkg getting started guide](https://learn.microsoft.com/en-us/vcpkg/get_started/overview).

The build system detects vcpkg automatically in the following locations (in order):

1. `VCPKG_ROOT` environment variable *(recommended)*
2. Windows: `c:/vcpkg` or `e:/vcpkg`
3. Linux/macOS: `$HOME/vcpkg` or `/usr/local/vcpkg`

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

### Windows

```powershell
# Configure (creates both Debug and Release build trees)
cmake -B build-debug   -S . -G "Visual Studio 17 2022"
cmake -B build-release -S . -G "Visual Studio 17 2022"

# Build
cmake --build build-debug   --config Debug
cmake --build build-release --config Release
```

### Linux / macOS

```bash
# Configure
cmake -B build-debug   -S . -DCMAKE_BUILD_TYPE=Debug
cmake -B build-release -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build-debug
cmake --build build-release
```

vcpkg will download and compile all required dependencies on the first run. This may take several minutes depending on your internet connection and machine.

## Build Output

| Artifact             | Location          |
| -------------------- | ----------------- |
| Libraries (Debug)    | `lib/Debug/`      |
| Libraries (Release)  | `lib/Release/`    |
| Executables (Debug)  | `bin/Debug/`      |
| Executables (Release)| `bin/Release/`    |

On Linux and macOS the libraries and executables land directly in `lib/` and `bin/` without a configuration subdirectory.

Debug library names carry a `d` suffix (e.g. `ivfd.lib` vs `ivf.lib`).

Shaders, fonts, and asset files from `bin/shaders/`, `bin/fonts/`, and `bin/assets/` are automatically copied to the output directory by CMake so examples work out of the box.

## Integrating with Your Project

ivf2 exposes a unified CMake target `ivf2::ivf2` that bundles all libraries and their transitive dependencies:

```cmake
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ivf2::ivf2)
```

See [Your First App](your_first_app.md) for a complete getting-started example.
