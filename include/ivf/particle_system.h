#pragma once

#include <ivf/node.h>
#include <glm/glm.hpp>

#include <functional>
#include <vector>
#include <memory>
#include <random>

namespace ivf {

class Texture;

/**
 * @brief CPU-simulated, GPU-instanced particle system scene node.
 *
 * Particles are simulated on the CPU each frame and rendered as instanced
 * camera-facing quads. The system integrates into the scene graph like any
 * other Node — just add it to the scene.
 *
 * @code
 * auto ps = ParticleSystem::create(5000);
 * ps->setEmitRate(200);
 * ps->setLifetime(1.0f, 3.0f);
 * ps->setInitialVelocity({-0.5f,-0.5f,-0.5f}, {0.5f,2.0f,0.5f});
 * ps->setGravity({0,-2,0});
 * ps->setStartColor({1,0.6f,0.1f,1});
 * ps->setEndColor({1,0,0,0});
 * ps->setStartSize(0.15f);
 * ps->setEndSize(0.01f);
 * ps->start();
 * scene->add(ps);
 * @endcode
 */
class ParticleSystem : public Node {
public:
    struct Particle {
        glm::vec3 position{0,0,0};
        glm::vec3 velocity{0,0,0};
        glm::vec4 color{1,1,1,1};
        float size{1.0f};
        float life{0.0f};    // remaining lifetime in seconds
        float maxLife{1.0f}; // full lifetime (for interpolation)
        bool  alive{false};
    };

    using UpdateFn = std::function<void(Particle& p, float dt)>;

private:
    // Simulation state
    std::vector<Particle> m_pool;
    int m_maxParticles;
    int m_aliveCount{0};

    // Emitter settings
    float m_emitRate{50.0f};        // particles per second
    float m_emitAccumulator{0.0f};
    float m_minLife{1.0f}, m_maxLife{2.0f};
    glm::vec3 m_minVel{-1,0,-1}, m_maxVel{1,2,1};
    glm::vec3 m_gravity{0,-9.8f,0};
    glm::vec4 m_startColor{1,1,1,1}, m_endColor{1,1,1,0};
    float m_startSize{0.1f}, m_endSize{0.0f};

    // Emitter shape: 0=point, 1=sphere, 2=box
    int  m_emitShape{0};
    glm::vec3 m_emitCenter{0,0,0};
    float m_emitRadius{0.5f};
    glm::vec3 m_emitBoxMin{-0.5f,-0.5f,-0.5f}, m_emitBoxMax{0.5f,0.5f,0.5f};

    bool m_playing{false};
    bool m_billboard{true};

    // Depth-fog / distance attenuation cue
    bool      m_fogEnabled{false};
    float     m_fogNear{5.0f};
    float     m_fogFar{40.0f};
    glm::vec3 m_fogColor{0.0f, 0.0f, 0.0f};

    UpdateFn m_updateFn;

    // GPU resources
    GLuint m_vao{0}, m_quadVBO{0}, m_instanceVBO{0};
    GLuint m_program{0};
    std::shared_ptr<Texture> m_texture;
    bool m_gpuReady{false};

    struct InstanceData {
        glm::vec3 pos;
        float     size;
        glm::vec4 color;
    };
    std::vector<InstanceData> m_instanceData;

    std::mt19937 m_rng{std::random_device{}()};

    void initGPU();
    void spawnParticle();
    float randRange(float lo, float hi);
    glm::vec3 randVec3(glm::vec3 lo, glm::vec3 hi);

public:
    explicit ParticleSystem(int maxParticles = 10000);
    ~ParticleSystem() override;

    static std::shared_ptr<ParticleSystem> create(int maxParticles = 10000);

    // ---- Emitter config ------------------------------------------------

    void setEmitRate(float particlesPerSecond) { m_emitRate = particlesPerSecond; }
    void setLifetime(float minLife, float maxLife) { m_minLife = minLife; m_maxLife = maxLife; }
    void setInitialVelocity(glm::vec3 minVel, glm::vec3 maxVel) { m_minVel = minVel; m_maxVel = maxVel; }
    void setGravity(glm::vec3 g) { m_gravity = g; }
    void setStartColor(glm::vec4 c) { m_startColor = c; }
    void setEndColor(glm::vec4 c)   { m_endColor   = c; }
    void setStartSize(float s)      { m_startSize   = s; }
    void setEndSize(float s)        { m_endSize     = s; }
    void setBillboard(bool b)       { m_billboard   = b; }
    void setTexture(std::shared_ptr<Texture> tex) { m_texture = tex; }

    // ---- Depth fog (distance attenuation) ------------------------------

    void setDepthFog(bool enabled)          { m_fogEnabled = enabled; }
    void setFogRange(float near, float far) { m_fogNear = near; m_fogFar = far; }
    void setFogColor(glm::vec3 c)           { m_fogColor = c; }
    [[nodiscard]] bool      depthFog() const { return m_fogEnabled; }
    [[nodiscard]] float     fogNear() const  { return m_fogNear; }
    [[nodiscard]] float     fogFar() const   { return m_fogFar; }
    [[nodiscard]] glm::vec3 fogColor() const { return m_fogColor; }

    // ---- Emitter shape -------------------------------------------------

    void setEmitFromPoint(glm::vec3 pos) { m_emitShape = 0; m_emitCenter = pos; }
    void setEmitFromSphere(glm::vec3 center, float radius) { m_emitShape = 1; m_emitCenter = center; m_emitRadius = radius; }
    void setEmitFromBox(glm::vec3 min, glm::vec3 max) { m_emitShape = 2; m_emitBoxMin = min; m_emitBoxMax = max; }

    // ---- Playback ------------------------------------------------------

    void emit(int count = 1);
    void start() { m_playing = true; }
    void stop()  { m_playing = false; }
    [[nodiscard]] bool isPlaying() const { return m_playing; }

    // ---- Custom per-particle update ------------------------------------

    void setUpdateFunction(UpdateFn fn) { m_updateFn = std::move(fn); }

    // ---- Per-frame update + draw (called by Node::draw) ----------------

    void update(float dt);
    void doSetup() override;
    void doDraw() override;

    [[nodiscard]] int aliveCount() const { return m_aliveCount; }
    [[nodiscard]] int maxParticles() const { return m_maxParticles; }
};

using ParticleSystemPtr = std::shared_ptr<ParticleSystem>;

} // namespace ivf
