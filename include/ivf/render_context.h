#pragma once
/**
 * @file render_context.h
 * @brief One OpenGL context and the ivf resources that belong to it.
 */

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <ivf/instancing.h>

#include <cstdint>
#include <memory>
#include <functional>
#include <optional>
#include <span>
#include <vector>
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

    /**
     * @brief Make this the current render context on the calling thread.
     *
     * Call it after the host has made the OpenGL context current: it also runs the
     * work queued with deleteLater().
     */
    void makeCurrent();

    /**
     * @brief Token that expires when this context is destroyed.
     *
     * Objects holding GL resources in several contexts (such as Mesh, with one GPU copy
     * per context) keep it to tell whether a context still exists: GL objects of a
     * destroyed context were freed with it and must not be deleted again.
     */
    [[nodiscard]] std::weak_ptr<const void> lifetime() const;

    /**
     * @brief Run work that needs this OpenGL context (typically deleting GL objects)
     * the next time the context is made current, or when it is destroyed.
     *
     * Use it for objects of this context that are released while another context is
     * current. If this context is current, prefer doing the work right away.
     */
    void deleteLater(std::function<void()> work);

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
     * @brief Draw repeated meshes with one instanced draw call (default on).
     *
     * The Renderer merges opaque items of the same node, mesh, material and textures (for
     * example the copies an InstanceNode makes) into one batch. Off draws every item on
     * its own. The environment variable `IVF2_AUTO_INSTANCING=0` turns it off by default.
     */
    void setAutoInstancing(bool enabled)
    {
        m_autoInstancing = enabled;
    }

    /** @brief See setAutoInstancing(). */
    [[nodiscard]] bool autoInstancing() const
    {
        return m_autoInstancing;
    }

    /**
     * @brief Upload model matrices as InstanceAttributes (white colour, zero data) to the
     *        context's instance buffer and return the buffer.
     *
     * Used by Mesh::drawInstances(); the buffer is reused (and grown) for every batch.
     */
    GLuint uploadInstanceTransforms(std::span<const glm::mat4> transforms);

    /**
     * @brief Running totals of drawing work done in this context.
     *
     * Renderer::stats() reports the difference over one execute() call. The totals only
     * grow; compare two snapshots to measure any stretch of code.
     */
    struct Counters {
        std::uint64_t drawCalls{0};       ///< glDraw* calls by meshes, text, billboards, particles, skybox.
        std::uint64_t programChanges{0};  ///< Program::use() calls that switched to another program.
        std::uint64_t materialApplies{0}; ///< Materials applied to nodes (own, inherited or default).
        std::uint64_t uniformLookups{0};  ///< Uniform locations queried from the driver (cache misses).
        std::uint64_t cameraBlockUploads{0}; ///< Uploads of the IvfCamera uniform buffer (camera changes).
        std::uint64_t lightsBlockUploads{0}; ///< Uploads of the IvfLights uniform buffer (light changes).
        std::uint64_t instances{0};          ///< Instances drawn by instanced draw calls of batches.
    };

    /** @brief Totals since the context was created. */
    [[nodiscard]] const Counters &counters() const
    {
        return m_counters;
    }

    /** @brief Count one draw call (called by the code issuing it). */
    void countDrawCall()
    {
        ++m_counters.drawCalls;
    }

    /** @brief Count a uniform location queried from the driver (called by Program::uniformLoc()). */
    void countUniformLookup()
    {
        ++m_counters.uniformLookups;
    }

    /** @brief Count an upload of the camera uniform block (called by TransformManager). */
    void countCameraBlockUpload()
    {
        ++m_counters.cameraBlockUploads;
    }

    /** @brief Count an upload of the lights uniform block (called by LightManager). */
    void countLightsBlockUpload()
    {
        ++m_counters.lightsBlockUploads;
    }

    /** @brief Count @p count instances drawn with one instanced draw call (Mesh::drawInstances()). */
    void countInstances(std::uint64_t count)
    {
        m_counters.instances += count;
    }

    /** @brief Count a material applied to a node. */
    void countMaterialApply()
    {
        ++m_counters.materialApplies;
    }

    /**
     * @brief Count a program change if @p program differs from the last one counted.
     *
     * Called by Program::use(). Renderer::execute() calls resetProgramTracking() first, as
     * code may bind programs directly with glUseProgram().
     */
    void countProgramUse(GLuint program)
    {
        if (program != m_lastProgram)
        {
            ++m_counters.programChanges;
            m_lastProgram = program;
        }
    }

    /** @brief Set the program countProgramUse() compares with (the one bound right now). */
    void resetProgramTracking(GLuint current)
    {
        m_lastProgram = current;
    }

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
    Counters m_counters;             ///< See counters().
    bool m_autoInstancing{true};     ///< See setAutoInstancing().
    GLuint m_instanceBuffer{0};      ///< See uploadInstanceTransforms().
    std::size_t m_instanceCapacity{0};             ///< Allocated size of m_instanceBuffer, in instances.
    std::vector<InstanceAttributes> m_instanceScratch; ///< Records being uploaded.
    GLuint m_lastProgram{0};         ///< Last program seen by countProgramUse().
    std::shared_ptr<const int> m_lifetime{std::make_shared<const int>(0)}; ///< See lifetime().
    std::vector<std::function<void()>> m_deleteLater;                       ///< See deleteLater().

    void runDeleteLater();
};

/** @brief Shared pointer to a RenderContext. */
using RenderContextPtr = std::shared_ptr<RenderContext>;

} // namespace ivf
