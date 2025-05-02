# Installation

This guide will help you install and set up the ivf2 library for your project.

## Prerequisites

- CMake 3.12+
- C++17 compatible compiler
- OpenGL 3.3+
- GLFW3
- GLEW
- ImGui
- Assimp (for model loading)
- Freetype (for text rendering)

## Dependencies

The following dependencies are required to build ivf2:

- GLFW (window management)
- GLEW (OpenGL extension loading)
- GLM (mathematics)
- ImGui (user interface)
- Assimp (model loading)
- Freetype (font rendering)
- stb_image (image loading)

## Building from Source

1. Clone the repository:
   ``` cmd
   git clone https://github.com/your-username/ivf2.git
   cd ivf2
   ```
1. Create a build directory:
   ``` cmd
   mkdir build
   cd build
   ```
1. Run CMake:
   ``` cmd
   cmake ..
   ```
1. Build the library:
   ``` cmd
   cmake --build .
   ```
1. Install the library (optional):
   ``` cmd
   cmake --install .
   ```

## Integrating with Your Project

### Using CMake

If you're using CMake, you can integrate ivf2 with your project like this:

```cmake
find_package(ivf2 REQUIRED)
target_link_libraries(your_project PRIVATE ivf2::ivf2)
```

### Manual Integration

If you're not using CMake, make sure to:

1. Include the ivf2 headers in your project
2. Link against the ivf2 library
3. Set up the appropriate include paths