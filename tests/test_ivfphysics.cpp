// ivfphysics without OpenGL: worlds, bodies, shapes, stepping, node sync, determinism, reset,
// raycasts, contact and trigger events, collision filters, step callbacks, dragging, throwing.

#include "test_main.h"

#include <ivfphysics/ivfphysics.h>

#include <ivf/box.h>
#include <ivf/capsule.h>
#include <ivf/cube.h>
#include <ivf/cylinder.h>
#include <ivf/sphere.h>

#include <glm/gtc/quaternion.hpp>

#include <cmath>
#include <vector>

using namespace ivfphysics;

namespace {

bool near(const glm::vec3 &a, const glm::vec3 &b, float eps = 1e-3f)
{
    return glm::length(a - b) <= eps;
}

bool near(const glm::quat &a, const glm::quat &b, float eps = 1e-3f)
{
    return std::fabs(std::fabs(glm::dot(a, b)) - 1.0f) <= eps;
}

/** A static ground plane at y = 0 on a node of its own. */
RigidBodyPtr addGround(PhysicsWorld &world)
{
    return world.addBody(ivf::Cube::create(), Shape::plane(), {.type = BodyType::Static});
}

std::shared_ptr<ivf::Cube> cubeAt(const glm::vec3 &pos)
{
    auto cube = ivf::Cube::create();
    cube->setPos(pos);
    return cube;
}

/** A small stack of boxes, knocked over by a sphere; returns the final transforms. */
std::vector<glm::vec3> runScene(int frames)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    std::vector<std::shared_ptr<ivf::Cube>> nodes;
    for (int i = 0; i < 5; ++i)
    {
        nodes.push_back(cubeAt({0.05f * static_cast<float>(i), 0.5f + static_cast<float>(i), 0.0f}));
        world->addBody(nodes.back(), Shape::box(glm::vec3(0.5f)), {.mass = 1.0f});
    }
    auto ball = cubeAt({-4.0f, 2.5f, 0.0f});
    auto ballBody = world->addBody(ball, Shape::sphere(0.4f), {.mass = 3.0f});
    ballBody->setLinearVelocity({8.0f, 0.0f, 0.0f});
    for (int i = 0; i < frames; ++i)
        world->step(1.0 / 60.0);
    std::vector<glm::vec3> result;
    for (const auto &body : world->bodies())
    {
        result.push_back(body->node()->pos());
        const glm::quat q = body->node()->rotation();
        result.push_back({q.x, q.y, q.z});
    }
    return result;
}

} // namespace

IVF_TEST(free_fall_follows_gravity)
{
    auto world = PhysicsWorld::create();
    auto cube = cubeAt({0.0f, 10.0f, 0.0f});
    auto body = world->addBody(cube, Shape::box(glm::vec3(0.5f)), {.mass = 2.0f, .linearDamping = 0.0f});
    IVF_CHECK_NEAR(body->mass(), 2.0f, 1e-4);

    int steps = 0;
    for (int i = 0; i < 60; ++i)
        steps += world->step(1.0 / 60.0);
    IVF_CHECK(steps == 240);
    IVF_CHECK(world->stepCount() == 240);
    IVF_CHECK_NEAR(world->time(), 1.0, 1e-9);
    // y = y0 - g t^2 / 2 (semi-implicit Euler is off by g t h / 2)
    IVF_CHECK_NEAR(body->position().y, 10.0f - 0.5f * 9.81f, 0.03);
    IVF_CHECK_NEAR(body->linearVelocity().y, -9.81f, 1e-3);
    IVF_CHECK(near(cube->pos(), body->position()));
}

IVF_TEST(box_lands_on_ground)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto cube = cubeAt({0.0f, 3.0f, 0.0f});
    auto body = world->addBody(cube, Shape::box(glm::vec3(0.5f)));
    for (int i = 0; i < 240; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK_NEAR(cube->pos().y, 0.5f, 0.02);
    IVF_CHECK_NEAR(cube->pos().x, 0.0f, 0.01);
    IVF_CHECK(glm::length(body->linearVelocity()) < 0.05f);
    IVF_CHECK(near(cube->rotation(), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 1e-3f));
    IVF_CHECK(body->isSleeping());
    IVF_CHECK(world->awakeBodyCount() == 0);
}

IVF_TEST(sphere_rolls_down_tilted_plane)
{
    auto world = PhysicsWorld::create();
    const glm::vec3 normal = glm::normalize(glm::vec3(0.3f, 1.0f, 0.0f));
    world->addBody(ivf::Cube::create(), Shape::plane(normal), {.type = BodyType::Static});
    auto ball = cubeAt(normal * 0.5f);
    world->addBody(ball, Shape::sphere(0.5f));
    for (int i = 0; i < 60; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK(ball->pos().x > 0.5f); // downhill is +x
    // Still resting on the plane: centre one radius above it
    IVF_CHECK_NEAR(glm::dot(ball->pos(), normal), 0.5f, 0.02);
}

IVF_TEST(nodes_are_interpolated_between_steps)
{
    PhysicsSettings settings;
    settings.gravity = glm::vec3(0.0f);
    auto world = PhysicsWorld::create(settings);
    auto cube = cubeAt({0.0f, 0.0f, 0.0f});
    auto body = world->addBody(cube, Shape::box(glm::vec3(0.5f)), {.angularDamping = 0.0f});
    body->setLinearVelocity({2.4f, 0.0f, 0.0f}); // 0.01 per step of 1/240 s

    // Half a step: one step is run, the node is shown half way
    IVF_CHECK(world->step(1.0 / 480.0) == 1);
    IVF_CHECK_NEAR(world->interpolationFactor(), 0.5, 1e-6);
    IVF_CHECK(near(cube->pos(), {0.005f, 0.0f, 0.0f}, 1e-5f));
    IVF_CHECK(near(body->position(), {0.01f, 0.0f, 0.0f}, 1e-5f));
    // The next half step is already simulated
    IVF_CHECK(world->step(1.0 / 480.0) == 0);
    IVF_CHECK(near(cube->pos(), {0.01f, 0.0f, 0.0f}, 1e-5f));
    IVF_CHECK(world->step(1.0 / 480.0) == 1);
    IVF_CHECK(near(cube->pos(), {0.015f, 0.0f, 0.0f}, 1e-5f));
    IVF_CHECK(world->step(1.0 / 480.0) == 0);

    world->setPaused(true);
    IVF_CHECK(world->step(1.0) == 0);
    world->stepOnce();
    IVF_CHECK(near(cube->pos(), {0.03f, 0.0f, 0.0f}, 1e-5f));
    world->setPaused(false);
    IVF_CHECK(world->step(10.0) == world->settings().maxSubSteps); // the rest is dropped
}

IVF_TEST(same_run_gives_same_result)
{
    const auto a = runScene(180);
    const auto b = runScene(180);
    IVF_CHECK(a.size() == b.size());
    bool same = true;
    for (std::size_t i = 0; i < a.size(); ++i)
        same = same && a[i] == b[i];
    IVF_CHECK(same);
    // ... and the ball did knock something over
    IVF_CHECK(a[2].x > 0.5f || a[4].x > 0.5f || a[8].x > 0.5f);
}

IVF_TEST(reset_restores_initial_state)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto cube = cubeAt({1.0f, 4.0f, 2.0f});
    cube->setEulerAngles(0.0f, 90.0f, 0.0f);
    auto body = world->addBody(cube, Shape::box({0.5f, 0.25f, 1.0f}));
    // Euler angles are folded into the rotation
    IVF_CHECK(near(cube->eulerAngles(), glm::vec3(0.0f)));
    const glm::quat start = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    IVF_CHECK(near(body->rotation(), start));

    body->setAngularVelocity({1.0f, 2.0f, 3.0f});
    for (int i = 0; i < 90; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK(cube->pos().y < 3.0f);

    world->reset();
    IVF_CHECK(world->time() == 0.0 && world->stepCount() == 0);
    IVF_CHECK(near(cube->pos(), {1.0f, 4.0f, 2.0f}));
    IVF_CHECK(near(cube->rotation(), start));
    IVF_CHECK(near(body->linearVelocity(), glm::vec3(0.0f)));
    IVF_CHECK(near(body->angularVelocity(), glm::vec3(0.0f)));
    world->step(1.0 / 60.0);
    IVF_CHECK(cube->pos().y < 4.0f); // falling again
}

IVF_TEST(impulses_change_momentum)
{
    PhysicsSettings settings;
    settings.gravity = glm::vec3(0.0f);
    auto world = PhysicsWorld::create(settings);
    auto body = world->addBody(cubeAt({0.0f, 0.0f, 0.0f}), Shape::sphere(1.0f), {.mass = 2.0f});
    body->applyImpulse({4.0f, 0.0f, 0.0f});
    IVF_CHECK(near(body->linearVelocity(), {2.0f, 0.0f, 0.0f}));
    // Off-centre: spins about z. A solid sphere has I = 2/5 m r^2 = 0.8
    body->applyImpulseAt({0.0f, 0.8f, 0.0f}, {1.0f, 0.0f, 0.0f});
    IVF_CHECK(near(body->linearVelocity(), {2.0f, 0.4f, 0.0f}));
    IVF_CHECK(near(body->angularVelocity(), {0.0f, 0.0f, 1.0f}, 2e-3f));
    IVF_CHECK(near(body->velocityAt({0.0f, 1.0f, 0.0f}), {1.0f, 0.4f, 0.0f}, 2e-3f));
    IVF_CHECK_NEAR(body->kineticEnergy(), 0.5f * 2.0f * (4.0f + 0.16f) + 0.5f * 0.8f, 5e-3);

    // A constant force for one second: dv = F t / m
    auto pushed = world->addBody(cubeAt({5.0f, 0.0f, 0.0f}), Shape::box(glm::vec3(0.5f)), {.mass = 1.0f});
    for (int i = 0; i < 240; ++i)
    {
        pushed->applyForce({0.0f, 0.0f, 3.0f});
        world->stepOnce();
    }
    IVF_CHECK_NEAR(pushed->linearVelocity().z, 3.0f, 1e-3);
}

IVF_TEST(kinematic_bodies_follow_their_node)
{
    auto world = PhysicsWorld::create();
    auto paddle = cubeAt({0.0f, 0.5f, 0.0f});
    auto body = world->addBody(paddle, Shape::box(glm::vec3(0.5f)), {.type = BodyType::Kinematic});
    paddle->setPos({1.0f, 0.5f, 0.0f});
    paddle->setRotation(glm::angleAxis(0.5f, glm::vec3(0.0f, 1.0f, 0.0f)));
    world->step(1.0 / 60.0);
    IVF_CHECK(near(body->position(), {1.0f, 0.5f, 0.0f}));
    IVF_CHECK(near(body->rotation(), glm::angleAxis(0.5f, glm::vec3(0.0f, 1.0f, 0.0f))));
    IVF_CHECK(near(paddle->pos(), {1.0f, 0.5f, 0.0f})); // the world does not move the node
    IVF_CHECK(body->mass() == 0.0f);
}

IVF_TEST(shapes_from_nodes)
{
    auto sphere = ivf::Sphere::create(0.75);
    auto s = Shape::fromNode(sphere);
    IVF_CHECK(s && (*s)->type() == ShapeType::Sphere);
    IVF_CHECK(s && std::fabs((*s)->radius() - 0.75f) < 1e-3f);

    auto box = ivf::Box::create({1.0f, 2.0f, 3.0f});
    box->setScale({2.0f, 1.0f, 1.0f});
    auto b = Shape::fromNode(box);
    IVF_CHECK(b && (*b)->type() == ShapeType::Box);
    const glm::vec3 half = 0.5f * box->localBoundingBox().size() * glm::vec3(2.0f, 1.0f, 1.0f);
    IVF_CHECK(b && near((*b)->halfExtents(), half));

    auto cube = ivf::Cube::create();
    auto c = Shape::fromNode(cube);
    IVF_CHECK(c && (*c)->type() == ShapeType::Box);

    auto capsule = ivf::Capsule::create(0.5, 1.0);
    auto cap = Shape::fromNode(capsule);
    IVF_CHECK(cap && (*cap)->type() == ShapeType::Capsule);
    IVF_CHECK(cap && std::fabs((*cap)->radius() - 0.5f) < 1e-2f);
    IVF_CHECK(cap && std::fabs((*cap)->height() + 2.0f * (*cap)->radius() - capsule->localBoundingBox().size().y) < 1e-2f);

    auto cylinder = ivf::Cylinder::create(0.5, 2.0);
    auto hull = Shape::fromNode(cylinder);
    IVF_CHECK(hull && (*hull)->type() == ShapeType::ConvexHull);
    if (hull)
    {
        const auto [lo, hi] = (*hull)->bounds();
        IVF_CHECK(near(lo, cylinder->localBoundingBox().min(), 1e-2f));
        IVF_CHECK(near(hi, cylinder->localBoundingBox().max(), 1e-2f));
        IVF_CHECK_NEAR((*hull)->volume(), 3.14159 * 0.25 * 2.0, 0.05); // 32 slices: close to the cylinder
    }

    auto mesh = Shape::fromNode(cylinder, true);
    IVF_CHECK(mesh && (*mesh)->type() == ShapeType::TriangleMesh && !(*mesh)->triangles().empty());

    IVF_CHECK(!Shape::convexHull(std::vector<glm::vec3>{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}));
    IVF_CHECK(!Shape::fromNode(nullptr));
}

IVF_TEST(convex_hulls)
{
    // A box mesh repeats its corners per face and has points inside its faces: 8 corners remain
    auto box = Shape::fromNode(ivf::Box::create({1.0f, 0.5f, 0.25f}, {4, 4, 4}), false);
    IVF_CHECK(box.has_value());
    auto boxHull = Shape::convexHull(ivf::Box::create({1.0f, 0.5f, 0.25f}, {4, 4, 4})->mesh());
    IVF_CHECK(boxHull && (*boxHull)->points().size() == 8);
    IVF_CHECK(boxHull && std::fabs((*boxHull)->volume() - 1.0f) < 1e-3f);

    // Points inside, on faces and duplicated do not matter
    std::vector<glm::vec3> points{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.1f, 0.1f, 0.1f},
                                  {0.5f, 0.5f, 0.0f}, {1, 0, 0}, {0.2f, 0.2f, 0.2f}};
    auto tetra = Shape::convexHull(points);
    IVF_CHECK(tetra && (*tetra)->points().size() == 4);
    IVF_CHECK(tetra && std::fabs((*tetra)->volume() - 1.0f / 6.0f) < 1e-4f);

    // Flat or too few points fail
    IVF_CHECK(!Shape::convexHull(std::vector<glm::vec3>{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {2, 3, 0}}));

    // A dense sphere (33 000 vertices) is reduced to its extreme points and stays close
    auto sphere = ivf::Sphere::create(1.0, 256, 128);
    auto hull = Shape::convexHull(sphere->mesh());
    IVF_CHECK(hull.has_value());
    IVF_CHECK(hull && (*hull)->points().size() <= 2048);
    IVF_CHECK(hull && std::fabs((*hull)->volume() - 4.18879f) < 0.03f);

    // A hull body rolls like a ball
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto ball = cubeAt({0.0f, 1.0f, 0.0f});
    world->addBody(ball, *hull);
    for (int i = 0; i < 60; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK_NEAR(ball->pos().y, 1.0f, 0.02);
}

IVF_TEST(shapes_and_bodies)
{
    auto world = PhysicsWorld::create();
    addGround(*world);

    // A dynamic body with a triangle mesh becomes static
    auto cylinder = ivf::Cylinder::create(0.5, 2.0);
    auto concave = Shape::fromNode(cylinder, true);
    IVF_CHECK(concave.has_value());
    auto fixed = world->addBody(cylinder, *concave);
    IVF_CHECK(fixed->type() == BodyType::Static);

    // A convex hull body falls onto the ground and rests on its flat end
    auto can = ivf::Cylinder::create(0.5, 1.0);
    can->setPos({3.0f, 2.0f, 0.0f});
    auto canBody = world->addBody(can);
    IVF_CHECK(canBody.has_value());
    for (int i = 0; i < 180; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK_NEAR(can->pos().y, 0.5f, 0.03);

    // Shared shapes, adding a node twice replaces its body, removing invalidates
    auto shape = Shape::box(glm::vec3(0.5f));
    auto n1 = cubeAt({-3.0f, 1.0f, 0.0f});
    auto first = world->addBody(n1, shape);
    auto second = world->addBody(n1, shape);
    IVF_CHECK(!first->isValid() && second->isValid());
    IVF_CHECK(world->bodyOf(n1) == second);
    const auto count = world->bodies().size();
    world->removeBody(second);
    IVF_CHECK(!second->isValid() && world->bodies().size() == count - 1);
    IVF_CHECK(world->bodyOf(n1) == nullptr);
    world->clear();
    IVF_CHECK(world->bodies().empty());

    // Bodies outlive their world safely
    RigidBodyPtr orphan;
    {
        auto other = PhysicsWorld::create();
        orphan = other->addBody(cubeAt({0.0f, 0.0f, 0.0f}), shape);
    }
    IVF_CHECK(!orphan->isValid());
    orphan->applyImpulse({1.0f, 0.0f, 0.0f}); // no effect, no crash
    IVF_CHECK(orphan->linearVelocity() == glm::vec3(0.0f));
}

IVF_TEST(material_and_settings)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto ball = cubeAt({0.0f, 2.0f, 0.0f});
    auto body = world->addBody(ball, Shape::sphere(0.5f), {.bounciness = 1.0f});
    body->setFriction(-1.0f);
    IVF_CHECK(body->friction() == 0.0f);
    body->setBounciness(2.0f);
    IVF_CHECK(body->bounciness() == 1.0f);
    body->setMass(5.0f);
    IVF_CHECK_NEAR(body->mass(), 5.0f, 1e-4);

    // A bouncy ball keeps bouncing
    float highest = 0.0f;
    for (int i = 0; i < 180; ++i)
    {
        world->step(1.0 / 60.0);
        if (i > 60)
            highest = std::max(highest, ball->pos().y);
    }
    IVF_CHECK(highest > 1.2f);

    world->setGravity({0.0f, 0.0f, 0.0f});
    IVF_CHECK(world->gravity() == glm::vec3(0.0f));
    world->setTimeScale(0.5);
    IVF_CHECK(world->step(1.0 / 60.0) == 2);
}

IVF_TEST(raycast_finds_nearest_and_all)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto near1 = world->addBody(cubeAt({0.0f, 0.5f, 0.0f}), Shape::box(glm::vec3(0.5f)), {});
    auto far1 = world->addBody(cubeAt({4.0f, 0.5f, 0.0f}), Shape::box(glm::vec3(0.5f)), {.category = 0x0002});

    const ivf::Ray ray{{-10.0f, 0.5f, 0.0f}, {2.0f, 0.0f, 0.0f}}; // direction need not be unit length
    auto hit = world->raycast(ray);
    IVF_CHECK(static_cast<bool>(hit));
    IVF_CHECK(hit.body == near1);
    IVF_CHECK_NEAR(hit.distance, 9.5f, 1e-3);
    IVF_CHECK(near(hit.point, {-0.5f, 0.5f, 0.0f}));
    IVF_CHECK(near(hit.normal, {-1.0f, 0.0f, 0.0f}));

    const auto all = world->raycastAll(ray);
    IVF_CHECK(all.size() == 2);
    IVF_CHECK(all.size() == 2 && all[0].body == near1 && all[1].body == far1);
    IVF_CHECK(all.size() == 2 && all[0].distance < all[1].distance);

    // Category mask, skip, range, miss
    IVF_CHECK(world->raycast(ray, {.mask = 0x0002}).body == far1);
    IVF_CHECK(world->raycast(ray, {.skip = [&](const RigidBodyPtr &b) { return b == near1; }}).body == far1);
    IVF_CHECK(!world->raycast(ray, {.maxDistance = 5.0f}));
    IVF_CHECK(!world->raycast({{0.0f, 5.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}));
    IVF_CHECK(!world->raycast({{0.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}));

    // Straight down onto the ground beside the boxes
    auto ground = world->raycast({{2.0f, 5.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
    IVF_CHECK(ground && ground.body->type() == BodyType::Static);
    IVF_CHECK_NEAR(ground.distance, 5.0f, 1e-3);

    // Bodies that moved are hit where they are now
    near1->setPosition({0.0f, 3.0f, 0.0f});
    IVF_CHECK(world->raycast(ray).body == far1);
}

IVF_TEST(contact_events_begin_and_end)
{
    auto world = PhysicsWorld::create();
    auto ground = addGround(*world);
    auto ballNode = cubeAt({0.0f, 2.5f, 0.0f});
    auto ball = world->addBody(ballNode, Shape::sphere(0.5f), {.bounciness = 0.0f});

    std::vector<Contact> events;
    world->setContactCallback([&](const Contact &c) { events.push_back(c); });

    for (int i = 0; i < 120; ++i)
        world->step(1.0 / 60.0);
    // It lands (and may bounce a little: Begin, End, Begin); Stay is not reported by default
    IVF_CHECK(!events.empty());
    if (!events.empty())
    {
        const auto &c = events.front();
        IVF_CHECK(c.phase == ContactPhase::Begin);
        IVF_CHECK(c.other(ball) == ground && c.other(ground) == ball);
        IVF_CHECK(!c.points.empty());
        // Dropped from 2 m: about sqrt(2 g h) = 6.3 m/s
        IVF_CHECK(c.impactSpeed > 5.5f && c.impactSpeed < 6.6f);
        IVF_CHECK(!c.points.empty() && std::fabs(c.points[0].position.y) < 0.05f);
        IVF_CHECK(events.back().phase == ContactPhase::Begin);
    }
    for (const auto &c : events)
        IVF_CHECK(c.phase != ContactPhase::Stay);
    // At rest the ball sleeps: falling asleep ends nothing, but sleeping contacts are not listed
    IVF_CHECK(ball->isSleeping());

    // Lifted off the ground: End
    ball->setPosition({0.0f, 3.0f, 0.0f});
    world->stepOnce();
    IVF_CHECK(!events.empty() && events.back().phase == ContactPhase::End);
    IVF_CHECK(world->stats().contacts == 0);

    // Removing a body in contact ends its contacts after the next step
    for (int i = 0; i < 120; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK(events.back().phase == ContactPhase::Begin);
    const auto count = events.size();
    world->removeBody(ball);
    IVF_CHECK(events.size() == count); // not before the step
    world->stepOnce();
    IVF_CHECK(events.size() == count + 1 && events.back().phase == ContactPhase::End && events.back().body1 &&
              events.back().body2);

    // Stay events when asked for
    auto stayWorld = PhysicsWorld::create({.stayEvents = true});
    addGround(*stayWorld);
    stayWorld->addBody(cubeAt({0.0f, 0.5f, 0.0f}), Shape::box(glm::vec3(0.5f)), {.canSleep = false});
    int begins = 0;
    int stays = 0;
    stayWorld->setContactCallback([&](const Contact &c) {
        begins += c.phase == ContactPhase::Begin;
        stays += c.phase == ContactPhase::Stay;
    });
    for (int i = 0; i < 10; ++i)
        stayWorld->stepOnce();
    // The first step finds the pair, the second its contact
    IVF_CHECK(begins == 1);
    IVF_CHECK(stays == 8);
    IVF_CHECK(stayWorld->stats().contacts == 1);
    IVF_CHECK(stayWorld->stats().contactPoints == 4);
    IVF_CHECK(stayWorld->contactPoints().size() == 4);
}

IVF_TEST(callbacks_may_remove_bodies)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    std::vector<RigidBodyPtr> balls;
    for (int i = 0; i < 4; ++i)
        balls.push_back(world->addBody(cubeAt({2.0f * static_cast<float>(i), 1.0f, 0.0f}), Shape::sphere(0.5f), {}));

    // Each ball is removed when it lands
    int ends = 0;
    world->setContactCallback([&](const Contact &c) {
        if (c.phase == ContactPhase::End)
        {
            ends++;
            return;
        }
        for (const auto &body : {c.body1, c.body2})
            if (body->type() == BodyType::Dynamic && body->isValid())
                world->removeBody(body);
    });
    for (int i = 0; i < 120; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK(world->bodies().size() == 1);
    IVF_CHECK(ends == 4);
    for (const auto &b : balls)
        IVF_CHECK(!b->isValid());
}

IVF_TEST(trigger_reports_enter_and_leave)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto zone = world->addBody(ivf::Box::create(), Shape::box({2.0f, 0.5f, 2.0f}),
                               {.type = BodyType::Static, .trigger = true});
    zone->setPosition({0.0f, 3.0f, 0.0f});
    IVF_CHECK(zone->isTrigger());
    auto ballNode = cubeAt({0.0f, 6.0f, 0.0f});
    auto ball = world->addBody(ballNode, Shape::sphere(0.25f), {.bounciness = 0.0f});

    std::vector<TriggerEvent> events;
    int contacts = 0;
    world->setTriggerCallback([&](const TriggerEvent &e) { events.push_back(e); });
    world->setContactCallback([&](const Contact &c) {
        if (c.phase == ContactPhase::Begin && (c.body1 == zone || c.body2 == zone))
            contacts++;
    });
    for (int i = 0; i < 180; ++i)
        world->step(1.0 / 60.0);

    // It fell through the trigger onto the ground
    IVF_CHECK_NEAR(ballNode->pos().y, 0.25f, 0.02);
    IVF_CHECK(contacts == 0);
    IVF_CHECK(events.size() == 2);
    if (events.size() == 2)
    {
        IVF_CHECK(events[0].phase == ContactPhase::Begin && events[0].trigger == zone && events[0].other == ball);
        IVF_CHECK(events[1].phase == ContactPhase::End && events[1].trigger == zone && events[1].other == ball);
    }

    // Raycasts pass through triggers unless asked
    const ivf::Ray down{{0.0f, 10.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};
    IVF_CHECK(world->raycast(down).body == ball);
    IVF_CHECK(world->raycast(down, {.triggers = true}).body == zone);
}

IVF_TEST(collision_filter_lets_groups_pass)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto lowerNode = cubeAt({0.0f, 0.5f, 0.0f});
    auto upperNode = cubeAt({0.0f, 2.5f, 0.0f});
    auto lower = world->addBody(lowerNode, Shape::box(glm::vec3(0.5f)), {.collidesWith = 0xFFFF & ~0x0002});
    auto upper = world->addBody(upperNode, Shape::box(glm::vec3(0.5f)), {.category = 0x0002});
    IVF_CHECK(upper->collisionCategory() == 0x0002 && upper->collisionMask() == 0xFFFF);
    IVF_CHECK(lower->collisionMask() == (0xFFFF & ~0x0002));

    for (int i = 0; i < 120; ++i)
        world->step(1.0 / 60.0);
    // Passed through the lower box onto the ground
    IVF_CHECK_NEAR(upperNode->pos().y, 0.5f, 0.02);
    IVF_CHECK_NEAR(lowerNode->pos().y, 0.5f, 0.02);

    // Filters can change: now they collide and push apart
    lower->setCollisionFilter(0x0001, 0xFFFF);
    upper->setPosition({0.0f, 2.5f, 0.0f});
    for (int i = 0; i < 120; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK_NEAR(upperNode->pos().y, 1.5f, 0.05);
}

IVF_TEST(step_callbacks_and_statistics)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto body = world->addBody(cubeAt({0.0f, 0.5f, 0.0f}), Shape::box(glm::vec3(0.5f)), {});

    int calls = 0;
    double total = 0.0;
    const int id = world->addStepCallback([&](double h) {
        calls++;
        total += h;
        body->applyForce({0.0f, 2.0f * 9.81f, 0.0f}); // lifts the 1 kg box at 9.81 m/s^2
    });
    IVF_CHECK(world->step(1.0 / 60.0) == 4);
    IVF_CHECK(calls == 4);
    IVF_CHECK_NEAR(total, 1.0 / 60.0, 1e-9);
    for (int i = 0; i < 59; ++i)
        world->step(1.0 / 60.0);
    // 1 s at 9.81 m/s^2 upwards: about 4.9 m
    IVF_CHECK_NEAR(body->position().y, 0.5f + 4.9f, 0.1);

    world->removeStepCallback(id);
    world->step(1.0 / 60.0);
    IVF_CHECK(calls == 240);

    const auto stats = world->stats();
    IVF_CHECK(stats.bodies == 2);
    IVF_CHECK(stats.dynamicBodies == 1);
    IVF_CHECK(stats.awakeBodies == 1);
    IVF_CHECK(stats.stepsLastUpdate == 4);
    IVF_CHECK(stats.updateTimeMs >= 0.0);
}

IVF_TEST(drag_controller_pulls_body)
{
    auto world = PhysicsWorld::create();
    addGround(*world);
    auto boxNode = cubeAt({0.0f, 0.5f, 0.0f});
    auto box = world->addBody(boxNode, Shape::box(glm::vec3(0.5f)), {.mass = 5.0f});
    auto drag = DragController::create(world);

    // The ground cannot be grabbed, nor empty space
    IVF_CHECK(!drag->grab({{3.0f, 5.0f, 0.0f}, {0.0f, -1.0f, 0.0f}}));
    IVF_CHECK(!drag->grab({{0.0f, 5.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}));
    IVF_CHECK(!drag->isDragging());

    // Grab the front face, then pull up and to the side
    IVF_CHECK(drag->grab({{0.0f, 0.5f, -5.0f}, {0.0f, 0.0f, 1.0f}}));
    IVF_CHECK(drag->isDragging() && drag->body() == box);
    IVF_CHECK_NEAR(drag->distance(), 4.5f, 1e-3);
    IVF_CHECK(near(drag->anchor(), {0.0f, 0.5f, -0.5f}));
    drag->drag({{0.0f, 0.5f, -5.0f}, glm::normalize(glm::vec3(1.0f, 2.0f, 4.5f))});
    const glm::vec3 target = drag->target();
    IVF_CHECK_NEAR(glm::length(target - glm::vec3(0.0f, 0.5f, -5.0f)), 4.5f, 1e-3);
    for (int i = 0; i < 180; ++i)
        world->step(1.0 / 60.0);
    // Hangs from the grabbed point (gravity sags it by about g / (2 pi f)^2 = 1 cm)
    IVF_CHECK(glm::length(drag->anchor() - target) < 0.05f);
    IVF_CHECK(box->position().y > 1.0f);

    // Released, it falls back down
    drag->release();
    IVF_CHECK(!drag->isDragging());
    for (int i = 0; i < 180; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK_NEAR(boxNode->pos().y, 0.5f, 0.05);

    // A removed body is let go
    IVF_CHECK(drag->grab({box->position() + glm::vec3(0.0f, 5.0f, 0.0f), {0.0f, -1.0f, 0.0f}}));
    world->removeBody(box);
    world->stepOnce();
    IVF_CHECK(!drag->isDragging());

    // The controller unregisters when destroyed
    drag.reset();
    world->stepOnce();
}

IVF_TEST(throw_body_along_ray)
{
    auto world = PhysicsWorld::create({.gravity = {0.0f, 0.0f, 0.0f}});
    auto ballNode = cubeAt({0.0f, 0.0f, 0.0f});
    auto ball = world->addBody(ballNode, Shape::sphere(0.25f), {.linearDamping = 0.0f, .angularDamping = 0.0f});
    throwBody(ball, {{1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, -2.0f}}, 10.0f, {0.0f, 5.0f, 0.0f});
    IVF_CHECK(near(ballNode->pos(), {1.0f, 2.0f, 3.0f}));
    IVF_CHECK(near(ball->linearVelocity(), {0.0f, 0.0f, -10.0f}));
    IVF_CHECK(near(ball->angularVelocity(), {0.0f, 5.0f, 0.0f}));
    for (int i = 0; i < 30; ++i)
        world->step(1.0 / 60.0);
    IVF_CHECK(near(ballNode->pos(), {1.0f, 2.0f, -2.0f}, 1e-2f));
}

IVF_TEST_MAIN()
