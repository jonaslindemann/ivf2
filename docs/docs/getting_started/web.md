# Running in the Browser

ivf2 programs also run in web browsers. The same C++ code is compiled to WebAssembly with [Emscripten](https://emscripten.org) and renders with WebGL 2, the browser's version of OpenGL ES 3.0. There is no separate renderer: scene graph, materials, shadows, PBR, effects, ImGui panels and `ivfvis` work as on the desktop.

[Open the examples in your browser](https://jonaslindemann.github.io/ivf2/web/){ .md-button .md-button--primary }

Every example except five runs there, built and tested by CI from the current sources. The pages need a browser with WebGL 2: current Chrome, Edge, Firefox and Safari all have it. Drag to rotate the camera as on the desktop.

## What is different in the browser

- **One window per page.** The window is the page's canvas, and it follows the size of its parent element. `multi_window1` and `embedding1` are not built for the web.
- **No Assimp, audio, MIDI or `ivfmusic`.** Load models as glTF (`GltfModel`); `assimp1`, `audio_reactive1` and `music_scene1` are not built for the web.
- **Files are packed into the page.** A program reads its shaders, fonts and assets from a virtual file system that is downloaded with the page (the `.data` file), using the same relative paths as on the desktop. Files the program writes (screenshots, `imgui.ini`, hot-reload edits) stay in the page's memory.
- **No threads.** The pages need no special HTTP headers, so any static web host can serve them.
- **High-DPI screens.** The canvas has device pixels, while window and mouse coordinates are in CSS pixels, as on a macOS Retina display. `RenderContext::pixelRatio()` is the number of framebuffer pixels per window unit; `OverlayNode` lays out in window units.
- **Settings from the URL.** Query parameters named `IVF2_*` become environment variables before `main()` runs, so the switches from [Diagnostics](../advanced/diagnostics.md) work in the browser too: `vis_points1.html?IVF2_RANDOM_SEED=1&IVF2_RENDER_STATS=60`. A parameter without a value is set to `1`.

## Building the examples

You need the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) (ivf2 is built and tested with version 3.1.70), Ninja and vcpkg (see [Installation](installation.md)).

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install 3.1.70
./emsdk activate 3.1.70
source ./emsdk_env.sh        # Windows: emsdk_env.bat (or emsdk_env.ps1)
```

`emsdk_env` sets `EMSDK` and puts `emcc` and Node.js on the path for the current shell. Then, in the ivf2 directory:

```bash
cmake --preset wasm
cmake --build --preset wasm
```

The first configure builds the dependencies for the triplet `wasm32-emscripten` with vcpkg, which takes a few minutes. Each example becomes a page in `build/wasm/bin/`: `<name>.html` with `.js`, `.wasm` and `.data` files.

Browsers do not run WebAssembly from `file://` URLs, so serve the directory over HTTP and open a page:

```bash
python -m http.server -d build/wasm/bin 8000
# http://localhost:8000/hello_ivf.html
```

## Your own application

### Inside the ivf2 source tree

The quickest way is a new example: a directory in `examples/` added in `examples/CMakeLists.txt`, with a `CMakeLists.txt` that calls `add_ivf2_example()`. It builds for the desktop and the browser. List the files the page has to load, relative to `examples/assets/`:

```cmake
add_ivf2_example(my_app my_app.cpp WEB_ASSETS textures/brick.png models/showcase.glb)
```

`WEB_ASSETS` accepts directories too. Load only what the page needs: the whole data file is downloaded before the program starts.

### A separate project

An application with its own CMake project uses an installed web build of ivf2, like the desktop package. Build and install ivf2 with the `wasm` preset:

```bash
cmake --preset wasm -DIVF2_INSTALL=ON
cmake --build --preset wasm
cmake --install build/wasm --prefix $HOME/ivf2-web
```

The application's `CMakeLists.txt` is the same as for the desktop, plus one call to `ivf2_web_executable()`, which does nothing in desktop builds:

```cmake
cmake_minimum_required(VERSION 3.24)
project(my_app LANGUAGES CXX)

find_package(ivf2 CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ivf2::ivf2)
ivf2_copy_runtime_files(my_app)                                   # desktop: shaders/, fonts/
ivf2_web_executable(my_app ASSETS_ROOT ${CMAKE_SOURCE_DIR}/assets  # browser: my_app.html
                    ASSETS textures/wood.png)
```

`ivf2_web_executable()` builds the target as a page and packs ivf2's `shaders/` and `fonts/` into its data file, plus the `ASSETS` (paths relative to `ASSETS_ROOT` keep their place below the root's name: `assets/textures/wood.png`). `SHELL <file>` replaces the page template. The program itself needs nothing special:

```cpp
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/glfw_application.h>
#include <ivfui/glfw_scene_window.h>

class WebAppWindow : public ivfui::GLFWSceneWindow {
public:
    using GLFWSceneWindow::GLFWSceneWindow;

    int onSetup() override
    {
        auto material = ivf::Material::create();
        material->setDiffuseColor(glm::vec4(0.9f, 0.6f, 0.2f, 1.0f));

        auto sphere = ivf::Sphere::create(0.8);
        sphere->setMaterial(material);
        sphere->setPos(glm::vec3(-1.2f, 0.0f, 0.0f));
        add(sphere);

        auto cube = ivf::Cube::create();
        cube->setMaterial(material);
        cube->setPos(glm::vec3(1.2f, 0.0f, 0.0f));
        add(cube);

        enableAxis();
        cameraManipulator()->setCameraPosition(glm::vec3(0.0f, 2.0f, 6.0f));
        return 0;
    }
};

int main()
{
    auto app = ivfui::GLFWApplication::create();
    auto window = std::make_shared<WebAppWindow>(1280, 720, "ivf2 in the browser");
    app->addWindow(window);
    return app->loop();
}
```

In the browser, `GLFWApplication::loop()` hands the frames to the page (`emscripten_set_main_loop`) and does not return: `main()` is left without running destructors, so the application and its windows live on with the page. Code after `loop()` does not run there.

Configure the application with vcpkg's toolchain, chained to Emscripten's, and the dependencies ivf2 was built with:

```bash
cmake -S my_app -B my_app/build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
  -DVCPKG_TARGET_TRIPLET=wasm32-emscripten \
  -DVCPKG_MANIFEST_MODE=OFF -DVCPKG_INSTALLED_DIR=<ivf2>/build/wasm/vcpkg_installed \
  -Divf2_DIR=$HOME/ivf2-web/lib/cmake/ivf2
cmake --build my_app/build
```

`ivf2_DIR` points at the package directly: Emscripten's toolchain searches packages only inside its own system root. This is the program and setup CI tests (`tests/package/web_app.cpp`).

### Hosting

A page is four static files (`.html`, `.js`, `.wasm`, `.data`); copy them to any web server. The server should send `.wasm` files as `application/wasm` (most do), and the files must be served over HTTP(S), not opened from disk. The page template (`cmake/ivf2_web_shell.html`) fills the browser window; the canvas takes the size of its parent element `#viewport`, so a page of your own (`SHELL`) can place it anywhere, for example in a fixed-size box in an article.

## Writing code that runs in both

The library hides most of the differences. Your own code should follow a few rules (the examples do):

- **Shaders stay desktop GLSL** (`#version 330 core`). On OpenGL ES, `Shader::compile()` translates them to GLSL ES 3.00 (version line, precisions, uniform initialisers). Index sampler arrays only with constants and don't use `layout(binding = ...)`.
- **Desktop-only GL calls** (`glPolygonMode`, `glDrawBuffer`, border clamping, timer queries, debug output) don't exist in WebGL: calling one stops the program with a "null function" error. Check `RenderContext::isGles()` first.
- **Sized texture formats.** OpenGL ES has no unsized `GL_RED`/`GL_RG` internal formats; use `GL_R8`/`GL_RG8`.
- **No feedback loops.** WebGL refuses to draw into a texture that a sampler of the program can see, even if the shader doesn't read it. Unbind render-target textures from texture units after using them.
- **Don't block.** `onDraw()` runs once per browser frame; long loops freeze the page.

## Testing

`tests/web/smoke_test.mjs` opens pages in headless Chrome, with WebGL on the CPU (SwiftShader), and checks that each one starts, keeps rendering and logs no errors. It needs Node.js 22 or later; the emsdk's Node is fine:

```bash
node tests/web/smoke_test.mjs build/wasm/bin                 # every page
node tests/web/smoke_test.mjs build/wasm/bin hello_ivf ibl1  # some pages
node tests/web/smoke_test.mjs build/wasm/bin --screenshots shots
```

`shader_check.html` (`tests/web`) compiles every built-in and file shader with the browser's WebGL 2 compiler and reports failures in its title. CI builds the web examples for pull requests and releases (`.github/workflows/web.yml`) and publishes them with this documentation.
