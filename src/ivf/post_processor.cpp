#include <ivf/post_processor.h>

#include <ivf/gl.h>

#include <ivf/texture.h>
#include <ivf/stock_shaders.h>

namespace ivf {

PostProcessor::PostProcessor(int width, int height)
    : m_width(width), m_height(height), m_fboA(0), m_fboB(0), m_textureA(0), m_textureB(0), m_quadVAO(0), m_quadVBO(0),
      m_time(0.0f)
{}

PostProcessor::~PostProcessor()
{
    glDeleteFramebuffers(1, &m_fboA);
    glDeleteFramebuffers(1, &m_fboB);
    glDeleteTextures(1, &m_textureA);
    glDeleteTextures(1, &m_textureB);
}

void PostProcessor::addEffect(ProgramPtr fxProgram)
{
    m_fxPrograms.push_back(fxProgram);
}

void PostProcessor::clearEffects()
{
    m_fxPrograms.clear();
}

void PostProcessor::initialize()
{
    glGenFramebuffers(1, &m_fboA);
    glGenFramebuffers(1, &m_fboB);
    glGenTextures(1, &m_textureA);
    glGenTextures(1, &m_textureB);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fboA);
    glBindTexture(GL_TEXTURE_2D, m_textureA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureA, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fboB);
    glBindTexture(GL_TEXTURE_2D, m_textureB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureB, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    this->initQuad();
}

void PostProcessor::resize(int width, int height)
{
    if ((m_width == width) && (m_height == height))
        return;

    m_width = width;
    m_height = height;
    this->drop();
    this->initialize();
}

void PostProcessor::drop()
{
    glDeleteFramebuffers(1, &m_fboA);
    glDeleteFramebuffers(1, &m_fboB);
    glDeleteTextures(1, &m_textureA);
    glDeleteTextures(1, &m_textureB);
}

std::shared_ptr<PostProcessor> PostProcessor::create(int width, int height)
{
    return std::make_shared<PostProcessor>(width, height);
}

void PostProcessor::initQuad()
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

void PostProcessor::setTime(float time)
{
    m_time = time;
}

void PostProcessor::apply(GLuint inputTexture)
{
    // Need two FBOs to ping-pong between effects
    GLuint sourceTexture = inputTexture;
    GLuint targetFBO;

    // Apply each effect in sequence
    for (size_t i = 0; i < m_fxPrograms.size(); i++)
    {
        // For last effect, render to screen instead of FBO
        bool isLastEffect = (i == m_fxPrograms.size() - 1);

        if (!isLastEffect)
        {
            // Ping-pong between FBOs
            targetFBO = (i % 2 == 0) ? m_fboA : m_fboB;
            glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
        }
        else
        {
            // Final pass renders to screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Clear the target
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the current effect's shader
        m_fxPrograms[i]->use();

        // Bind source texture and set uniforms
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        m_fxPrograms[i]->uniformInt("screenTexture", 0);
        // m_fxPrograms[i]->uniformInt("screenWidth", m_width);
        // m_fxPrograms[i]->uniformInt("screenHeight", m_height);
        m_fxPrograms[i]->uniformFloat("time", m_time);

        glDisable(GL_DEPTH_TEST);

        // Draw full-screen quad
        glViewport(0, 0, m_width, m_height);
        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Next effect will use this pass's output as input
        if (!isLastEffect)
        {
            sourceTexture = (targetFBO == m_fboA) ? m_textureA : m_textureB;
        }
    }
}

} // namespace ivf
