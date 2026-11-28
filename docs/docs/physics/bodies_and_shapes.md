# Bodies and Shapes

A rigid body pairs a scene node with a collision shape and physical properties such as mass, friction and bounciness. The `rigid_body1` example drops a pile of boxes, spheres, capsules and cylinders whose shapes come from their nodes.

## Adding bodies

```cpp
using namespace ivfphysics;

auto crate = ivf::Cube::create();
crate->setPos({0.0f, 3.0f, 0.0f});
scene->add(crate);

RigidBodyPtr body = world->addBody(crate, Shape::box(glm::vec3(0.5f)),
                                   {.mass = 2.0f, .friction = 0.6f, .bounciness = 0.2f});
```

The body starts at the node's position and rotation. `BodyDesc` holds the starting properties:

| Field | Default | Meaning |
|-------|---------|---------|
| `type` | `Dynamic` | `Static`, `Kinematic` or `Dynamic` (see [Nodes and bodies](index.md#nodes-and-bodies)) |
| `mass` | 1 | Mass of a dynamic body in kg; the inertia follows from the shape |
| `friction` | 0.5 | Friction coefficient; a contact uses the geometric mean of both bodies' values |
| `bounciness` | 0.1 | Restitution, from 0 (no bounce) to 1 (elastic); a contact uses the larger of both |
| `linearDamping`, `angularDamping` | 0, 0.05 | Slow the body down, like air resistance |
| `gravity` | true | Whether the world's gravity acts on it |
| `canSleep` | true | Whether it may sleep when it comes to rest |
| `category`, `collidesWith` | 1, 0xFFFF | [Collision filtering](events.md#collision-filtering) |
| `trigger` | false | A [trigger](events.md#triggers): reports overlaps but does not collide |

Adding a node a second time replaces its body. `world->bodyOf(node)` finds the body of a node, and `world->bodies()` lists all of them in the order they were added. `world->removeBody(body)` takes a body out of the world, together with its joints. The node stays where it is, so remove it from the scene yourself if it should disappear:

```cpp
scene->remove(body->node());
world->removeBody(body);
```

The world owns its bodies; a `RigidBodyPtr` you keep becomes invalid (`isValid()` false) once the body is removed or the world is destroyed.

## Shapes

Shapes are in the body's local coordinates, in world units. They are not scaled with the node.

| Shape | Created with | Notes |
|-------|--------------|-------|
| Box | `Shape::box(halfExtents)` | Half extents, as `ivf::Box` takes them |
| Sphere | `Shape::sphere(radius)` | |
| Capsule | `Shape::capsule(radius, height)` | Along local Y; `height` is the distance between the cap centres |
| Ground plane | `Shape::plane(normal, offset, size)` | A box 1000 wide and 1 thick whose top face is the plane |
| Convex hull | `Shape::convexHull(points)` or `(mesh, scale)` | The convex hull of a point set or of a mesh's vertices |
| Triangle mesh | `Shape::triangleMesh(positions, triangles)` or `(mesh, scale)` | Any (concave) surface; static and kinematic bodies only |
| Compound | `Shape::compound(parts)` | Several shapes in one body, see [Compound bodies](joints.md#compound-bodies) |

Hulls and triangle meshes return `ivf::Result<ShapePtr>`, because they can fail (too few points, all in one plane). A shape can be shared by many bodies, which saves memory for hundreds of identical bricks:

```cpp
auto brick = Shape::box({0.4f, 0.2f, 0.2f});
for (auto &node : bricks)
    world->addBody(node, brick, {.mass = 1.5f});
```

`setOffset(position, rotation)` places a shape away from the body origin, for a node whose geometry is not centred on its position. The offset is read when the shape is added to a body.

### Shapes from nodes

`Shape::fromNode(node)`, or `addBody(node, desc)` without a shape, derives the shape from the node and its local bounding box. The node's scale is baked in:

| Node | Shape |
|------|-------|
| `Cube`, `Box` | Box |
| `Sphere` | Sphere |
| `Capsule` | Capsule |
| `Plane` | A thin box just below the plane's surface |
| Any other node with meshes (`Cylinder`, `CappedCylinder`, `Cone`, `Dodecahedron`, `MeshNode`, a loaded model) | Convex hull of its vertices |
| The same with `fromNode(node, true)` | Triangle mesh (static bodies: terrain, a level) |

```cpp
// Shape from the node; the result fails for a node without geometry
if (auto body = world->addBody(rock, {.mass = 20.0f}); !body)
    ivf::logError(body.error().message);

// Concave ground from a loaded model
auto terrain = ivf::MeshNode::create(); // ... loaded or generated
world->addBody(terrain, *Shape::fromNode(terrain, true), {.type = BodyType::Static});
```

Hulls are a good fit for convex objects of any shape. A concave dynamic object, such as a chair, an L-bracket or a cup, needs a [compound shape](joints.md#compound-bodies): only static and kinematic bodies can use triangle meshes. Adding a triangle mesh to a dynamic body makes the body static, with a warning.

!!! note "Cylinders"
    `ivf::Cylinder` is an open tube, so its hull is solid but it looks hollow. Use `CappedCylinder` for a solid cylinder that looks like one.

## Moving bodies

Bodies are usually moved by forces, contacts and joints. You can also push them yourself:

```cpp
body->applyForce({0.0f, 50.0f, 0.0f});            // N, during the next step
body->applyForceAt(force, worldPoint);             // also turns the body
body->applyTorque({0.0f, 5.0f, 0.0f});             // N m
body->applyImpulse({0.0f, 0.0f, -10.0f});          // N s, changes the velocity at once
body->applyImpulseAt(impulse, worldPoint);

body->setLinearVelocity({0.0f, 5.0f, 0.0f});
body->setAngularVelocity({0.0f, 3.14f, 0.0f});     // rad/s around the world axes
body->setPosition({0.0f, 10.0f, 0.0f});            // teleport; the node follows at once
body->setRotation(glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0)));
```

Forces act during one step only and are cleared after it. A force that should act all the time, such as a spring, a thruster or wind, is applied in a [step callback](events.md#forces-every-step), which runs before every fixed step. Impulses and velocities take effect at once, which suits jumps, explosions and throws.

The body reports its state as of the latest step: `position()`, `rotation()`, `linearVelocity()`, `angularVelocity()`, `velocityAt(point)`, `centerOfMass()` and `kineticEnergy()`. The node shows the interpolated state, a fraction of a step earlier.

## Properties at run time

Every property in `BodyDesc` can be changed later. Each setter wakes the body:

```cpp
body->setMass(5.0f);                     // the inertia is scaled with it
body->setFriction(0.1f);                 // ice
body->setBounciness(0.9f);
body->setDamping(0.0f, 0.5f);
body->setGravityEnabled(false);          // floats
body->setType(BodyType::Kinematic);      // now moved through its node
```

Static and kinematic bodies have infinite mass: `mass()` returns 0 for them.

## Sleeping

A body that comes to rest falls asleep: it is no longer simulated until something touches it, a joint pulls it, or you change it. This keeps large piles cheap. `isSleeping()`, `wakeUp()` and `setCanSleep(false)` control it per body, and `PhysicsSettings::sleeping` for the whole world. `world->awakeBodyCount()` tells how many bodies are still moving, for example to decide when a scene has settled.

## Size and accuracy

Think of a unit as a metre: the engine works best with objects between about 0.1 and 10 units in size, and with touching bodies whose masses are not too different (a heavy box on a light one makes the stack soft). Very thin or very fast bodies can tunnel through thin walls in one step: make walls thicker than the distance a body travels in one step (speed / 240), or use a smaller `fixedTimeStep`. The default `velocityIterations` (10) and `positionIterations` (5) of `PhysicsSettings` give stable stacks. Raise them for tall towers; lower them for many loose bodies.
