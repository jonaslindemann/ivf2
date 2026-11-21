#include <ivf/gl_debug.h>

#include <ivf/logger.h>

#include <algorithm>
#include <format>
#include <mutex>
#include <string>
#include <unordered_map>

namespace {

constexpr int kMaxRepeats = 5; // reports of one message id before it is suppressed

const char *sourceName(GLenum source)
{
    switch (source)
    {
    case GL_DEBUG_SOURCE_API: return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "window system";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "third party";
    case GL_DEBUG_SOURCE_APPLICATION: return "application";
    default: return "other";
    }
}

const char *typeName(GLenum type)
{
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR: return "error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "undefined behaviour";
    case GL_DEBUG_TYPE_PORTABILITY: return "portability";
    case GL_DEBUG_TYPE_PERFORMANCE: return "performance";
    case GL_DEBUG_TYPE_MARKER: return "marker";
    default: return "other";
    }
}

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                              const GLchar *message, const void * /*userParam*/)
{
    if (type == GL_DEBUG_TYPE_PUSH_GROUP || type == GL_DEBUG_TYPE_POP_GROUP)
        return;

    // Count repeats per (source, type, id); the callback may run on driver threads.
    static std::mutex mutex;
    static std::unordered_map<unsigned long long, int> counts;
    int count = 0;
    {
        std::lock_guard<std::mutex> lock(mutex);
        const auto key = (static_cast<unsigned long long>(source) << 48) ^
                         (static_cast<unsigned long long>(type) << 32) ^ id;
        count = ++counts[key];
    }
    if (count > kMaxRepeats + 1)
        return;

    std::string text = length > 0 ? std::string(message, static_cast<size_t>(length)) : std::string(message);
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
        text.pop_back();

    std::string line = std::format("{} {} (id {}): {}", sourceName(source), typeName(type), id, text);
    if (count == kMaxRepeats + 1)
        line = std::format("{} {} (id {}) repeated - further messages suppressed", sourceName(source),
                           typeName(type), id);

    // Performance messages are hints (e.g. NVIDIA's shader-recompile notice), not errors.
    if (type == GL_DEBUG_TYPE_PERFORMANCE && (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM))
        severity = GL_DEBUG_SEVERITY_LOW;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: ivf::logError(line, "OpenGL"); break;
    case GL_DEBUG_SEVERITY_MEDIUM: ivf::logWarning(line, "OpenGL"); break;
    case GL_DEBUG_SEVERITY_LOW: ivf::logInfo(line, "OpenGL"); break;
    default: ivf::logDebug(line, "OpenGL"); break; // GL_DEBUG_SEVERITY_NOTIFICATION
    }
}

} // namespace

namespace ivf {

bool glDebugOutputSupported()
{
    // glad loads the OpenGL 4.3 entry points only when the context provides them.
    return glDebugMessageCallback != nullptr && glDebugMessageControl != nullptr;
}

bool enableGLDebugOutput(bool synchronous)
{
    if (!glDebugOutputSupported())
        return false;

    glEnable(GL_DEBUG_OUTPUT);
    if (synchronous)
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    else
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(debugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    logInfo(std::string("OpenGL debug output enabled") +
                ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) ? "" : " (context is not a debug context - fewer messages)"),
            "OpenGL");
    return true;
}

void disableGLDebugOutput()
{
    if (!glDebugOutputSupported())
        return;
    glDebugMessageCallback(nullptr, nullptr);
    glDisable(GL_DEBUG_OUTPUT);
}

void glLabel(GLenum identifier, GLuint name, std::string_view label)
{
    if (glObjectLabel == nullptr || name == 0 || label.empty())
        return;

    GLint maxLength = 256;
    glGetIntegerv(GL_MAX_LABEL_LENGTH, &maxLength);
    const auto length = static_cast<GLsizei>(std::min<size_t>(label.size(), static_cast<size_t>(maxLength) - 1));
    glObjectLabel(identifier, name, length, label.data());
}

} // namespace ivf
