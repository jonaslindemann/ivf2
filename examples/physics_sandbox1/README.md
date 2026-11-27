# physics_sandbox1 - an interactive physics sandbox

The complete `ivfphysics` application: pick a preset, throw balls, drag bodies around and
watch the colours change with their speed.

**Presets** (panel or keys **1**-**5**; each starts in motion):

1. *Brick wall* - a heavy steel ball rolls into a wall of 72 bricks.
2. *Domino run* - dominoes along an S-curve; the first one is tipped.
3. *Tower and wrecking ball* - a tower of crossed logs and an 80 kg ball on a ball and socket
   joint, released from the side.
4. *Pendulums* - a Newton's cradle (balls on hinges with bounciness 1 and small gaps, so the
   impulse passes from ball to ball) and a chain of ten links on ball joints.
5. *Ramp course* - three ramps, each one static body with a compound shape (floor, rails, back
   wall); balls, capsules and cylinders roll down into a bin.

**Interaction**

- Drag bodies with the left mouse button (`DragController`); the wheel moves a dragged body
  nearer or farther. The camera is blocked while dragging.
- **Ctrl + click** or **Space** throws a steel ball through the mouse position (`throwBody()`).
- **P** pauses, **N** runs a single step, **R** rebuilds the preset, **L** toggles the debug
  view (velocity arrows, contact points, colliders).
- Sliders for gravity, friction, bounciness, simulation speed and throw speed.

**Display.** Every body has its own `PBRMaterial`; each frame its colour is set from its speed
through an `ivfvis` colormap, shown by a `ColorBar` (or the body's own colour when colouring is
off). The scene is lit by a procedural sky (`EnvironmentMap::loadSky()`, also the skybox) and a
shadow-casting sun, tone mapped with ACES. Strings of pendulums are drawn with `DebugDraw`
lines from each joint's anchor to its bodies.

The simulation uses fixed steps and no randomness, so a run is the same every time (the
visual test shows the brick wall after one second). `IVF2_SANDBOX_PRESET=1..5` starts with
another preset; in the browser add it to the page URL, e.g. `physics_sandbox1.html?IVF2_SANDBOX_PRESET=4`.
