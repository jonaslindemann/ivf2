#include <ivf/shadow_map.h>
#include <ivf/gl_debug.h>

#include <ivf/logger.h>
#include <ivf/render_context.h>

#include <iostream>

using namespace ivf;

ShadowMap::ShadowMap(int width, int height) : m_width(width), m_height(height), m_lightSpaceMatrix(1.0f)
{
    initialize();
}

ShadowMap::~ShadowMap()
{
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_depthTexture);
}

std::shared_ptr<ShadowMap> ShadowMap::create(int width, int height)
{
    return std::make_shared<ShadowMap>(width, height);
}

void ShadowMap::initialize()
{
    // Create depth texture
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // Try these parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if (RenderContext::isGles())
    {
        // No border clamping in OpenGL ES 3.0 (WebGL 2); ivfShadowFactor() rejects coordinates
        // outside the map and clamps inside each tile itself.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }
    // Turn OFF comparison mode for debugging
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    // Create and configure FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glLabel(GL_FRAMEBUFFER, m_fbo, "ivf shadow map");
    glLabel(GL_TEXTURE, m_depthTexture, "ivf shadow map depth");
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    const GLenum none = GL_NONE;
    glDrawBuffers(1, &none); // glDrawBuffer() is not in OpenGL ES
    glReadBuffer(GL_NONE);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        logErrorfc("ShadowMap", "Shadow map framebuffer is not complete!");

    glBindTexture(GL_TEXTURE_2D, 0); // not left on a unit (WebGL feedback loop checks)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::resize(int width, int height)
{
    if (m_width == width && m_height == height)
        return;

    m_width = width;
    m_height = height;

    this->unbind();

    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_depthTexture);

    // glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    this->initialize();
}

void ShadowMap::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void ShadowMap::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint ShadowMap::depthTexture() const
{
    return m_depthTexture;
}

void ShadowMap::setLightSpaceMatrix(const glm::mat4 &matrix)
{
    m_lightSpaceMatrix = matrix;
}

glm::mat4 ShadowMap::lightSpaceMatrix() const
{
    return m_lightSpaceMatrix;
}
