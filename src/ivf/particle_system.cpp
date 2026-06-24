#include <ivf/particle_system.h>
#include <ivf/texture.h>
#include <ivf/transform_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/logger.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <vector>

namespace ivf {

// ---- Inline shaders --------------------------------------------------------

static const char* k_particleVert = R"(
#version 330 core

// Per-vertex (unit quad, -0.5..0.5)
layout(location = 0) in vec2 aQuadPos;

// Per-instance
layout(location = 1) in vec3  iPos;
layout(location = 2) in float iSize;
layout(location = 3) in vec4  iColor;

out vec2  vUV;
out vec4  vColor;

uniform mat4 viewProj;
uniform mat4 view;
uniform bool billboard;

void main() {
    vUV    = aQuadPos + 0.5;
    vColor = iColor;

    vec3 quadPos;
    if (billboard) {
        // extract camera right/up from inverse view (upper 3x3)
        vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
        vec3 up    = vec3(view[0][1], view[1][1], view[2][1]);
        quadPos = iPos + (right * aQuadPos.x + up * aQuadPos.y) * iSize;
    } else {
        quadPos = iPos + vec3(aQuadPos, 0.0) * iSize;
    }

    gl_Position = viewProj * vec4(quadPos, 1.0);
}
)";

static const char* k_particleFrag = R"(
#version 330 core
in vec2  vUV;
in vec4  vColor;
out vec4 fragColor;
uniform sampler2D tex;
uniform bool useTexture;
void main() {
    vec4 c = vColor;
    if (useTexture) c *= texture(tex, vUV);
    if (c.a < 0.01) discard;
    fragColor = c;
}
)";

// ---- Static quad vertices (two triangles) ----------------------------------

static const float k_quad[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.5f,  0.5f,
    -0.5f, -0.5f,
     0.5f,  0.5f,
    -0.5f,  0.5f,
};

// ---- Ctor / Dtor -----------------------------------------------------------

ParticleSystem::ParticleSystem(int maxParticles)
    : m_maxParticles(maxParticles)
{
    m_pool.resize(maxParticles);
    m_instanceData.resize(maxParticles);
}

ParticleSystem::~ParticleSystem()
{
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); }
    if (m_quadVBO) { glDeleteBuffers(1, &m_quadVBO); }
    if (m_instanceVBO) { glDeleteBuffers(1, &m_instanceVBO); }
    if (m_program) { glDeleteProgram(m_program); }
}

std::shared_ptr<ParticleSystem> ParticleSystem::create(int maxParticles)
{
    return std::make_shared<ParticleSystem>(maxParticles);
}

// ---- GPU init --------------------------------------------------------------

void ParticleSystem::initGPU()
{
    if (m_gpuReady) return;

    // Compile shaders with error checking
    auto compileShader = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> log(len);
            glGetShaderInfoLog(s, len, nullptr, log.data());
            logError(std::string("ParticleSystem shader compile error: ") + log.data(), "ParticleSystem");
        }
        return s;
    };

    GLuint vert = compileShader(GL_VERTEX_SHADER,   k_particleVert);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, k_particleFrag);
    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);
    {
        GLint ok = 0;
        glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> log(len);
            glGetProgramInfoLog(m_program, len, nullptr, log.data());
            logError(std::string("ParticleSystem shader link error: ") + log.data(), "ParticleSystem");
        }
    }
    glDeleteShader(vert);
    glDeleteShader(frag);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_quadVBO);
    glGenBuffers(1, &m_instanceVBO);

    glBindVertexArray(m_vao);

    // Static quad VBO (location 0)
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(k_quad), k_quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    // Instance VBO (locations 1,2,3) — size updated each frame
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(m_maxParticles * sizeof(InstanceData)), nullptr, GL_DYNAMIC_DRAW);

    // location 1: pos (vec3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                          reinterpret_cast<void*>(offsetof(InstanceData, pos)));
    glVertexAttribDivisor(1, 1);

    // location 2: size (float)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                          reinterpret_cast<void*>(offsetof(InstanceData, size)));
    glVertexAttribDivisor(2, 1);

    // location 3: color (vec4)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                          reinterpret_cast<void*>(offsetof(InstanceData, color)));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    m_gpuReady = true;
}

// ---- Helpers ---------------------------------------------------------------

float ParticleSystem::randRange(float lo, float hi)
{
    std::uniform_real_distribution<float> d(lo, hi);
    return d(m_rng);
}

glm::vec3 ParticleSystem::randVec3(glm::vec3 lo, glm::vec3 hi)
{
    return { randRange(lo.x, hi.x), randRange(lo.y, hi.y), randRange(lo.z, hi.z) };
}

void ParticleSystem::spawnParticle()
{
    // Find a dead slot
    for (auto& p : m_pool) {
        if (p.alive) continue;

        glm::vec3 spawnPos{0,0,0};
        if (m_emitShape == 0) {
            spawnPos = m_emitCenter;
        } else if (m_emitShape == 1) {
            // Random point inside sphere
            float theta = randRange(0, glm::two_pi<float>());
            float phi   = randRange(0, glm::pi<float>());
            float r     = m_emitRadius * std::cbrt(randRange(0,1));
            spawnPos = m_emitCenter + glm::vec3(
                r * std::sin(phi) * std::cos(theta),
                r * std::cos(phi),
                r * std::sin(phi) * std::sin(theta));
        } else {
            spawnPos = randVec3(m_emitBoxMin, m_emitBoxMax);
        }

        p.position = spawnPos;
        p.velocity = randVec3(m_minVel, m_maxVel);
        p.maxLife  = randRange(m_minLife, m_maxLife);
        p.life     = p.maxLife;
        p.color    = m_startColor;
        p.size     = m_startSize;
        p.alive    = true;
        ++m_aliveCount;
        return;
    }
}

// ---- Emit / Update / Draw --------------------------------------------------

void ParticleSystem::emit(int count)
{
    for (int i = 0; i < count; ++i) spawnParticle();
}

void ParticleSystem::update(float dt)
{
    if (m_playing) {
        m_emitAccumulator += dt;
        float interval = (m_emitRate > 0) ? 1.0f / m_emitRate : 0.0f;
        while (m_emitAccumulator >= interval && interval > 0) {
            spawnParticle();
            m_emitAccumulator -= interval;
        }
    }

    m_aliveCount = 0;
    for (auto& p : m_pool) {
        if (!p.alive) continue;
        p.life -= dt;
        bool expired = p.life <= 0.0f;

        if (!expired) {
            if (m_updateFn) {
                m_updateFn(p, dt);
            } else {
                p.velocity += m_gravity * dt;
                p.position += p.velocity * dt;
            }
        }

        float t = glm::clamp(1.0f - p.life / p.maxLife, 0.0f, 1.0f);
        p.color = glm::mix(m_startColor, m_endColor, t);
        p.size  = glm::mix(m_startSize,  m_endSize,  t);

        if (expired) {
            p.alive = false;
            continue;
        }

        ++m_aliveCount;
    }
}

void ParticleSystem::doSetup()
{
    initGPU();
}

void ParticleSystem::doDraw()
{
    if (!m_gpuReady) initGPU();

    // Advance simulation
    // (The scene window doesn't call update() — we expose it so users can
    //  call it in their onUpdate(). Alternatively hook via a behavior.)

    if (m_aliveCount == 0) return;

    // Pack instance data
    int idx = 0;
    for (const auto& p : m_pool) {
        if (!p.alive) continue;
        m_instanceData[idx++] = { p.position, p.size, p.color };
        if (idx >= m_maxParticles) break;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    GLsizeiptr(idx * sizeof(InstanceData)),
                    m_instanceData.data());

    auto& xfm = *TransformManager::instance();
    glm::mat4 vp = xfm.projectionMatrix() * xfm.viewMatrix();

    glUseProgram(m_program);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "viewProj"), 1, GL_FALSE, glm::value_ptr(vp));
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"),     1, GL_FALSE, glm::value_ptr(xfm.viewMatrix()));
    glUniform1i(glGetUniformLocation(m_program, "billboard"), m_billboard ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_program, "useTexture"), (m_texture ? 1 : 0));

    if (m_texture) {
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        glUniform1i(glGetUniformLocation(m_program, "tex"), 0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // additive: overlapping particles glow
    glDepthMask(GL_FALSE);

    glBindVertexArray(m_vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, idx);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    if (m_texture) m_texture->unbind();

    // Restore the scene's basic shader so subsequent drawing uses the correct program.
    ShaderManager::instance()->currentProgram()->use();
}

} // namespace ivf
