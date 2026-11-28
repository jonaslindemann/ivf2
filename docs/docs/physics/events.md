# Queries and Events

A world answers questions about its bodies and reports what happens to them. Rays find bodies along a line. Callbacks report contacts and trigger overlaps after each step. Step callbacks apply forces in every step, and statistics show what the simulation costs. `physics_sandbox1` uses rays to grab and throw bodies; `rigid_body1` shows the statistics.

## Raycasts

```cpp
ivfphysics::RaycastHit hit = world->raycast(ivf::Ray{origin, direction});
if (hit)
    ivf::logInfo(ivf::logStr("hit ", hit.body->node()->name(), " at distance ", hit.distance));
```

A hit has the `body`, the world `point`, the surface `normal` and the `distance` from the origin. It converts to `false` when nothing was hit. The direction need not be unit length. `raycastAll()` returns every body along the ray, nearest first, one hit per body. `RaycastOptions` narrows the search:

```cpp
ivfphysics::RaycastOptions options;
options.maxDistance = 50.0f;
options.mask = 0x0001;                                        // only the scene category
options.triggers = false;                                     // the default: triggers are not hit
options.skip = [&](const ivfphysics::RigidBodyPtr &b) { return b == player; };
auto below = world->raycast(ivf::Ray{player->position(), {0.0f, -1.0f, 0.0f}}, options);
```

In a `GLFWSceneWindow`, `cameraRay(x, y)` gives the ray from the camera through a window position, so `world->raycast(cameraRay(mouseX(), mouseY()))` is the body under the mouse. Rays use the state of the latest step, which the nodes show a fraction of a step later. For anything you click on, the difference is too small to notice.

## Contacts

```cpp
world->setContactCallback([this](const ivfphysics::Contact &c) {
    if (c.phase == ivfphysics::ContactPhase::Begin && c.impactSpeed > 2.0f)
        playClack(c.points.front().position, c.impactSpeed);
});
```

The callback is called after each fixed step, once per pair of bodies that started (`Begin`) or stopped (`End`) touching in it. With `PhysicsSettings::stayEvents` it is also called for pairs that keep touching (`Stay`, every step: many calls for a resting pile). A `Contact` holds:

- `body1`, `body2`: the two bodies; `other(body)` gives the one that is not `body`.
- `points`: the contact points, each with a `position`, a `normal` from the first body towards the second, and a penetration `depth`. `End` contacts have no points.
- `impactSpeed`: how fast the bodies approached each other along the normal before the step, for sounds, sparks and damage.

Events are collected during the step and delivered after it, outside the engine. That makes it safe to change the world in a callback: remove a body that was hit, add debris, break a joint. A removed body gets `End` events for its contacts after the next step, so anything that counts touching pairs stays consistent.

`world->contactPoints()` lists all contact points after the latest step, without callbacks: useful for drawing them (the [debug view](interaction.md#the-debug-view) does).

!!! note "Sleeping bodies keep their contacts"
    Bodies that fall asleep while touching report no `End`: the contact still exists, it is just not simulated. It ends when one of them wakes up and moves away. A new contact begins one step after the bodies first touch, once the engine has confirmed the pair.

## Triggers

A trigger is a body that reports what enters and leaves it but does not collide: a goal area, a checkpoint, a kill zone below the level.

```cpp
auto zone = ivf::Box::create({1.0f, 1.0f, 1.0f});
zone->setPos({6.8f, 1.0f, 0.0f});
zone->setVisible(false);                     // or a translucent material
scene->add(zone);
auto goal = world->addBody(zone, ivfphysics::Shape::box({1.0f, 1.0f, 1.0f}),
                           {.type = ivfphysics::BodyType::Static, .trigger = true});

world->setTriggerCallback([this](const ivfphysics::TriggerEvent &e) {
    if (e.phase == ivfphysics::ContactPhase::Begin)
        ++m_score;                           // e.trigger is the zone, e.other what entered it
});
```

Triggers are usually static or kinematic (move the node to move the zone): a dynamic trigger would fall through everything. `setTrigger(true)` turns an existing body into one. Rays pass through triggers unless `RaycastOptions::triggers` is set.

## Collision filtering

Each body has a 16-bit category and a mask of the categories it collides with. Two bodies collide only if each one's category is in the other's mask:

```cpp
constexpr std::uint16_t Scene = 1, Projectile = 2, Ghost = 4;

// Projectiles hit the scene but pass through each other
world->addBody(ball, ivfphysics::Shape::sphere(0.2f),
               {.mass = 1.0f, .category = Projectile, .collidesWith = 0xFFFF & ~Projectile});

// A ghost collides with the scene only
ghostBody->setCollisionFilter(Ghost, Scene);
```

Raycasts take a mask too (`RaycastOptions::mask`), so a ray can look for the scene only and ignore projectiles. Bodies that are joined do not collide with each other unless the joint says so (`JointDesc::collideConnected`).

## Forces every step

Forces applied with `applyForce()` act for one step only. A force that acts all the time belongs in a step callback, which the world calls before every fixed step with the step length:

```cpp
// A spring pulling a body towards a point, and wind on everything awake
int id = world->addStepCallback([=](double /*dt*/) {
    const glm::vec3 stretch = anchor - body->position();
    body->applyForce(40.0f * stretch - 4.0f * body->linearVelocity());

    for (const auto &b : world->bodies())
        if (b->type() == ivfphysics::BodyType::Dynamic && !b->isSleeping())
            b->applyForce({3.0f, 0.0f, 0.0f});
});

world->removeStepCallback(id);   // when it is no longer needed
```

A force applied in `onUpdate()` would act only in the first step of the frame and depend on the frame rate. A step callback acts the same at any frame rate. `DragController` pulls dragged bodies this way.

## Statistics

```cpp
const ivfphysics::PhysicsStats s = world->stats();
ImGui::Text("Bodies %d, dynamic %d, awake %d", s.bodies, s.dynamicBodies, s.awakeBodies);
ImGui::Text("Contacts %d (%d points)", s.contacts, s.contactPoints);
ImGui::Text("Update %.2f ms (%d steps)", s.averageUpdateTimeMs, s.stepsLastUpdate);
```

`updateTimeMs` is the time of the last `step()` call, events included; `averageUpdateTimeMs` is smoothed for display. `stepsLastUpdate` is usually 4 at 60 frames per second with the default 240 steps per second. `contacts` counts touching pairs of colliders: a [compound body](joints.md#compound-bodies) touching the ground with four legs counts four. `time()` and `stepCount()` give the simulated time and the number of steps since the start or the last `reset()`.
