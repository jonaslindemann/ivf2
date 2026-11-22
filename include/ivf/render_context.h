#pragma once
/**
 * @file render_context.h
 * @brief One OpenGL context and the ivf resources that belong to it.
 */

#include <glad/glad.h>

#include <memory>
#include <optional>
#include <string>

namespace ivf {

class ShaderManager;
class LightManager;
class TextureManager;
class FontManager;
class MeshManager;
class SelectionManager;
class TransformManager;
class DebugDraw;
class ShaderWatcher;

/**
 * @brief Function that returns the address of an OpenGL function, e.g.
 *        glfwGetProcAddress, SDL_GL_GetProcAddress, eglGetProcAddress or
 *        QOpenGLContext::getProcAddress (cast to this type).
 */
using GLLoadProc = void *(*)(const char *name);

/**
 * @class RenderContext
 * @brief One OpenGL context and the ivf resources bound to it.
 *
 * A RenderContext owns everything in ivf that only exists inside a particular
 * OpenGL context: shader programs, lights (shader uniforms), texture and
 * selection state, the transform stack's uniform bindings, fonts, DebugDraw and
 * ShaderWatcher, plus shared fallback textures.
 *
 * It knows nothing about windows. Whatever owns an OpenGL context creates one -
 * ivfui::GLFWWindow does this automatically; other hosts (Qt, SDL, EGL, ...) do:
 *
 * @code
 * // the host has created its GL context and made it current
 * auto ctx = ivf::RenderContext::create(reinterpret_cast<ivf::GLLoadProc>(glfwGetProcAddress));
 * ctx->makeCurrent();
 * ctx->setupDefaults();          // default shaders, font and light
 * ...
 * ctx->makeCurrent();            // before rendering with this context
 * ...
 * ctx.reset();                   // while the GL context is still current
 * @endcode
 *
 * The managers are created on first use. The classic singleton accessors
 * (ShaderManager::instance(), LightManager::instance(), ...) forward to the
 * context that is current on the calling thread.
 */
class RenderContext {
public:
    /**
     * @brief Create a render context for the OpenGL context that is current on this thread.
     * @param loader Function used to load OpenGL entry points. Pass nullptr if the
     *               host has already loaded them (e.g. called gladLoadGL itself).
     * @return The new context, or nullptr if OpenGL could not be loaded.
     */
    [[nodiscard]] static std::shared_ptr<RenderContext> create(GLLoadProc loader = nullptr);

    /**
     * @brief Release all resources. The owning OpenGL context should be current.
     *
     * If this context is current on the calling thread it stops being current.
     */
    ~RenderContext();

    RenderContext(const RenderContext &) = delete;
    RenderContext &operator=(const RenderContext &) = delete;

    /** @brief Make this the current render context on the calling thread. */
    void makeCurrent();

    /** @brief True if this is the current render context on the calling thread. */
    bool isCurrent() const;

    /** @brief The current render context on the calling thread, or nullptr. */
    static RenderContext *current();

    /** @brief Clear the current render context on the calling thread. */
    static void clearCurrent();

    /**
     * @brief Set up the default rendering state used by GLFWSceneWindow: the
     *        render-to-texture, basic and PBR shaders (basic is applied), the
     *        default font and one directional light.
     * @param fontFile Default font to load (skipped when ivf2 is built without FreeType).
     * @return false if the shaders failed to compile or link.
     */
    bool setupDefaults(const std::string &fontFile = "fonts/Gidole-Regular.ttf");

    /** @name Managers (created on first use) */
    ///@{
    ShaderManager *shaders();
    LightManager *lights();
    TextureManager *textures();
    FontManager *fonts();
    MeshManager *meshes();
    SelectionManager *selection();
    TransformManager *transforms();
    DebugDraw *debugDraw();
    ShaderWatcher *shaderWatcher();
    ///@}

    /** @brief 1x1 white 2D texture bound to unused sampler slots (created on first use). */
    GLuint fallbackTexture2D();

    /** @brief 1x1 black cube map bound to unused cube map slots (created on first use). */
    GLuint fallbackCubemap();

    /**
     * @brief True if glLineWidth() accepts widths above 1.
     *
     * Forward-compatible contexts (required on macOS, used by GLFWWindow) remove wide
     * lines: there a width above 1 is a GL_INVALID_VALUE error and lines stay 1 pixel.
     */
    bool wideLinesSupported();

    /**
     * @brief The current context, or a process-wide fallback context if none is current.
     *
     * Used by the singleton accessors for backwards compatibility. The fallback is
     * created once, with a logged warning; code should normally run while a context
     * owned by a window (or other host) is current.
     */
    static RenderContext *currentOrFallback();

private:
    RenderContext() = default;

    ShaderManager *m_shaders{nullptr};
    LightManager *m_lights{nullptr};
    TextureManager *m_textures{nullptr};
    FontManager *m_fonts{nullptr};
    MeshManager *m_meshes{nullptr};
    SelectionManager *m_selection{nullptr};
    TransformManager *m_transforms{nullptr};
    DebugDraw *m_debugDraw{nullptr};
    ShaderWatcher *m_shaderWatcher{nullptr};

    GLuint m_fallbackTexture2D{0};
    GLuint m_fallbackCubemap{0};
    std::optional<bool> m_wideLines; ///< Queried on first use.
};

/** @brief Shared pointer to a RenderContext. */
using RenderContextPtr = std::shared_ptr<RenderContext>;

} // namespace ivf
