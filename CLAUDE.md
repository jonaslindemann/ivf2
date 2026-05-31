# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**ivf2** (Interactive Visualisation Framework 2) is a modern C++20 scene graph-based graphics and visualization library built on OpenGL 3.3+. It is a complete rewrite of the original ivfplusplus library. Documentation: https://interactive-visualisation-framework-2.readthedocs.io/

## Build Commands

**Requirements**: CMake 3.24+, vcpkg (auto-detected from `VCPKG_ROOT`), C++20 compiler.

**Windows (Visual Studio 2022)**:
```
cmake -B build-debug -S . -G "Visual Studio 17 2022"
cmake -B build-release -S . -G "Visual Studio 17 2022"
cmake --build build-debug --config Debug
cmake --build build-release --config Release
```

**Linux/macOS**:
```
cmake -B build-debug -S . -DCMAKE_BUILD_TYPE=Debug
cmake -B build-release -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build-debug
cmake --build build-release
```

Output libraries go to `lib/Debug/` or `lib/Release/`; executables and assets go to `bin/Debug/` or `bin/Release/`. Assets from `bin/shaders/`, `bin/fonts/`, and `bin/assets/` are automatically copied to the output directory by CMake.

## Running Examples

Examples are the primary way to validate functionality. Each example is a standalone executable built alongside the library:

```
bin/Debug/hello_ivf       # Basic primitives and scene setup
bin/Debug/lighting1       # Lighting configuration
bin/Debug/textures1       # Texture mapping
bin/Debug/animation1      # Keyframe animation
bin/Debug/procedural1     # Geometry generation
```

## Code Formatting

Clang Format is configured (`.clang-format`): Microsoft `BasedOnStyle`, 4-space indent, Microsoft-style braces, sorted includes disabled.

## Architecture

### Layer Overview

1. **ivfui** — GLFW window/event management + ImGui integration
2. **ivf** — Core scene graph, rendering, primitives, materials, shaders, effects, animation
3. **generator** — Header-only procedural geometry generation library (~97 generators)
4. **ivfmath** — Keyframe/spline interpolation, spherical math

### Scene Graph

All drawable entities inherit from `Node`. The hierarchy is:

- `Node` — base class (materials, textures, visibility, selection, property inspection)
  - `TransformNode` — adds position/rotation/scale transform
    - Primitives (`Cube`, `Sphere`, `Cylinder`, `Cone`, `Box`, `RoundedBox`, `Capsule`, `Plane`, `Disk`, `Dodecahedron`, `Extrusion`, ...)
    - `MeshNode` — mesh from Assimp-loaded models
    - `TextNode` — text rendering
  - `CompositeNode` — groups child nodes (Composite pattern)

### Manager Singletons

Global state is managed via singleton managers, accessed through static `instance()`:

- `ShaderManager` — GLSL program lifecycle
- `LightManager` — point/directional/spot lights (max defined in shaders)
- `TextureManager` — texture loading and binding
- `FontManager` — FreeType font loading

### Key Patterns

- **Factory methods**: All major classes use `ClassName::create(...)` returning `std::shared_ptr<ClassName>`. Never use raw `new`.
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
    void onInit() override { /* scene setup */ }
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

Effects are applied to a `CompositeNode` via the effects system. Available effects: `BlurEffect`, `BloomEffect`, `ChromaticEffect`, `DitheringEffect`, `FilmGrainEffect`, `PixelationEffect`, `TintEffect`, `VignetteEffect`, `EdgeDetectionEffect`.

### Deformers

Mesh deformation is applied to geometry nodes. Available: `ScaleDeformer`, `TwistDeformer`, `BendDeformer`, `WaveDeformer`, `TurbulenceDeformer`, `RandomDeformer`, `FunctionDeformer`.

### Procedural Geometry (generator)

The `generator` library is header-only and included via `#include <generator/generator.hpp>`. Generators produce meshes, paths, and shapes that can be composed (repeat, transform, scale, rotate, mirror, subdivide). Use `ivf::MeshNode` to consume generator output.

### Physics

ReactPhysics3D integration is in `examples/rigid_body1/`. The framework does not wrap ReactPhysics3D directly — examples show manual integration.

## Dependencies (vcpkg.json)

GLFW3, GLEW, OpenGL, ImGui (docking), Assimp, FreeType, libpng, libjpeg-turbo, stb, nlohmann-json, zlib, ReactPhysics3D, GLM (header-only, bundled in `include/glm/`).

## Shaders

GLSL shaders live in `shaders/` (source) and are copied to `bin/shaders/` at build time. The `ShaderManager` loads them at runtime relative to the executable. When adding a new shader, register it in CMakeLists.txt asset copy rules.

## Documentation

- User guide (Zensical): `docs/docs/`, configured by `docs/zensical.toml`
- API reference (Doxygen): `Doxyfile` at repo root; generated output in `docs/docs/doxygen/html/`
- Build docs: `.readthedocs.yaml` (Python 3.11, doxygen, graphviz, zensical required)
