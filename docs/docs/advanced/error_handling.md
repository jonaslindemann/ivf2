# Error Handling

Functions that can fail for reasons outside your program's control - a missing file, an unsupported format, a shader that does not compile - return an `ivf::Result<T>`. A result holds either the value or an `ivf::Error` that says what went wrong. It is C++23's `std::expected<T, ivf::Error>`.

```cpp
#include <ivf/result.h>
```

## Using a result

```cpp
auto texture = ivf::Texture::fromFile("assets/brick.png");
if (!texture)
{
    showMessage(texture.error().message);   // "Could not load texture 'assets/brick.png': file not found"
    return -1;
}
node->setTexture(*texture);                  // * gives the value
```

Common forms:

```cpp
// Fall back to a default value
node->setTexture(ivf::Texture::fromFile("brick.png").value_or(defaultTexture));

// Functions without a value return Result<void>, usable directly as a condition
if (!renderContext()->fonts()->loadFace("fonts/Roboto.ttf", "roboto"))
    return -1;

// React to specific failures
auto model = ivf::ModelLoader::load(path);
if (!model && model.error().code == ivf::ErrorCode::FileNotFound)
    askUserForAnotherFile();
```

- `*result` on a failed result is undefined behaviour - check first, or use `value_or()`.
- `result.value()` throws `std::bad_expected_access` on a failed result.
- Ignoring a result produces a compiler warning (`std::expected` is `[[nodiscard]]`). If ignoring it is intended, write `static_cast<void>(...)` and say why in a comment.

## The error

```cpp
struct Error {
    ErrorCode code;        // FileNotFound, ReadFailed, WriteFailed, UnsupportedFormat, InvalidData,
                           // CompileFailed, LinkFailed, InvalidArgument, NotSupported, Unknown
    std::string message;   // readable, includes the file name
};
```

`ivf::toString(code)` gives a readable name for a code.

## Policy

- **Expected failures return a `Result`.** Missing or unreadable files, unsupported formats, compile and link errors, features not available in this build.
- **Errors are also logged.** A failure is visible in the log even when the caller does not check the result, so existing code that ignores failures still reports them.
- **Exceptions are for programming errors only**, such as passing a required pointer as `nullptr`.

In your own code, `ivf::makeError()` logs and returns an error in one step:

```cpp
ivf::Result<Heightmap> loadHeightmap(const std::string &path)
{
    if (!std::filesystem::exists(path))
        return ivf::makeError(ivf::ErrorCode::FileNotFound, "Heightmap '" + path + "' not found", "Terrain");
    ...
    return heightmap;
}
```

`ivf::silentError()` creates an error without logging it, for errors the caller reports itself.

## Functions returning a Result

| Function | Returns |
|----------|---------|
| `Texture::fromFile(path)` | `Result<TexturePtr>` |
| `Texture::load(path)`, `Texture::loadHDR(path)` | `Result<void>` |
| `Cubemap::load(...)`, `Cubemap::loadFromDirectory(dir)` | `Result<void>` |
| `FontManager::loadFace(file, name)` | `Result<void>` |
| `ModelLoader::load(path)` | `Result<CompositeNodePtr>` |
| `ShaderManager::compileProgramFromFiles(vert, frag, name)` | `Result<ProgramPtr>` |
| `ShaderManager::compileProgramFromStrings(vert, frag, name)` | `Result<ProgramPtr>` |
| `ShaderManager::loadBasicShader()`, `loadPBRShader()`, `loadBumpShader()`, `loadRenderToTextureShader()` | `Result<ProgramPtr>` |
| `SceneSerializer::loadFile(path)` | `Result<CompositeNodePtr>` |
| `SceneSerializer::saveFile(root, path)` | `Result<void>` |

`Texture::load`, `Cubemap::load` and `FontManager::loadFace` used to return `bool` or `void`; code that uses them in a condition, `if (texture->load(...))`, compiles unchanged.

The `load*Shader()` functions used to return the `ProgramPtr` directly. Code that ignores the return value compiles unchanged (with a warning); code that uses it now needs `*`: `(*shaderMgr->loadBumpShader())->use()`.

### Shader programs

For compile and link failures the error message ends with the driver's output, so it can be shown in a UI, for example while editing a shader:

```cpp
auto program = shaderMgr->compileProgramFromFiles("shaders/wave.vert", "shaders/wave.frag", "wave");
if (!program)
{
    m_status = program.error().message; // "Shader program 'wave': fragment shader failed to compile:\n0(12) : error ..."
    return;
}
material->setProgram(*program);
```

A failed program is not registered with the shader manager and does not replace an earlier program with the same name. Effects build their programs with `Effect::compileProgram()`, which returns `nullptr` on failure; `GLFWSceneWindow::addEffect()` skips such an effect.

### Deprecated

These still work but are marked `[[deprecated]]` and will be removed in a later release:

| Deprecated | Use instead |
|------------|-------------|
| `ModelLoader::loadModel(path)` - throws on failure | `ModelLoader::load(path)` |
| `SceneSerializer::load(path)` - `nullptr` on failure | `SceneSerializer::loadFile(path)` |
| `SceneSerializer::save(root, path)` - `false` on failure | `SceneSerializer::saveFile(root, path)` |
| `ShaderManager::loadProgramFromFiles(...)` - `nullptr` on failure | `ShaderManager::compileProgramFromFiles(...)` |
| `ShaderManager::loadProgramFromStrings(...)` - `nullptr` on failure | `ShaderManager::compileProgramFromStrings(...)` |
| `ShaderManager::compileLinkErrors()` - flag for the last program | the `Result` of the function that built the program |
| `smLoadProgramFromFiles`, `smLoadProgramFromStrings`, `smCompileLinkErrors` | the `ShaderManager` functions above |

The deprecated `loadProgramFrom*()` functions used to return an unusable program when compiling failed; they now return `nullptr`.
