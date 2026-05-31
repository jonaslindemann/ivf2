# Creative Programming

ivf2 is designed to be a productive tool for creative coding — building generative visuals, interactive installations, data visualisations, and real-time experiments. This section covers the features that support that kind of work.

## What is Creative Programming?

Creative programming treats code as a medium for making things that move, react, and evolve over time. In ivf2 this means:

- **Generative motion** — particles, deformers, and procedural geometry that produce complex behaviour from simple rules
- **Time as a material** — slowing, pausing, and looping time to shape the feel of a piece
- **Layered systems** — combining flow fields, animations, and effects to build emergent visual complexity

## Featured Systems

### [Flow Fields](flow_field.md)

Steer thousands of particles through organic, fluid-like trajectories using curl-noise vector fields. Because curl noise is divergence-free, particles neither cluster nor disperse — they simply flow. Start with `FlowField::create()` and a single `applyToParticleSystem()` call.

### [Time-Driven Animation](time_and_animation.md)

`TimeController` gives you a global, pausable, scalable time stream. Wire it into any animation, deformer, or particle system and get pause, slow-motion, and time-scrubbing for free. Combine it with `TimeControlPanel` for a drop-in interactive control.

### Camera Animation

Compose cinematic fly-throughs from a handful of position keyframes. Auto-look-forward, looping, and shake effects turn a static scene into a presentation. See [Camera Animation](../animation/camera_animation.md).

## Getting Started

The three examples in the [Examples](../examples/time_control_examples.md) section are the fastest path to working code:

- `time_control1` — wave-deformed plane and orbiting cubes with pause/slow-motion
- `flow_field1` — 8 000 curl-noise particles with live-tweak sliders
- `camera_anim1` — spline fly-through with shake and keyboard controls
