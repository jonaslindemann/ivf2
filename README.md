# Interactive Visualisation Framework - Ivf++ 2

Lightweight scene graph library and wrapper over modern OpenGL. 

![alt text](docs/docs/images/logo.png "Logo")

## Features

- **Modern OpenGL wrapper** with scene graph architecture
- **Simplified build system** with automatic vcpkg detection
- **Unified library target** (`ivf2::ivf2`) for easy integration
- **Automatic asset management** - shaders, fonts, and models copied automatically
- **Cross-platform** - Windows, Linux, and macOS support
- **Rich examples** demonstrating lighting, animations, physics, UI, and more

# Documentation

Documentation for Ivf++ can be found at https://interactive-visualisation-framework-2.readthedocs.io/en/latest/. The documentation is generated from the source code and is available in the **docs** folder.

# Getting Started

## 1. Clone the repository

```bash
git clone https://github.com/jonaslindemann/ivf2.git
cd ivf2
```

## 2. Install vcpkg

Ivf++ uses vcpkg for dependency management. Installation instructions for your platform can be found here:

https://learn.microsoft.com/sv-se/vcpkg/get_started/overview

The build system automatically detects vcpkg - no manual configuration needed!

## 3. Build the library

See the [Building ivf2](#building-ivf2) section below for detailed instructions.

# Building ivf2

## Prerequisites

1. **CMake** 3.24 or higher
2. **vcpkg** package manager
3. **Compiler**: 
   - Windows: Visual Studio 2022 (or compatible MSVC compiler)
   - Linux: GCC or Clang with C++20 support
   - macOS: Xcode with C++20 support

## Vcpkg Setup

The build system automatically detects vcpkg in these locations:

1. **VCPKG_ROOT environment variable** (recommended)
2. **Windows**: `c:/vcpkg` or `e:/vcpkg`
3. **Linux/macOS**: `$HOME/vcpkg` or `/usr/local/vcpkg`

### Setting VCPKG_ROOT (Recommended)

**Windows (PowerShell):**
```powershell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
```

**Linux/macOS:**
```bash
export VCPKG_ROOT=/path/to/vcpkg
```

If vcpkg is in a custom location, you can also use:
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Quick Start

### Windows

```powershell
# Configure
cmake -B build-debug -S . -G "Visual Studio 17 2022"
cmake -B build-release -S . -G "Visual Studio 17 2022"

# Build Debug
cmake --build build-debug --config Debug

# Build Release
cmake --build build-release --config Release
```

### Linux/macOS

```bash
# Configure Debug
cmake -B build-debug -S . -DCMAKE_BUILD_TYPE=Debug

# Configure Release
cmake -B build-release -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build-debug
cmake --build build-release
```

## Build Output

- **Libraries**: Located in `lib/Debug/` and `lib/Release/`
  - `ivf[d].lib` - Core visualization framework
  - `ivfui[d].lib` - UI components
  - `ivfmath[d].lib` - Math utilities
  - `generator[d].lib` - Geometry generation
  - `glad[d].lib` - OpenGL loader
  - *(Note: 'd' suffix indicates debug builds)*

- **Examples**: Located in `bin/Debug/` and `bin/Release/`
  - All examples are automatically built with the library
  - Assets (shaders, fonts, models) are automatically copied to output directories

# Using ivf2 in your own projects

The build system provides a unified `ivf2::ivf2` target that bundles all ivf libraries and their dependencies for easy integration.

## In CMake projects

```cmake
# Simple example
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ivf2::ivf2)
```

## Creating examples

Use the `add_ivf2_example` helper function in the examples directory:

```cmake
# Simple example with default source file (example_name.cpp)
add_ivf2_example(my_example)

# With explicit source files
add_ivf2_example(my_example SOURCES main.cpp utils.cpp)

# With additional libraries (e.g., for physics examples)
add_ivf2_example(physics_demo SOURCES demo.cpp EXTRA_LIBS ReactPhysics3D::ReactPhysics3D)
```

This eliminates the need to manually list all dependencies (`ivfui`, `ivf`, `ivfmath`, `generator`, `glad`, `imgui::imgui`, etc.) for each project.

### Automatic Asset Management

The `add_ivf2_example` helper automatically copies shared resources to the executable directory after each build:
- **Shaders** (from `bin/shaders/`) - GLSL shader files
- **Fonts** (from `bin/fonts/`) - TrueType fonts for text rendering  
- **Assets** (from `bin/assets/`) - Models, textures, and other resources

This ensures all examples have immediate access to required runtime assets without manual copying.

# Troubleshooting

## vcpkg not found

If CMake can't find vcpkg, set the `VCPKG_ROOT` environment variable:

**Windows:**
```powershell
# PowerShell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"

# CMD
set VCPKG_ROOT=C:\path\to\vcpkg
```

**Linux/macOS:**
```bash
export VCPKG_ROOT=/path/to/vcpkg
```

Or specify it directly when configuring:
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Build fails with "cannot find library"

Make sure you're using the correct compiler:
- **Windows**: Use Visual Studio 2022 generator: `-G "Visual Studio 17 2022"`
- **Linux/macOS**: Ensure your compiler supports C++20

## Examples can't find shaders or assets

The `add_ivf2_example` helper automatically copies assets. If you created an example manually, make sure it's using the helper function or manually copy the `bin/shaders`, `bin/fonts`, and `bin/assets` directories to your executable's output directory.

## Library naming (Debug vs Release)

- Debug builds automatically append 'd' to library names: `ivfd.lib`, `generatord.lib`, etc.
- Release builds use the base names: `ivf.lib`, `generator.lib`, etc.
- The unified `ivf2::ivf2` target handles this automatically - no need to specify different libraries for different configurations.
