#include <ivf/render_context.h>

#include <ivf/config.h>
#include <ivf/debug_draw.h>
#include <ivf/font_manager.h>
#include <ivf/light_manager.h>
#include <ivf/logger.h>
#include <ivf/mesh_manager.h>
#include <ivf/selection_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/resource_watcher.h>
#include <ivf/shader_watcher.h>
#include <ivf/texture_manager.h>
#include <ivf/transform_manager.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>

using namespace ivf;

namespace {

thread_local RenderContext *t_current = nullptr;

// Created on demand by currentOrFallback() for code that uses the singleton
// accessors without any current context. Intentionally never destroyed: at
// program exit no GL context is current, so releasing its GL objects then would
// be invalid (this matches the old singletons, which were never deleted either).
RenderContext *g_fallback = nullptr;

// OpenGL ES 3 loaded (one kind of GL per process: the browser's WebGL 2, or desktop GL)
bool g_gles = false;

// glad's desktop loader reads the version from glGetString(GL_VERSION) and loads only the
// functions of that version. OpenGL ES 3.0 (WebGL 2) reports "OpenGL ES 3.0", so glad would
// skip the 3.1-3.3 functions ivf2 uses (uniform blocks, instancing, samplers), although ES 3
// has them. While loading, the shim below reports 3.3; functions ES lacks stay null.
using GetStringProc = const GLubyte *(*)(GLenum);
GLLoadProc g_loader = nullptr;
GetStringProc g_getString = nullptr;

const GLubyte *esVersionShim(GLenum name)
{
    if (name == GL_VERSION)
        return reinterpret_cast<const GLubyte *>("3.3.0 (OpenGL ES 3.0 via ivf2 loader shim)");
    return g_getString(name);
}

void *esLoaderShim(const char *name)
{
    if (std::strcmp(name, "glGetString") == 0)
        return reinterpret_cast<void *>(&esVersionShim);
    return g_loader(name);
}

} // namespace

bool RenderContext::isGles()
{
    return g_gles;
}

std::shared_ptr<RenderContext> RenderContext::create(GLLoadProc loader)
{
    if (loader != nullptr)
    {
        g_loader = loader;
        g_getString = reinterpret_cast<GetStringProc>(loader("glGetString"));
        const auto *version = g_getString ? reinterpret_cast<const char *>(g_getString(GL_VERSION)) : nullptr;
        g_gles = version != nullptr && std::strncmp(version, "OpenGL ES", 9) == 0;

        const int loaded = g_gles ? gladLoadGLLoader(reinterpret_cast<GLADloadproc>(&esLoaderShim))
                                  : gladLoadGLLoader(reinterpret_cast<GLADloadproc>(loader));
        if (g_gles && g_getString != nullptr)
            glad_glGetString = reinterpret_cast<PFNGLGETSTRINGPROC>(g_getString); // the real version again
        if (!loaded)
        {
            logError("Failed to load OpenGL functions", "RenderContext");
            return nullptr;
        }

        auto str = [](GLenum name) {
            const auto *s = reinterpret_cast<const char *>(glGetString(name));
            return std::string(s ? s : "?");
        };
        logInfo("OpenGL " + str(GL_VERSION) + " | " + str(GL_RENDERER) + " | GLSL " +
                    str(GL_SHADING_LANGUAGE_VERSION),
                "RenderContext");
    }

    auto context = std::shared_ptr<RenderContext>(new RenderContext());
    if (const char *value = std::getenv("IVF2_AUTO_INSTANCING"); value != nullptr && std::strcmp(value, "0") == 0)
        context->m_autoInstancing = false;
    return context;
}

GLuint RenderContext::uploadInstanceTransforms(std::span<const glm::mat4> transforms)
{
    m_instanceScratch.resize(transforms.size());
    for (std::size_t i = 0; i < transforms.size(); ++i)
        m_instanceScratch[i] = InstanceAttributes{transforms[i], glm::vec4(1.0f), glm::vec4(0.0f)};

    if (m_instanceBuffer == 0)
        glGenBuffers(1, &m_instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceBuffer);
    // New storage for every batch (orphaning): earlier draws keep theirs, no waiting on the GPU
    m_instanceCapacity = std::max<std::size_t>(transforms.size(), m_instanceCapacity);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_instanceCapacity * sizeof(InstanceAttributes)), nullptr,
                 GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(transforms.size() * sizeof(InstanceAttributes)),
                    m_instanceScratch.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return m_instanceBuffer;
}

RenderContext::~RenderContext()
{
    // GL objects other contexts handed over for deletion (the context is current here)
    runDeleteLater();

    // Managers that look up uniforms in the shader manager's programs first,
    // the shader manager (owning the programs) last.
    delete m_resourceWatcher;
    delete m_shaderWatcher;
    delete m_debugDraw;
    delete m_selection;
    delete m_lights;
    delete m_textures;
    delete m_transforms;
    delete m_meshes;
    delete m_fonts;
    delete m_shaders;

    if (m_instanceBuffer != 0)
        glDeleteBuffers(1, &m_instanceBuffer);

    if (m_fallbackTexture2D != 0)
        glDeleteTextures(1, &m_fallbackTexture2D);
    if (m_fallbackCubemap != 0)
        glDeleteTextures(1, &m_fallbackCubemap);

    if (t_current == this)
        t_current = nullptr;
}

void RenderContext::makeCurrent()
{
    t_current = this;
    runDeleteLater();
}

std::weak_ptr<const void> RenderContext::lifetime() const
{
    return m_lifetime;
}

void RenderContext::deleteLater(std::function<void()> work)
{
    if (work)
        m_deleteLater.push_back(std::move(work));
}

void RenderContext::runDeleteLater()
{
    // Swap first: the work may queue more (it runs on the next makeCurrent())
    std::vector<std::function<void()>> work;
    work.swap(m_deleteLater);
    for (auto &fn : work)
        fn();
}

bool RenderContext::isCurrent() const
{
    return t_current == this;
}

RenderContext *RenderContext::current()
{
    return t_current;
}

void RenderContext::clearCurrent()
{
    t_current = nullptr;
}

RenderContext *RenderContext::currentOrFallback()
{
    if (t_current != nullptr)
        return t_current;

    if (g_fallback == nullptr)
    {
        logWarning("No current RenderContext - using a process-wide fallback context. Create a RenderContext "
                   "for your OpenGL context (GLFWWindow does this automatically).",
                   "RenderContext");
        g_fallback = new RenderContext();
    }
    return g_fallback;
}

bool RenderContext::setupDefaults([[maybe_unused]] const std::string &fontFile)
{
#if IVF2_HAS_FREETYPE
    // A missing default font only disables text rendering; the reason is logged.
    static_cast<void>(fonts()->loadFace(fontFile, "gidole"));
#endif

    auto shaderMgr = shaders();
    // A failure is logged with the driver output by the ShaderManager.
    if (!shaderMgr->loadRenderToTextureShader() || !shaderMgr->loadBasicShader() || !shaderMgr->loadPBRShader())
    {
        logError("Couldn't compile the default shaders", "RenderContext");
        return false;
    }
    shaderMgr->apply();

    auto lightMgr = lights();
    lightMgr->enableLighting();
    auto dirLight = lightMgr->addDirectionalLight();
    dirLight->setAmbientColor(glm::vec3(0.3, 0.3, 0.3));
    dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    dirLight->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
    dirLight->setDirection(glm::vec3(-1.0, -2.0, -3.0));
    dirLight->setEnabled(true);
    lightMgr->apply();

    return true;
}

// Managers are created on first use, in the order the application asks for
// them - several constructors look up uniforms in the current shader program.

ShaderManager *RenderContext::shaders()
{
    if (!m_shaders)
        m_shaders = new ShaderManager();
    return m_shaders;
}

LightManager *RenderContext::lights()
{
    if (!m_lights)
        m_lights = new LightManager();
    return m_lights;
}

TextureManager *RenderContext::textures()
{
    if (!m_textures)
        m_textures = new TextureManager();
    return m_textures;
}

FontManager *RenderContext::fonts()
{
    if (!m_fonts)
        m_fonts = new FontManager();
    return m_fonts;
}

MeshManager *RenderContext::meshes()
{
    if (!m_meshes)
        m_meshes = new MeshManager();
    return m_meshes;
}

SelectionManager *RenderContext::selection()
{
    if (!m_selection)
        m_selection = new SelectionManager();
    return m_selection;
}

TransformManager *RenderContext::transforms()
{
    if (!m_transforms)
        m_transforms = new TransformManager();
    return m_transforms;
}

DebugDraw *RenderContext::debugDraw()
{
    if (!m_debugDraw)
        m_debugDraw = new DebugDraw();
    return m_debugDraw;
}

ResourceWatcher *RenderContext::resourceWatcher()
{
    if (!m_resourceWatcher)
        m_resourceWatcher = new ResourceWatcher();
    return m_resourceWatcher;
}

ShaderWatcher *RenderContext::shaderWatcher()
{
    if (!m_shaderWatcher)
        m_shaderWatcher = new ShaderWatcher();
    return m_shaderWatcher;
}

GLuint RenderContext::fallbackTexture2D()
{
    if (m_fallbackTexture2D == 0)
    {
        GLint previous = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);

        const unsigned char pixel[] = {255, 255, 255, 255};
        glGenTextures(1, &m_fallbackTexture2D);
        glBindTexture(GL_TEXTURE_2D, m_fallbackTexture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous));
    }
    return m_fallbackTexture2D;
}

bool RenderContext::wideLinesSupported()
{
    if (!m_wideLines)
    {
        GLint flags = 0;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        GLfloat range[2] = {1.0f, 1.0f};
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
        m_wideLines = (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) == 0 && range[1] > 1.0f;
    }
    return *m_wideLines;
}

GLuint RenderContext::fallbackCubemap()
{
    if (m_fallbackCubemap == 0)
    {
        GLint previous = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &previous);

        const unsigned char pixel[] = {0, 0, 0, 255};
        glGenTextures(1, &m_fallbackCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_fallbackCubemap);
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++face)
            glTexImage2D(face, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, static_cast<GLuint>(previous));
    }
    return m_fallbackCubemap;
}
