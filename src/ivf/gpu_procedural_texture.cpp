#include <ivf/gpu_procedural_texture.h>
#include <ivf/procedural_shaders.h>
#include <ivf/shader_manager.h>
#include <ivf/vertex_shader.h>
#include <ivf/fragment_shader.h>
#include <ivf/logger.h>
#include <ivf/utils.h>

using namespace ivf;

// ============================================================================
// GPUProceduralTexture Base Class
// ============================================================================

GPUProceduralTexture::GPUProceduralTexture()
    : Texture()
{
    createQuad();
}

GPUProceduralTexture::~GPUProceduralTexture()
{
    cleanup();
}

void GPUProceduralTexture::createQuad()
{
    // Fullscreen quad vertices: position (xyz) + texcoord (uv)
    float quadVertices[] = {
        // positions        // texcoords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    GL_ERR_BEGIN;
    
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glGenBuffers(1, &m_quadEBO);
    
    glBindVertexArray(m_quadVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coord attribute (location 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    GL_ERR_END("GPUProceduralTexture::createQuad()");
}

void GPUProceduralTexture::cleanup()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_quadVBO);
        glDeleteBuffers(1, &m_quadEBO);
        m_quadVAO = m_quadVBO = m_quadEBO = 0;
    }
}

void GPUProceduralTexture::setSize(int width, int height)
{
    if (width != m_width || height != m_height) {
        m_width = width;
        m_height = height;
        m_needsRegeneration = true;
    }
}

void GPUProceduralTexture::setNeedsRegeneration(bool flag)
{
    m_needsRegeneration = flag;
}

void GPUProceduralTexture::setGeneratorShader(ProgramPtr shader)
{
    m_generatorShader = shader;
}

void GPUProceduralTexture::setTime(float time)
{
    if (m_time != time) {
        m_time = time;
        m_needsRegeneration = true;
    }
}

void GPUProceduralTexture::regenerate()
{
    if (!m_generatorShader) {
        logErrorfc("GPUProceduralTexture", "No generator shader set");
        return;
    }
    
    logInfofc("GPUProceduralTexture", "Regenerating GPU texture: {}x{}", m_width, m_height);
    
    // Save current OpenGL state
    GLint oldFBO;
    GLint oldViewport[4];
    GLint oldProgram;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
    glGetIntegerv(GL_VIEWPORT, oldViewport);
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    
    GL_ERR_BEGIN;
    
    // Create/recreate FBO if needed
    if (m_fbo == 0) {
        glGenFramebuffers(1, &m_fbo);
    }
    
    // Ensure texture is allocated with correct size
    glBindTexture(GL_TEXTURE_2D, this->id());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->id(), 0);
    
    // Check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        logErrorfc("GPUProceduralTexture", "Framebuffer not complete: {}", status);
        glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
        GL_ERR_END("GPUProceduralTexture::regenerate() - FBO error");
        return;
    }
    
    // Set viewport to texture size
    glViewport(0, 0, m_width, m_height);
    
    // Clear and render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Disable depth test for 2D rendering
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (depthTestEnabled) glDisable(GL_DEPTH_TEST);
    
    // Use procedural shader
    m_generatorShader->use();
    
    // Set common uniforms
    m_generatorShader->uniformFloat("time", m_time);
    
    // Set subclass-specific uniforms
    setShaderUniforms();
    
    // Draw fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // Restore depth test
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    
    // Generate mipmaps
    glBindTexture(GL_TEXTURE_2D, this->id());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
    glUseProgram(oldProgram);
    
    GL_ERR_END("GPUProceduralTexture::regenerate()");
    
    m_needsRegeneration = false;
    
    logInfofc("GPUProceduralTexture", "GPU texture generation complete");
}

// ============================================================================
// GPUCheckerboardTexture
// ============================================================================

GPUCheckerboardTexture::GPUCheckerboardTexture()
    : GPUProceduralTexture()
{
    // Create and compile procedural shader
    auto vertShader = VertexShader::create();
    vertShader->setSource(proc_quad_vert_shader);
    vertShader->compile();
    
    auto fragShader = FragmentShader::create();
    fragShader->setSource(proc_checkerboard_frag_shader);
    fragShader->compile();
    
    auto program = Program::create();
    program->setName("GPUCheckerboardShader");
    program->addShader(vertShader);
    program->addShader(fragShader);
    program->link();
    
    setGeneratorShader(program);
}

std::shared_ptr<GPUCheckerboardTexture> GPUCheckerboardTexture::create()
{
    return std::make_shared<GPUCheckerboardTexture>();
}

void GPUCheckerboardTexture::setColors(const glm::vec4 &color1, const glm::vec4 &color2)
{
    m_color1 = color1;
    m_color2 = color2;
    m_needsRegeneration = true;
}

void GPUCheckerboardTexture::setCheckerSize(float size)
{
    m_checkerSize = size;
    m_needsRegeneration = true;
}

void GPUCheckerboardTexture::setShaderUniforms()
{
    m_generatorShader->uniformVec4("color1", m_color1);
    m_generatorShader->uniformVec4("color2", m_color2);
    m_generatorShader->uniformFloat("checkerSize", m_checkerSize);
}

// ============================================================================
// GPUNoiseTexture
// ============================================================================

GPUNoiseTexture::GPUNoiseTexture()
    : GPUProceduralTexture()
{
    // Create and compile procedural shader
    auto vertShader = VertexShader::create();
    vertShader->setSource(proc_quad_vert_shader);
    vertShader->compile();
    
    auto fragShader = FragmentShader::create();
    fragShader->setSource(proc_noise_frag_shader);
    fragShader->compile();
    
    auto program = Program::create();
    program->setName("GPUNoiseShader");
    program->addShader(vertShader);
    program->addShader(fragShader);
    program->link();
    
    setGeneratorShader(program);
}

std::shared_ptr<GPUNoiseTexture> GPUNoiseTexture::create()
{
    return std::make_shared<GPUNoiseTexture>();
}

void GPUNoiseTexture::setScale(float scale)
{
    m_scale = scale;
    m_needsRegeneration = true;
}

void GPUNoiseTexture::setOctaves(int octaves)
{
    m_octaves = octaves;
    m_needsRegeneration = true;
}

void GPUNoiseTexture::setPersistence(float persistence)
{
    m_persistence = persistence;
    m_needsRegeneration = true;
}

void GPUNoiseTexture::setLacunarity(float lacunarity)
{
    m_lacunarity = lacunarity;
    m_needsRegeneration = true;
}

void GPUNoiseTexture::setColorRange(const glm::vec4 &colorLow, const glm::vec4 &colorHigh)
{
    m_colorLow = colorLow;
    m_colorHigh = colorHigh;
    m_needsRegeneration = true;
}

void GPUNoiseTexture::setShaderUniforms()
{
    m_generatorShader->uniformVec4("colorLow", m_colorLow);
    m_generatorShader->uniformVec4("colorHigh", m_colorHigh);
    m_generatorShader->uniformFloat("scale", m_scale);
    m_generatorShader->uniformInt("octaves", m_octaves);
    m_generatorShader->uniformFloat("persistence", m_persistence);
    m_generatorShader->uniformFloat("lacunarity", m_lacunarity);
}

// ============================================================================
// GPUGradientTexture
// ============================================================================

GPUGradientTexture::GPUGradientTexture()
    : GPUProceduralTexture()
{
    // Create and compile procedural shader
    auto vertShader = VertexShader::create();
    vertShader->setSource(proc_quad_vert_shader);
    vertShader->compile();
    
    auto fragShader = FragmentShader::create();
    fragShader->setSource(proc_gradient_frag_shader);
    fragShader->compile();
    
    auto program = Program::create();
    program->setName("GPUGradientShader");
    program->addShader(vertShader);
    program->addShader(fragShader);
    program->link();
    
    setGeneratorShader(program);
}

std::shared_ptr<GPUGradientTexture> GPUGradientTexture::create()
{
    return std::make_shared<GPUGradientTexture>();
}

void GPUGradientTexture::setType(Type type)
{
    m_type = type;
    m_needsRegeneration = true;
}

void GPUGradientTexture::setColors(const glm::vec4 &colorStart, const glm::vec4 &colorEnd)
{
    m_colorStart = colorStart;
    m_colorEnd = colorEnd;
    m_needsRegeneration = true;
}

void GPUGradientTexture::setAngle(float angle)
{
    m_angle = angle;
    m_needsRegeneration = true;
}

void GPUGradientTexture::setCenter(const glm::vec2 &center)
{
    m_center = center;
    m_needsRegeneration = true;
}

void GPUGradientTexture::setShaderUniforms()
{
    m_generatorShader->uniformVec4("colorStart", m_colorStart);
    m_generatorShader->uniformVec4("colorEnd", m_colorEnd);
    m_generatorShader->uniformInt("gradientType", static_cast<int>(m_type));
    m_generatorShader->uniformFloat("angle", m_angle);
    // Use two floats instead of vec2
    GLint centerLoc = m_generatorShader->uniformLoc("center");
    if (centerLoc != -1) {
        glUniform2f(centerLoc, m_center.x, m_center.y);
    }
}
