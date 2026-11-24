# Render Context

An `ivf::RenderContext` is one OpenGL context together with the ivf resources that belong to it: shader programs, lights, texture, selection and transform state, fonts, `DebugDraw` and `ShaderWatcher`. Each window has its own, so several windows can be open at once, each with its own scene, lights and user interface.

## Using it in a window

Windows create their render context automatically. Access the managers through it:

```cpp
int onSetup() override
{
    auto lights = renderContext()->lights();
    auto sun = lights->addDirectionalLight();
    sun->setDirection({-0.5f, -1.0f, -0.4f});
    sun->setEnabled(true);
    lights->apply();

    auto prog = renderContext()->shaders()->program("basic");
    return 0;
}
```

| Accessor | Manager |
|----------|---------|
| `shaders()` | `ShaderManager` |
| `lights()` | `LightManager` |
| `textures()` | `TextureManager` |
| `fonts()` | `FontManager` |
| `meshes()` | `MeshManager` |
| `selection()` | `SelectionManager` |
| `transforms()` | `TransformManager` |
| `debugDraw()` | `DebugDraw` |
| `shaderWatcher()` | `ShaderWatcher` |

Managers are created on first use and released when the window closes, while its OpenGL context still exists.

!!! note "Existing code keeps working"
    The classic accessors `LightManager::instance()`, `ShaderManager::create()`, ... still work. They return the manager of the *current* render context. A window makes its context current before it calls `onSetup()`, `onUpdate()`, `onDraw()`, the UI callbacks and the input callbacks, so inside a window both forms return the same object. New code should prefer `renderContext()->...`.

## Several windows

Add more than one window to the application. Each has its own render context, ImGui context and scene:

```cpp
auto a = SceneWindow::create(800, 600, "Window A");
auto b = SceneWindow::create(800, 600, "Window B");
app->addWindow(a);
app->addWindow(b);
return app->loop();
```

Changing the lights in one window does not affect the other. See the `multi_window1` example.

## Using ivf in another application

The render context does not depend on GLFW or `ivfui`. Any application that owns an OpenGL context (Qt `QOpenGLWidget`, SDL, a game engine, an offscreen EGL context, ...) can host ivf:

```cpp
// 1. The host has created its OpenGL context and made it current.

// 2. One render context per OpenGL context; pass the host's GL function loader
//    (glfwGetProcAddress, SDL_GL_GetProcAddress, QOpenGLContext::getProcAddress, ...).
auto ctx = ivf::RenderContext::create(reinterpret_cast<ivf::GLLoadProc>(glfwGetProcAddress));
ctx->makeCurrent();
ctx->setupDefaults();            // default shaders, font and one light

// 3. Build a scene and render it in the host's render loop.
auto scene = ivf::CompositeNode::create();
// ... add nodes ...

// each frame:
ctx->makeCurrent();              // with several GL contexts, make the matching one current
auto xfm = ctx->transforms();
xfm->enableProjectionMatrix(); xfm->identity(); xfm->perspective(45.0f, aspect, 0.1f, 100.0f);
xfm->enableViewMatrix();       xfm->identity(); xfm->lookAt(eye, target);
xfm->enableModelMatrix();      xfm->identity();
scene->draw(*ctx);               // or collect and run the passes yourself (Render Queue page)

// 4. On shutdown, release ivf objects while the GL context is still current.
scene.reset();
ctx.reset();
```

Pass `nullptr` to `create()` if the host has already loaded the OpenGL functions with glad. See the `embedding1` example, which hosts ivf in a plain GLFW window without `ivfui`.

## Details

- **Current context.** `RenderContext::current()` returns the context made current on the calling thread with `makeCurrent()`. It is per thread, like OpenGL's own current context.
- **No current context.** If a manager is requested while no context is current, a process-wide fallback context is created once and a warning is logged. This keeps old code working but should be avoided.
- **Creating nodes.** Meshes are uploaded to the GPU when they are first drawn, once per context that draws them, so geometry nodes can be created before any window exists and shown in several windows. Textures, fonts and shader materials still need a current context when they are created - create them in `onSetup()` or later.
- **Explicit context.** The drawing code takes the render context as a parameter instead of looking up the current one: `Renderer` (constructed with a context), `LightManager::renderShadowMaps(context, queue)`, `PlanarReflection::render(context, queue)`, `Node::draw(context)` and `Mesh::draw(context)`. Nodes that override `doDrawPass()` get it as `DrawContext::context`. Materials and the other node hooks still use the current context, so make the context current before drawing (see [Render Queue](render_queue.md)).
- **Fallback textures.** The 1×1 textures bound to unused material slots (`fallbackTexture2D()`, `fallbackCubemap()`) are also per context.
