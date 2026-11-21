# Diagnostics

ivf2 reports problems through its `Logger`. This page covers OpenGL debug output, redirecting log messages, and running applications unattended.

## OpenGL debug output

With an OpenGL *debug context*, the driver reports errors, undefined behaviour and performance hints as they happen, instead of failing silently. ivf2 routes these messages to the Logger with the context `OpenGL`:

```text
23:02:02.410 ERROR OpenGL           API error (id 1282): GL_INVALID_OPERATION error generated. Render buffer not bound.
```

- **When it is on:** in debug builds by default. Set the environment variable `IVF2_GL_DEBUG=1` to enable it in release builds, or `IVF2_GL_DEBUG=0` to turn it off.
- **Requirements:** OpenGL 4.3. Windows and Linux windows request the newest OpenGL version available; macOS supports at most 4.1, so there is no debug output on macOS.
- **Severity:** driver errors are logged as errors, warnings as warnings; performance hints and notifications are logged at info and debug level.
- **Repeats:** a message that repeats more than a few times is suppressed, so a mistake inside the render loop does not flood the log.
- **Finding the culprit:** messages are delivered synchronously, inside the OpenGL call that caused them. Put a breakpoint on `ivf::logError` and the call stack shows the offending code.

In your own host application (see [Render Context](render_context.md)), enable it after creating a debug context:

```cpp
#include <ivf/gl_debug.h>

ivf::enableGLDebugOutput();
```

### Object labels

ivf2 names its OpenGL objects - shader programs by their name (`"basic"`, `"pbr"`, ...), textures by file name, and its framebuffers (`"ivf scene"`, `"ivf shadow map"`, ...). The names appear in debug messages and in graphics debuggers such as RenderDoc and Nsight. Label your own objects with:

```cpp
ivf::glLabel(GL_TEXTURE, textureId, "terrain height map");
```

## Log format

Each message is one aligned line - time, level, source (context) and message:

```text
23:02:01.912 INFO  GLFWApplication  Initializing GLFW
23:02:02.151 INFO  RenderContext    OpenGL 4.6.0 NVIDIA 621.01 | NVIDIA GeForce RTX 3060/PCIe/SSE2
23:02:02.410 WARN  DirectionalLight Directional light direction cannot be zero.
```

Sources longer than the column are shortened with `~`. The layout can be adjusted:

```cpp
auto log = ivf::Logger::instance();
log->setShowDate(true);         // 2026-11-21 23:02:01.912 instead of the time only
log->setContextWidth(20);       // wider source column (default 16)
log->setWrapColumn(120);        // wrap long messages at column 120 (default 100, 0 = no wrapping)
log->setShowTimestamp(false);   // no time column
log->setLogLevel(ivf::LogLevel::Debug);  // also show debug messages
```

Long messages are wrapped at column 100 by default. Continuation lines - and the lines of multi-line messages such as shader compiler output - are indented to the message column:

```text
23:02:02.151 INFO  RenderContext    OpenGL 4.6.0 NVIDIA 621.01 | NVIDIA GeForce RTX 3060/PCIe/SSE2 |
                                    GLSL 4.60 NVIDIA
```

`Logger::formatMessage()` returns a line exactly as it would be written.

## Redirecting log messages

Register a *sink* to receive every message that passes the log level, for example to show the log in your own ImGui console or to collect messages in tests:

```cpp
#include <ivf/logger.h>

int sinkId = ivf::Logger::instance()->addSink(
    [](ivf::LogLevel level, const std::string &message, const std::string &context) {
        myConsole.add(level, context + ": " + message);
    });

// later
ivf::Logger::instance()->removeSink(sinkId);
```

Sinks receive the raw message without timestamp or formatting, in addition to the console and file output. They may be called from any thread that logs, and may log themselves.

`ivf::logStr()` builds a message from mixed values, like streaming to `std::cout`:

```cpp
ivf::logDebug(ivf::logStr("Picked node ", id, " at (", p.x, ", ", p.y, ")"), "Picker");
```

## Running unattended

These environment variables, read when a window is created, let you run any ivf2 application from a script or CI job:

| Variable | Effect |
|----------|--------|
| `IVF2_HIDE_WINDOWS=1` | Create windows invisible |
| `IVF2_EXIT_AFTER_FRAMES=N` | Close each window after N frames |
| `IVF2_SCREENSHOT=file.png` | With `IVF2_EXIT_AFTER_FRAMES`, save the last frame (including the UI); further windows write `file_2.png`, ... |
| `IVF2_GL_DEBUG=1` / `0` | Force OpenGL debug output on / off |

For example, in PowerShell:

```powershell
$env:IVF2_HIDE_WINDOWS = "1"; $env:IVF2_EXIT_AFTER_FRAMES = "90"; $env:IVF2_SCREENSHOT = "shot.png"
.\lighting1.exe
```
