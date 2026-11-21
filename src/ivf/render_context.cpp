#include <ivf/render_context.h>

#include <ivf/config.h>
#include <ivf/debug_draw.h>
#include <ivf/font_manager.h>
#include <ivf/light_manager.h>
#include <ivf/logger.h>
#include <ivf/mesh_manager.h>
#include <ivf/selection_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/shader_watcher.h>
#include <ivf/texture_manager.h>
#include <ivf/transform_manager.h>

#include <glm/glm.hpp>

using namespace ivf;

namespace {

thread_local RenderContext *t_current = nullptr;

// Created on demand by currentOrFallback() for code that uses the singleton
// accessors without any current context. Intentionally never destroyed: at
// program exit no GL context is current, so releasing its GL objects then would
// be invalid (this matches the old singletons, which were never deleted either).
RenderContext *g_fallback = nullptr;

} // namespace

std::shared_ptr<RenderContext> RenderContext::create(GLLoadProc loader)
{
    if (loader != nullptr && !gladLoadGLLoader(reinterpret_cast<GLADloadproc>(loader)))
    {
        logError("Failed to load OpenGL functions", "RenderContext");
        return nullptr;
    }

    return std::shared_ptr<RenderContext>(new RenderContext());
}

RenderContext::~RenderContext()
{
    // Managers that look up uniforms in the shader manager's programs first,
    // the shader manager (owning the programs) last.
    delete m_shaderWatcher;
    delete m_debugDraw;
    delete m_selection;
    delete m_lights;
    delete m_textures;
    delete m_transforms;
    delete m_meshes;
    delete m_fonts;
    delete m_shaders;

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

bool RenderContext::setupDefaults(const std::string &fontFile)
{
#if IVF2_HAS_FREETYPE
    fonts()->loadFace(fontFile, "gidole");
#endif

    auto shaderMgr = shaders();
    shaderMgr->loadRenderToTextureShader();
    shaderMgr->loadBasicShader();
    shaderMgr->loadPBRShader();
    shaderMgr->apply();

    if (shaderMgr->compileLinkErrors())
    {
        logError("Couldn't compile the default shaders", "RenderContext");
        return false;
    }

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
