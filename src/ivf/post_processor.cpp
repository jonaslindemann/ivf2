#include <ivf/post_processor.h>

#include <ivf/gl.h>

#include <ivf/texture.h>
#include <ivf/stock_shaders.h>
#include <ivf/shader_manager.h>

namespace ivf {

PostProcessor::PostProcessor(int width, int height)
    : m_width(width), m_height(height), m_fboA(0), m_fboB(0), m_textureA(0), m_textureB(0), m_historyFBO(0),
      m_historyTex{0, 0}, m_historyIndex(0), m_quadVAO(0), m_quadVBO(0), m_time(0.0f)
{}

PostProcessor::~PostProcessor()
{
    glDeleteFramebuffers(1, &m_fboA);
    glDeleteFramebuffers(1, &m_fboB);
    glDeleteTextures(1, &m_textureA);
    glDeleteTextures(1, &m_textureB);
    glDeleteFramebuffers(1, &m_historyFBO);
    glDeleteTextures(2, m_historyTex);
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

    // History framebuffer + double-buffered textures used to retain the previous
    // frame's final composite (bound to unit 1 as "previousFrame" for temporal effects).

    glGenFramebuffers(1, &m_historyFBO);
    glGenTextures(2, m_historyTex);

    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_historyTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Clear to black so the first frame has no garbage to feed back.
        glBindFramebuffer(GL_FRAMEBUFFER, m_historyFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_historyTex[i], 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    m_historyIndex = 0;

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
    glDeleteFramebuffers(1, &m_historyFBO);
    glDeleteTextures(2, m_historyTex);
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
    // Only enabled effects participate in the chain. A disabled effect is skipped
    // entirely (Program::use() is a no-op when disabled, so drawing it would re-run
    // the previously bound shader and apply effects multiple times).
    std::vector<ProgramPtr> active;
    active.reserve(m_fxPrograms.size());
    for (auto &program : m_fxPrograms)
        if (program && program->enabled())
            active.push_back(program);

    // Nothing enabled: the scene was already drawn to the screen by the caller.
    if (active.empty())
        return;

    GLuint sourceTexture = inputTexture;
    GLuint targetFBO = m_fboA;

    // The previous frame's composite (read) and where we write this frame's composite.
    const int prev = m_historyIndex;
    const int cur = 1 - m_historyIndex;

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, m_width, m_height);

    // Bind the previous frame to unit 1 for the whole chain. Effects that declare a
    // "previousFrame" sampler (feedback, trails, motion blur) read from it; others ignore it.
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_historyTex[prev]);

    // Apply each enabled effect in sequence
    for (size_t i = 0; i < active.size(); i++)
    {
        // The final pass renders into the history FBO so we can both display it and
        // retain it as the previous frame for the next call.
        bool isLastEffect = (i == active.size() - 1);

        if (!isLastEffect)
        {
            // Ping-pong between FBOs
            targetFBO = (i % 2 == 0) ? m_fboA : m_fboB;
            glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
        }
        else
        {
            // Final pass renders into the history texture for this frame.
            glBindFramebuffer(GL_FRAMEBUFFER, m_historyFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_historyTex[cur], 0);
        }

        // Clear the target
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the current effect's shader
        active[i]->use();

        // Bind source texture and set uniforms
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTexture);

        active[i]->uniformInt("screenTexture", 0);
        active[i]->uniformInt("previousFrame", 1);
        active[i]->uniformFloat("time", m_time);

        // Draw full-screen quad
        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Next effect will use this pass's output as input
        if (!isLastEffect)
        {
            sourceTexture = (targetFBO == m_fboA) ? m_textureA : m_textureB;
        }
    }

    // Blit this frame's composite (now in m_historyTex[cur]) to the screen.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    smApplyProgram("render_to_texture");
    smCurrentProgram()->uniformInt("screenTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_historyTex[cur]);
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // The composite we just produced becomes next frame's previous frame.
    m_historyIndex = cur;
}

} // namespace ivf
