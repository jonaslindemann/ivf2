# rigid_body1 - rigid body physics with ivfphysics

The minimal `ivfphysics` example: a pile of boxes, spheres, capsules and capped cylinders
falls onto the ground and tumbles.

- A `PhysicsWorld` is stepped with the window's frame time (`world->step(frameTime())`): fixed
  steps of 1/240 s, with the nodes interpolated in between, so the motion is smooth at any frame
  rate and the same in every run.
- The ground is a static body with a plane shape; every other node gets a dynamic body whose
  shape comes from the node (`world->addBody(node, {.mass = 1.0f, ...})` uses
  `Shape::fromNode()`: boxes, spheres and capsules become those shapes, the cylinder a convex
  hull of its vertices).
- The bodies move their nodes; the scene is drawn as usual.

Keys: **D** drops another body, **P** pauses, **S** runs a single step, **R** resets the scene
to the start. The panel also has sliders for the time scale and gravity.

```cpp
auto world = ivfphysics::PhysicsWorld::create();
world->addBody(ground, ivfphysics::Shape::plane(), {.type = ivfphysics::BodyType::Static});

auto box = ivf::Box::create({0.4f, 0.4f, 0.4f}); // half extents
box->setPos({0.0f, 3.0f, 0.0f});
scene->add(box);
world->addBody(box, ivfphysics::Shape::box({0.4f, 0.4f, 0.4f}), {.mass = 1.0f});

// every frame
world->step(frameTime());
```

ReactPhysics3D does the simulation; it is private to `ivfphysics`, so the example includes
no physics engine headers.
