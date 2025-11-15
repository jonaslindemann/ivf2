#include <ivf/framebuffer.h>

#include <ivf/texture.h>
#include <ivf/logger.h>

#include <iostream>

using namespace ivf;

FrameBuffer::FrameBuffer(int width, int height)
    : m_width(width), m_height(height), m_multisample(false), m_samples(4), m_quadVAO(0), m_quadVBO(0),
      m_multisampledFrameBuffer(0), m_multisampledDepthBuffer(0), m_multisampledColorTexture(0), m_frameBuffer(0),
      m_depthBuffer(0), m_colorTexture(0)
{}

FrameBuffer::~FrameBuffer()
{
    this->drop();
}

void ivf::FrameBuffer::initQuad()
{
    float quadVertices[] = {
        // positions        // texture coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
}

std::shared_ptr<FrameBuffer> FrameBuffer::create(int width, int height)
{
    return std::make_shared<FrameBuffer>(width, height);
}

void FrameBuffer::attachColorTexture(GLuint &texture)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
}

void FrameBuffer::attachDepthBuffer(GLuint &buffer)
{
    glGenRenderbuffers(1, &buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer);
}

void ivf::FrameBuffer::attachMultisampledColorTexture()
{
    // Multisampled color buffer
    glGenTextures(1, &m_multisampledColorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_multisampledColorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGBA16F, m_width, m_height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_multisampledColorTexture,
                           0);
}

void ivf::FrameBuffer::attachMultisampledDepthBuffer()
{
    // Multisampled depth buffer
    glGenRenderbuffers(1, &m_multisampledDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_multisampledDepthBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH_COMPONENT, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_multisampledDepthBuffer);
}

void ivf::FrameBuffer::initialize()
{
    this->drop();
    this->initQuad();

    if (m_multisample)
    {
        glGenFramebuffers(1, &m_multisampledFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_multisampledFrameBuffer);

        this->attachMultisampledColorTexture();
        this->attachMultisampledDepthBuffer();

        glGenFramebuffers(1, &m_frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

        this->attachColorTexture(m_colorTexture);
        this->attachDepthBuffer(m_depthBuffer);

        /*
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
        glGenTextures(1, &m_currentColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_currentColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_currentColorTexture, 0);
        */
    }
    else
    {
        glGenFramebuffers(1, &m_frameBuffer);

        this->attachColorTexture(m_colorTexture);
        this->attachDepthBuffer(m_depthBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    }

    this->unbind();
}

void ivf::FrameBuffer::resize(int width, int height)
{
    if ((m_width == width) && (m_height == height))
        return;

    m_width = width;
    m_height = height;

    this->initialize();
}

void ivf::FrameBuffer::drop()
{
    glDeleteBuffers(1, &m_quadVBO);
    glDeleteVertexArrays(1, &m_quadVAO);

    if (m_multisample)
    {
        glDeleteTextures(1, &m_multisampledColorTexture);
        glDeleteRenderbuffers(1, &m_multisampledDepthBuffer);
        glDeleteFramebuffers(1, &m_multisampledFrameBuffer);
    }

    glDeleteTextures(1, &m_colorTexture);
    glDeleteRenderbuffers(1, &m_depthBuffer);
    glDeleteFramebuffers(1, &m_frameBuffer);
}

void FrameBuffer::bind()
{
    if (m_multisample)
        glBindFramebuffer(GL_FRAMEBUFFER, m_multisampledFrameBuffer);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ivf::FrameBuffer::attachMultisampledStencilBuffer()
{}

void ivf::FrameBuffer::setMultisample(bool multisample)
{
    m_multisample = multisample;
    this->initialize();
}

void ivf::FrameBuffer::setSamples(int samples)
{
    m_samples = samples;
    this->initialize();
}

bool ivf::FrameBuffer::multisample()
{
    return m_multisample;
}

int ivf::FrameBuffer::samples()
{
    return m_samples;
}

void FrameBuffer::checkStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        logErrorfc("FrameBuffer", "Framebuffer is not complete!");
    }
}

void ivf::FrameBuffer::begin()
{
    if (m_multisample)
        glBindFramebuffer(GL_FRAMEBUFFER, m_multisampledFrameBuffer);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
}

void ivf::FrameBuffer::end()
{
    if (m_multisample)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_multisampledFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ivf::FrameBuffer::draw()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(m_quadVAO);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);

    smCurrentProgram()->uniformInt("screenTexture", 0);

    glViewport(0, 0, m_width, m_height);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_DEPTH_TEST);
}

GLuint ivf::FrameBuffer::colorTexture()
{
    return m_colorTexture;
}

GLuint ivf::FrameBuffer::id()
{
    return m_frameBuffer;
}
