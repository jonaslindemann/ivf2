# Flow Fields

A flow field is a vector field that assigns a direction and speed to every point in space. In ivf2, `FlowField` uses **curl noise** to generate these fields — a mathematical construction that guarantees particles neither cluster into piles nor spread out to nothing. The result is organic, fluid-like streams that evolve slowly over time.

## How Curl Noise Works

Curl noise derives a velocity field from the curl (rotation) of a scalar potential. Because the curl of any field is divergence-free, the resulting flow has no sources or sinks. Particles steered by curl noise meander continuously without accumulating or dispersing.

Under the hood, `FlowField` layers multiple octaves of Perlin noise (fBm) to add detail at different scales, then takes finite differences to approximate the curl. The field also evolves over time so the streams shift gradually.

## Setup

```cpp
#include <ivf/flow_field.h>   // included via <ivf/gl.h>

auto field = ivf::FlowField::create();
field->setScale(0.25f);                            // spatial feature size
field->setStrength(3.5f);                          // peak velocity (world units/sec)
field->setOctaves(2);                              // fBm detail layers
field->setTimeScale(0.06f);                        // how fast the field evolves
field->setOffset(glm::vec3(31.4f, 17.3f, 57.1f)); // move away from noise origin
```

### Why set an offset?

Perlin noise is nearly zero near the origin. An offset places your field in a well-varied region of the noise domain, giving diverse flow directions across the scene.

## With a Particle System

The simplest integration is `applyToParticleSystem()`. It installs a steering function that blends each living particle's velocity toward the local curl-noise velocity each frame.

```cpp
// In onSetup():
m_ps = ivf::ParticleSystem::create(8000);
m_ps->setEmitRate(400.0f);
m_ps->setLifetime(6.0f, 12.0f);
m_ps->setGravity(glm::vec3(0.0f));          // no gravity — let the field do the work
m_ps->setEmitFromBox(
    glm::vec3(-8.0f, -0.5f, -8.0f),
    glm::vec3( 8.0f,  0.5f,  8.0f)
);
m_ps->start();
add(m_ps);

field->applyToParticleSystem(m_ps, 3.0f);  // blendRate: higher = snappier response
```

The `blendRate` parameter controls how quickly particles align with the field:

| blendRate | Character |
|-----------|-----------|
| 0.5 | Sluggish — inertia dominates, flow is subtle |
| 3–5 | Responsive — particles follow the field closely |
| 10+ | Instant — particles move exactly with the field |

## Driving with TimeController

Call `setTime()` every frame so the field evolves:

```cpp
void onUpdate() override
{
    double t  = ivf::TimeController::instance()->elapsed();
    double dt = ivf::TimeController::instance()->delta();

    m_field->setTime(static_cast<float>(t));
    m_ps->update(static_cast<float>(dt));
}
```

Because `TimeController::elapsed()` respects pause and scale, pausing the app freezes the field and the particles simultaneously.

## Manual Sampling

Sample any position in the field without a particle system:

```cpp
glm::vec3 pos = {x, y, z};
glm::vec3 vel = field->sampleVelocity(pos);

// For flat (XZ-plane) scenes:
glm::vec2 vel2d = field->sampleVelocity2D(x, z);  // y component is 0
```

Useful for steering custom objects, driving deformers, or visualising the field with line primitives.

## Parameter Guide

| Parameter | Method | Effect |
|-----------|--------|--------|
| Scale | `setScale(float)` | Smaller → broader, slower swirls; larger → tight, fast eddies |
| Strength | `setStrength(float)` | Peak particle speed in world units per second |
| Octaves | `setOctaves(int)` | More octaves add fine vortex detail (1–4 is practical) |
| Lacunarity | `setLacunarity(float)` | Frequency multiplier per octave (default 2.0) |
| Gain | `setGain(float)` | Amplitude decay per octave (default 0.5) |
| Time scale | `setTimeScale(float)` | Field evolution speed — 0 freezes the field pattern |
| Offset | `setOffset(glm::vec3)` | Domain shift — escape the near-zero noise origin |

## Live-Tweak Sliders

Use ImGui sliders to explore parameters in real time:

```cpp
void onDrawUi() override
{
    ImGui::Begin("Flow Field");
    bool changed = false;
    changed |= ImGui::SliderFloat("Scale",      &m_scale,     0.05f, 2.0f);
    changed |= ImGui::SliderFloat("Strength",   &m_strength,  0.1f, 10.0f);
    changed |= ImGui::SliderInt  ("Octaves",    &m_octaves,   1, 4);
    changed |= ImGui::SliderFloat("Time speed", &m_timeScale, 0.0f, 0.5f);
    if (changed) {
        m_field->setScale(m_scale);
        m_field->setStrength(m_strength);
        m_field->setOctaves(m_octaves);
        m_field->setTimeScale(m_timeScale);
    }
    ImGui::End();
}
```

See the full working example at [Flow Field Examples](../examples/flow_field_examples.md).
