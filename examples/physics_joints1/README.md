# physics_joints1 - joints and compound bodies with ivfphysics

Joints connect bodies to each other or to the world. Each is created from an anchor point
(and an axis) in world coordinates, in the pose the bodies have at that moment:

- **Hanging bridge**: planks along a sagging curve, each hinged to the next
  (`addHingeJoint(plank, next, point, {0, 0, 1})`); the end planks are hinged to the world
  (`nullptr` as the second body). Crates drop onto it.
- **Pendulum chain**: spheres joined by ball and socket joints, starting horizontally.
- **Windmill**: a cross made of two boxes is one body with a compound shape
  (`Shape::compound()`), on a hinge whose motor turns it (`setMotor(speed, maxTorque)`).
- **Piston**: a heavy block on a slider with limits (`setLimits(0, 3)`) and a motor that the
  example reverses at the ends of the stroke; it pushes crates.
- **Welded tower**: blocks held by fixed joints with a break force and torque
  (`JointDesc{.breakForce = 400, .breakTorque = 150}`). Throw balls at it (**T**): welds that
  take too much break, and the panel counts them (`setJointBreakCallback()`). **R** resets the
  scene and mends them.
- **Table**: one body with a compound shape of a top and four legs, under a crate.

The debug view (**V**) shows the joints: anchors, axes and the lines to the joined bodies;
colliders and contacts can be added in the panel. Bodies can be dragged with the left mouse
button, **P** pauses.

```cpp
auto door = world->addBody(doorNode, ivfphysics::Shape::box({0.5f, 1.0f, 0.05f}), {.mass = 10.0f});
auto hinge = world->addHingeJoint(door, nullptr, {-0.5f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
hinge->setLimits(-1.5f, 1.5f);  // radians, around the pose at creation
hinge->setMotor(0.5f, 20.0f);   // rad/s, at most 20 N m
```
