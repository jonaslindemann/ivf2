#pragma once

/**
 * @file flow_field.h
 * @brief Declares FlowField — a curl-noise-based divergence-free 3D vector field.
 */

#include <memory>
#include <glm/glm.hpp>
#include <ivf/particle_system.h>

namespace ivf {

/**
 * @class FlowField
 * @brief Curl-noise vector field for fluid-like particle and deformer steering.
 *
 * Wraps `curlNoise3D` with configurable scale, strength, fBm octaves, and
 * time evolution. The resulting field is divergence-free: particles neither
 * cluster nor disperse — they simply flow.
 *
 * Typical use with a particle system:
 * @code
 * auto field = FlowField::create();
 * field->setScale(0.4f);
 * field->setStrength(3.0f);
 * field->setOctaves(2);
 * field->applyToParticleSystem(ps);   // installs update function
 *
 * // in onUpdate():
 * field->setTime(TimeController::instance()->elapsed());
 * ps->update(dt);
 * @endcode
 *
 * Or sample manually:
 * @code
 * glm::vec3 vel = field->sampleVelocity(particle.position);
 * @endcode
 */
class FlowField : public std::enable_shared_from_this<FlowField> {
private:
    float m_scale{0.5f};       // spatial scale: positions divided before noise lookup
    float m_strength{2.0f};    // output vector magnitude multiplier
    int   m_octaves{2};        // fBm layers for richer curl detail
    float m_lacunarity{2.0f};  // frequency multiplier per fBm octave
    float m_gain{0.5f};        // amplitude multiplier per fBm octave
    float m_timeScale{0.05f};  // how fast the field evolves (multiplies time before lookup)
    float m_time{0.0f};        // current animated time
    glm::vec3 m_offset{0.0f}; // world-space offset into the noise domain

public:
    FlowField() = default;

    static std::shared_ptr<FlowField> create();

    // ---- Configuration ---------------------------------------------------

    /** Spatial scale. Smaller values → broader, slower features. */
    void setScale(float s)      { m_scale      = s; }
    float scale() const         { return m_scale; }

    /** Velocity magnitude multiplier. */
    void setStrength(float s)   { m_strength   = s; }
    float strength() const      { return m_strength; }

    /** fBm octave count. More octaves → finer vortex detail. */
    void setOctaves(int n)      { m_octaves    = std::max(1, n); }
    int  octaves() const        { return m_octaves; }

    void setLacunarity(float l) { m_lacunarity = l; }
    void setGain(float g)       { m_gain       = g; }

    /** Controls how quickly the field rotates over time. */
    void setTimeScale(float s)  { m_timeScale  = s; }
    float timeScale() const     { return m_timeScale; }

    /** World-space translation into the noise domain. */
    void setOffset(glm::vec3 o) { m_offset     = o; }
    glm::vec3 offset() const    { return m_offset; }

    // ---- Time ------------------------------------------------------------

    /** Set current time directly (e.g. from TimeController::elapsed()). */
    void setTime(float t)       { m_time       = t; }
    float time() const          { return m_time; }

    // ---- Sampling --------------------------------------------------------

    /**
     * @brief Sample the flow velocity at a world-space position.
     *
     * Uses fBm-layered curl noise. Thread-safe (reads only const state).
     */
    glm::vec3 sampleVelocity(glm::vec3 pos) const;

    /**
     * @brief Sample the 2D flow velocity at (x, z) — useful for flat scenes.
     *
     * Returns an XZ vector (y component is 0).
     */
    glm::vec2 sampleVelocity2D(float x, float z) const;

    // ---- Particle system integration ------------------------------------

    /**
     * @brief Install a curl-noise steering function on a ParticleSystem.
     *
     * Each living particle's velocity is blended toward the local curl-noise
     * velocity at rate `blendRate` (units: 1/second). Call field->setTime() each
     * frame before ps->update() for time-animated flow.
     *
     * @param ps        The particle system to steer.
     * @param blendRate How quickly particles align with the flow (5 = fast, 0.5 = sluggish).
     */
    void applyToParticleSystem(std::shared_ptr<ParticleSystem> ps, float blendRate = 5.0f);
};

using FlowFieldPtr = std::shared_ptr<FlowField>;

} // namespace ivf
