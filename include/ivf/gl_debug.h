#pragma once
/**
 * @file gl_debug.h
 * @brief OpenGL debug output and object labels.
 *
 * With a debug context, the driver reports errors, undefined behaviour and
 * performance warnings through a callback (OpenGL 4.3 / KHR_debug). ivf routes
 * these messages to the Logger. Object labels give GL objects readable names in
 * those messages and in tools such as RenderDoc or Nsight.
 *
 * ivfui::GLFWWindow enables this automatically in debug builds, or when the
 * environment variable IVF2_GL_DEBUG=1 is set (IVF2_GL_DEBUG=0 disables it).
 * On platforms without OpenGL 4.3 (macOS) everything here is a harmless no-op.
 */

#include <glad/glad.h>

#include <string_view>

namespace ivf {

/** @brief True if the current GL context provides debug output (OpenGL 4.3). */
bool glDebugOutputSupported();

/**
 * @brief Route the current context's debug messages to the Logger.
 *
 * Severity maps to log level: high -> error, medium -> warning, low -> info,
 * notifications -> debug. A message id that repeats more than a few times is
 * suppressed (with a note) so a mistake inside the render loop does not flood the log.
 *
 * @param synchronous Deliver messages on the calling thread, at the offending
 *                    GL call, so a debugger breakpoint in the log shows the culprit.
 * @return false if debug output is not supported by the context.
 */
bool enableGLDebugOutput(bool synchronous = true);

/** @brief Stop debug output for the current context. */
void disableGLDebugOutput();

/**
 * @brief Give a GL object a name for debug messages and graphics debuggers.
 * @param identifier Object type: GL_PROGRAM, GL_SHADER, GL_TEXTURE, GL_FRAMEBUFFER,
 *                   GL_RENDERBUFFER, GL_BUFFER, GL_VERTEX_ARRAY, ...
 * @param name       The GL object name (id). The object must already exist (bound once).
 * @param label      Readable name.
 */
void glLabel(GLenum identifier, GLuint name, std::string_view label);

} // namespace ivf
