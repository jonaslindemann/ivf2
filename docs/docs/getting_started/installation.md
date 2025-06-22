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

## Install vcpkg

Ivf2 uses vcpkg to manage its dependencies. Instruction on how to install vcpkg can be found [here](https://learn.microsoft.com/sv-se/vcpkg/get_started/overview)

## Clone the Repository

To get started, clone the ivf2 repository from GitHub:

``` bash
git clone https://github.com/jonaslindemann/ivf2.git
cd ivf2
```

## Configuring the Library

To build the ivf2 library, follow these steps:

In the ivf2 directory, run the following commands:

```cmd
python build-ivf.py --configure --build-type=Debug --preset=default
```

This command will check for the vcpkg.cmake toolchain file and configure the build system accordingly. It will also build the library in Debug mode.
You can change the `--build-type` to `Release` for a release build.

```cmd

--- Setting up toolchain file...

Enter the path to the toolchain file (default: c:/vcpkg/scripts/buildsystems/vcpkg.cmake):
```

Press Enter to use the default path or provide a custom path to the vcpkg toolchain file.
This will set up the vcpkg toolchain file for the build process. Next, the required package dependencies will be installed. This may take some time depending on your internet connection and system performance.

When the configuration is complete, you will see a message indicating that the dependencies have been successfully installed.

```cmd
-- Configuring done (20.8s)
-- Generating done (0.5s)
-- Build files have been written to: E:/Users/Jonas/Development/ivf2/build-debug
```

## Building the Library
After configuring the library, you can build it using the following command:

```bash
cmake --build build-debug --config Debug
```

or

```bash
cmake --build build-release --config Release
```
This will compile the library and create the necessary binaries. The output will be located in the `build-debug` or `build-release` directory, depending on the build type you chose.

When the build is complete, you will see a message indicating that the build was successful (MSBUILD).

```cmd
  example_window_1.cpp
  example_window_2.cpp
  Generating Code...
  ui1d.vcxproj -> E:\Users\Jonas\Development\ivf2\bin\Debug\ui1d.exe
  Building Custom Rule E:/Users/Jonas/Development/ivf2/CMakeLists.txt
```

On Windows executables will be located in the `bin/Debug` or `bin/Release` directory, depending on the build type you chose. On Linux, the executables will be located in the `bin` directory.
On macOS, the executables will be located in the `bin` directory as well.

Libraries will be located in the `lib/Debug` or `lib/Release` directory on Windows and `lib` on Linux and mac. Header files will be in the `include` directory.

## Integrating with Your Project

To integrate ivf2 into your project, follow these steps:

1. Include the ivf2 headers in your project.
2. Link against the ivf2 library.
3. Set up the appropriate include paths.