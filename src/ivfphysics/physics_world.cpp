#include "engine.h"

#include <ivf/logger.h>
#include <ivf/transform_node.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/gtc/constants.hpp>

using namespace ivfphysics;
using detail::toGlm;
using detail::toRp;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::shared_ptr<detail::Engine> detail::engine()
{
    // Shared while any shape or world exists, so shapes can move between worlds
    static std::weak_ptr<Engine> s_engine;
    auto engine = s_engine.lock();
    if (!engine)
    {
        engine = std::make_shared<Engine>();
        s_engine = engine;
    }
    return engine;
}

glm::quat detail::rotationBetween(const glm::vec3 &from, const glm::vec3 &to)
{
    const float d = glm::dot(from, to);
    if (d > 1.0f - 1e-6f)
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    if (d < -1.0f + 1e-6f)
    {
        // Half turn around any axis perpendicular to from
        glm::vec3 axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), from);
        if (glm::dot(axis, axis) < 1e-6f)
            axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), from);
        return glm::angleAxis(glm::pi<float>(), glm::normalize(axis));
    }
    const glm::vec3 c = glm::cross(from, to);
    return glm::normalize(glm::quat(1.0f + d, c.x, c.y, c.z));
}

glm::quat detail::nodeRotation(const ivf::TransformNode &node)
{
    // Same order as TransformNode::localTransform(): Euler X, Y, Z, axis rotation, quaternion
    const glm::vec3 e = glm::radians(node.eulerAngles());
    glm::quat q = glm::angleAxis(e.x, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::angleAxis(e.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
                  glm::angleAxis(e.z, glm::vec3(0.0f, 0.0f, 1.0f));
    if (node.rotAngle() != 0.0f && glm::dot(node.rotAxis(), node.rotAxis()) > 0.0f)
        q = q * glm::angleAxis(glm::radians(node.rotAngle()), glm::normalize(node.rotAxis()));
    return glm::normalize(q * node.rotation());
}

void detail::setNodeTransform(ivf::TransformNode &node, const glm::vec3 &position, const glm::quat &rotation)
{
    node.setPos(position);
    node.setEulerAngles(glm::vec3(0.0f));
    node.setRotAngle(0.0f);
    node.setRotation(rotation);
}

namespace {

void detachBody(RigidBody::Impl &body)
{
    body.world = nullptr;
    body.body = nullptr;
    body.collider = nullptr;
}

} // namespace

// ---------------------------------------------------------------------------
// PhysicsWorld
// ---------------------------------------------------------------------------

PhysicsWorld::PhysicsWorld(const PhysicsSettings &settings) : m_impl(std::make_unique<Impl>())
{
    auto &s = m_impl->settings;
    s = settings;
    s.fixedTimeStep = std::clamp(s.fixedTimeStep, 1e-5, 1.0);
    s.maxSubSteps = std::max(s.maxSubSteps, 1);

    rp3d::PhysicsWorld::WorldSettings ws;
    ws.gravity = toRp(s.gravity);
    ws.defaultVelocitySolverNbIterations = static_cast<rp3d::uint16>(std::max(s.velocityIterations, 1));
    ws.defaultPositionSolverNbIterations = static_cast<rp3d::uint16>(std::max(s.positionIterations, 1));
    ws.isSleepingEnabled = s.sleeping;
    m_impl->world = m_impl->engine->common.createPhysicsWorld(ws);
}

std::shared_ptr<PhysicsWorld> PhysicsWorld::create(const PhysicsSettings &settings)
{
    return std::shared_ptr<PhysicsWorld>(new PhysicsWorld(settings));
}

PhysicsWorld::~PhysicsWorld()
{
    // Destroying the engine world destroys its bodies; the RigidBody objects may live on
    m_impl->engine->common.destroyPhysicsWorld(m_impl->world);
    for (auto &body : m_impl->bodies)
        detachBody(body->impl());
}

const PhysicsSettings &PhysicsWorld::settings() const
{
    return m_impl->settings;
}

RigidBodyPtr PhysicsWorld::addBody(const std::shared_ptr<ivf::TransformNode> &node, const ShapePtr &shape,
                                   const BodyDesc &desc)
{
    if (!node || !shape)
        throw std::invalid_argument("PhysicsWorld::addBody: node and shape are required");

    if (auto existing = bodyOf(node))
        removeBody(existing);

    auto impl = std::make_unique<RigidBody::Impl>();
    impl->world = m_impl.get();
    impl->node = node;
    impl->shape = shape;
    impl->desc = desc;
    if (desc.type == BodyType::Dynamic && shape->type() == ShapeType::TriangleMesh)
    {
        ivf::logWarning("A triangle mesh cannot be dynamic - the body of '" + node->name() + "' is static", "Physics");
        impl->desc.type = BodyType::Static;
    }

    const glm::vec3 position = node->pos();
    const glm::quat rotation = detail::nodeRotation(*node);
    detail::setNodeTransform(*node, position, rotation);

    const rp3d::Transform transform(toRp(position), toRp(rotation));
    impl->initial = transform;
    impl->previous = transform;
    impl->body = m_impl->world->createRigidBody(transform);

    const auto &shapeImpl = shape->impl();
    impl->collider = impl->body->addCollider(
        shapeImpl.shape, rp3d::Transform(toRp(shapeImpl.offsetPosition), toRp(shapeImpl.offsetRotation)));
    auto &material = impl->collider->getMaterial();
    material.setFrictionCoefficient(std::max(desc.friction, 0.0f));
    material.setBounciness(std::clamp(desc.bounciness, 0.0f, 1.0f));

    impl->body->setType(detail::toRp(impl->desc.type));
    impl->applyMass();
    impl->body->setLinearDamping(std::max(desc.linearDamping, 0.0f));
    impl->body->setAngularDamping(std::max(desc.angularDamping, 0.0f));
    impl->body->enableGravity(desc.gravity);
    impl->body->setIsAllowedToSleep(desc.canSleep);

    auto body = RigidBodyPtr(new RigidBody(std::move(impl)));
    m_impl->bodies.push_back(body);
    return body;
}

ivf::Result<RigidBodyPtr> PhysicsWorld::addBody(const std::shared_ptr<ivf::TransformNode> &node,
                                                const BodyDesc &desc)
{
    auto shape = Shape::fromNode(node);
    if (!shape)
        return std::unexpected(shape.error());
    return addBody(node, *shape, desc);
}

void PhysicsWorld::removeBody(const RigidBodyPtr &bodyRef)
{
    const RigidBodyPtr body = bodyRef; // bodyRef may be an element of the list erased below
    if (!body || body->impl().world != m_impl.get())
        return;
    auto &bodies = m_impl->bodies;
    bodies.erase(std::remove(bodies.begin(), bodies.end(), body), bodies.end());
    m_impl->world->destroyRigidBody(body->impl().body);
    detachBody(body->impl());
}

void PhysicsWorld::clear()
{
    while (!m_impl->bodies.empty())
        removeBody(m_impl->bodies.back());
}

const std::vector<RigidBodyPtr> &PhysicsWorld::bodies() const
{
    return m_impl->bodies;
}

RigidBodyPtr PhysicsWorld::bodyOf(const std::shared_ptr<ivf::TransformNode> &node) const
{
    for (const auto &body : m_impl->bodies)
        if (body->impl().node == node)
            return body;
    return nullptr;
}

void PhysicsWorld::runStep()
{
    const auto h = static_cast<rp3d::decimal>(m_impl->settings.fixedTimeStep);
    for (auto &body : m_impl->bodies)
        body->impl().previous = body->impl().body->getTransform();
    m_impl->world->update(h);
    m_impl->steps++;
    m_impl->time = static_cast<double>(m_impl->steps) * m_impl->settings.fixedTimeStep;
}

void PhysicsWorld::syncNodes(double alpha)
{
    m_impl->alpha = alpha;
    const bool interpolate = m_impl->settings.interpolate;
    const auto a = static_cast<float>(alpha);
    for (auto &body : m_impl->bodies)
    {
        auto &b = body->impl();
        if (b.desc.type != BodyType::Dynamic)
            continue;
        const rp3d::Transform &current = b.body->getTransform();
        glm::vec3 position = toGlm(current.getPosition());
        glm::quat rotation = toGlm(current.getOrientation());
        if (interpolate && a < 1.0f)
        {
            position = glm::mix(toGlm(b.previous.getPosition()), position, a);
            rotation = glm::slerp(toGlm(b.previous.getOrientation()), rotation, a);
        }
        detail::setNodeTransform(*b.node, position, glm::normalize(rotation));
    }
}

int PhysicsWorld::step(double dt)
{
    if (m_impl->paused || !(dt > 0.0) || !std::isfinite(dt))
        return 0;

    // The simulation runs up to one step ahead of the shown time: the accumulator is the
    // shown time minus the simulated time, in (-h, 0] after the steps, and the nodes are
    // interpolated to the shown time. A small tolerance keeps frame times that are whole
    // multiples of the step (1/60 and 1/240) from being split unevenly by rounding.
    const double h = m_impl->settings.fixedTimeStep;
    m_impl->accumulator += dt * m_impl->timeScale;
    auto n = static_cast<int>(std::ceil(m_impl->accumulator / h - 1e-6));
    n = std::max(n, 0);
    if (n > m_impl->settings.maxSubSteps)
    {
        n = m_impl->settings.maxSubSteps;
        m_impl->accumulator = n * h; // drop the time the simulation cannot keep up with
    }
    m_impl->accumulator = std::clamp(m_impl->accumulator - n * h, -h, 0.0);

    if (n > 0)
    {
        // Kinematic bodies get the velocity that takes them to their node in n steps
        const double duration = n * h;
        for (auto &body : m_impl->bodies)
        {
            auto &b = body->impl();
            if (b.desc.type != BodyType::Kinematic)
                continue;
            const glm::vec3 target = b.node->pos();
            const glm::quat targetRotation = detail::nodeRotation(*b.node);
            const rp3d::Transform &current = b.body->getTransform();
            const glm::vec3 velocity = (target - toGlm(current.getPosition())) / static_cast<float>(duration);
            glm::quat delta = targetRotation * glm::inverse(toGlm(current.getOrientation()));
            if (delta.w < 0.0f)
                delta = -delta;
            const float angle = 2.0f * std::acos(std::clamp(delta.w, -1.0f, 1.0f));
            const float s = std::sqrt(std::max(1.0f - delta.w * delta.w, 0.0f));
            const glm::vec3 axis = s > 1e-6f ? glm::vec3(delta.x, delta.y, delta.z) / s : glm::vec3(0.0f);
            b.body->setLinearVelocity(toRp(velocity));
            b.body->setAngularVelocity(toRp(axis * (angle / static_cast<float>(duration))));
        }
        for (int i = 0; i < n; ++i)
            runStep();
        // Kinematic bodies end exactly at their node
        for (auto &body : m_impl->bodies)
        {
            auto &b = body->impl();
            if (b.desc.type == BodyType::Kinematic)
                b.body->setTransform(rp3d::Transform(toRp(b.node->pos()), toRp(detail::nodeRotation(*b.node))));
        }
    }
    syncNodes(1.0 + m_impl->accumulator / h);
    return n;
}

void PhysicsWorld::stepOnce()
{
    runStep();
    m_impl->accumulator = 0.0;
    syncNodes(1.0);
}

void PhysicsWorld::setPaused(bool paused)
{
    m_impl->paused = paused;
}

bool PhysicsWorld::isPaused() const
{
    return m_impl->paused;
}

void PhysicsWorld::setTimeScale(double scale)
{
    m_impl->timeScale = std::max(scale, 0.0);
}

double PhysicsWorld::timeScale() const
{
    return m_impl->timeScale;
}

void PhysicsWorld::reset()
{
    for (auto &body : m_impl->bodies)
    {
        auto &b = body->impl();
        b.body->setTransform(b.initial);
        b.previous = b.initial;
        b.body->setLinearVelocity(rp3d::Vector3(0, 0, 0));
        b.body->setAngularVelocity(rp3d::Vector3(0, 0, 0));
        b.body->resetForce();
        b.body->resetTorque();
        if (b.desc.type != BodyType::Static)
            b.body->setIsSleeping(false);
        detail::setNodeTransform(*b.node, toGlm(b.initial.getPosition()), toGlm(b.initial.getOrientation()));
    }
    m_impl->accumulator = 0.0;
    m_impl->time = 0.0;
    m_impl->steps = 0;
    m_impl->alpha = 1.0;
}

void PhysicsWorld::setGravity(const glm::vec3 &gravity)
{
    m_impl->settings.gravity = gravity;
    rp3d::Vector3 g = toRp(gravity);
    m_impl->world->setGravity(g);
    for (auto &body : m_impl->bodies)
        if (body->impl().desc.type == BodyType::Dynamic)
            body->impl().body->setIsSleeping(false);
}

glm::vec3 PhysicsWorld::gravity() const
{
    return m_impl->settings.gravity;
}

double PhysicsWorld::time() const
{
    return m_impl->time;
}

std::uint64_t PhysicsWorld::stepCount() const
{
    return m_impl->steps;
}

double PhysicsWorld::interpolationFactor() const
{
    return m_impl->alpha;
}

int PhysicsWorld::awakeBodyCount() const
{
    return static_cast<int>(std::count_if(m_impl->bodies.begin(), m_impl->bodies.end(), [](const RigidBodyPtr &b) {
        return b->impl().desc.type == BodyType::Dynamic && !b->impl().body->isSleeping();
    }));
}
