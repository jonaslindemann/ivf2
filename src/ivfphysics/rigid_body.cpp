#include "engine.h"

#include <ivf/logger.h>
#include <ivf/transform_node.h>

#include <algorithm>
#include <cmath>

using namespace ivfphysics;
using detail::toGlm;
using detail::toRp;

void RigidBody::Impl::applyMass()
{
    if (!body || desc.type != BodyType::Dynamic)
        return;
    const float volume = std::max(shape->volume(), 1e-9f);
    const float mass = std::max(desc.mass, 1e-6f);
    collider->getMaterial().setMassDensity(mass / volume);
    body->updateMassPropertiesFromColliders();
}

glm::vec3 RigidBody::Impl::inverseInertiaWorld(const glm::vec3 &v) const
{
    const glm::vec3 inertia = toGlm(body->getLocalInertiaTensor());
    const glm::quat q = toGlm(body->getTransform().getOrientation());
    const glm::vec3 local = glm::inverse(q) * v;
    const glm::vec3 scaled(inertia.x > 0.0f ? local.x / inertia.x : 0.0f, inertia.y > 0.0f ? local.y / inertia.y : 0.0f,
                           inertia.z > 0.0f ? local.z / inertia.z : 0.0f);
    return q * scaled;
}

RigidBody::RigidBody(std::unique_ptr<Impl> impl) : m_impl(std::move(impl))
{}

RigidBody::~RigidBody() = default;

std::shared_ptr<ivf::TransformNode> RigidBody::node() const
{
    return m_impl->node;
}

ShapePtr RigidBody::shape() const
{
    return m_impl->shape;
}

bool RigidBody::isValid() const
{
    return m_impl->body != nullptr;
}

BodyType RigidBody::type() const
{
    return m_impl->desc.type;
}

void RigidBody::setType(BodyType type)
{
    if (type == BodyType::Dynamic && m_impl->shape->type() == ShapeType::TriangleMesh)
    {
        ivf::logWarning("A triangle mesh body cannot be dynamic", "Physics");
        return;
    }
    m_impl->desc.type = type;
    if (!m_impl->body)
        return;
    m_impl->body->setType(toRp(type));
    m_impl->applyMass();
    m_impl->body->setIsSleeping(false);
}

glm::vec3 RigidBody::position() const
{
    return m_impl->body ? toGlm(m_impl->body->getTransform().getPosition()) : glm::vec3(0.0f);
}

glm::quat RigidBody::rotation() const
{
    return m_impl->body ? toGlm(m_impl->body->getTransform().getOrientation()) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

void RigidBody::setPosition(const glm::vec3 &position)
{
    if (!m_impl->body)
        return;
    rp3d::Transform transform = m_impl->body->getTransform();
    transform.setPosition(toRp(position));
    m_impl->body->setTransform(transform);
    m_impl->previous = transform;
    m_impl->body->setIsSleeping(false);
    detail::setNodeTransform(*m_impl->node, position, rotation());
}

void RigidBody::setRotation(const glm::quat &rotation)
{
    if (!m_impl->body)
        return;
    rp3d::Transform transform = m_impl->body->getTransform();
    transform.setOrientation(toRp(glm::normalize(rotation)));
    m_impl->body->setTransform(transform);
    m_impl->previous = transform;
    m_impl->body->setIsSleeping(false);
    detail::setNodeTransform(*m_impl->node, position(), glm::normalize(rotation));
}

glm::vec3 RigidBody::linearVelocity() const
{
    return m_impl->body ? toGlm(m_impl->body->getLinearVelocity()) : glm::vec3(0.0f);
}

void RigidBody::setLinearVelocity(const glm::vec3 &velocity)
{
    if (!m_impl->body || m_impl->desc.type == BodyType::Static)
        return;
    m_impl->body->setLinearVelocity(toRp(velocity));
    m_impl->body->setIsSleeping(false);
}

glm::vec3 RigidBody::angularVelocity() const
{
    return m_impl->body ? toGlm(m_impl->body->getAngularVelocity()) : glm::vec3(0.0f);
}

void RigidBody::setAngularVelocity(const glm::vec3 &velocity)
{
    if (!m_impl->body || m_impl->desc.type == BodyType::Static)
        return;
    m_impl->body->setAngularVelocity(toRp(velocity));
    m_impl->body->setIsSleeping(false);
}

glm::vec3 RigidBody::velocityAt(const glm::vec3 &point) const
{
    return linearVelocity() + glm::cross(angularVelocity(), point - centerOfMass());
}

void RigidBody::applyForce(const glm::vec3 &force)
{
    if (m_impl->body && m_impl->desc.type == BodyType::Dynamic)
        m_impl->body->applyWorldForceAtCenterOfMass(toRp(force));
}

void RigidBody::applyForceAt(const glm::vec3 &force, const glm::vec3 &point)
{
    if (m_impl->body && m_impl->desc.type == BodyType::Dynamic)
        m_impl->body->applyWorldForceAtWorldPosition(toRp(force), toRp(point));
}

void RigidBody::applyTorque(const glm::vec3 &torque)
{
    if (m_impl->body && m_impl->desc.type == BodyType::Dynamic)
        m_impl->body->applyWorldTorque(toRp(torque));
}

void RigidBody::applyImpulse(const glm::vec3 &impulse)
{
    if (!m_impl->body || m_impl->desc.type != BodyType::Dynamic)
        return;
    const float m = static_cast<float>(m_impl->body->getMass());
    if (m <= 0.0f)
        return;
    m_impl->body->setLinearVelocity(toRp(linearVelocity() + impulse / m));
    m_impl->body->setIsSleeping(false);
}

void RigidBody::applyImpulseAt(const glm::vec3 &impulse, const glm::vec3 &point)
{
    if (!m_impl->body || m_impl->desc.type != BodyType::Dynamic)
        return;
    applyImpulse(impulse);
    const glm::vec3 r = point - centerOfMass();
    m_impl->body->setAngularVelocity(toRp(angularVelocity() + m_impl->inverseInertiaWorld(glm::cross(r, impulse))));
}

float RigidBody::mass() const
{
    return m_impl->body && m_impl->desc.type == BodyType::Dynamic ? static_cast<float>(m_impl->body->getMass())
                                                                   : 0.0f;
}

void RigidBody::setMass(float mass)
{
    m_impl->desc.mass = mass;
    m_impl->applyMass();
}

glm::vec3 RigidBody::centerOfMass() const
{
    if (!m_impl->body)
        return glm::vec3(0.0f);
    return toGlm(m_impl->body->getTransform() * m_impl->body->getLocalCenterOfMass());
}

float RigidBody::friction() const
{
    return m_impl->desc.friction;
}

void RigidBody::setFriction(float friction)
{
    m_impl->desc.friction = std::max(friction, 0.0f);
    if (m_impl->collider)
        m_impl->collider->getMaterial().setFrictionCoefficient(m_impl->desc.friction);
}

float RigidBody::bounciness() const
{
    return m_impl->desc.bounciness;
}

void RigidBody::setBounciness(float bounciness)
{
    m_impl->desc.bounciness = std::clamp(bounciness, 0.0f, 1.0f);
    if (m_impl->collider)
        m_impl->collider->getMaterial().setBounciness(m_impl->desc.bounciness);
}

float RigidBody::linearDamping() const
{
    return m_impl->desc.linearDamping;
}

float RigidBody::angularDamping() const
{
    return m_impl->desc.angularDamping;
}

void RigidBody::setDamping(float linear, float angular)
{
    m_impl->desc.linearDamping = std::max(linear, 0.0f);
    m_impl->desc.angularDamping = std::max(angular, 0.0f);
    if (m_impl->body)
    {
        m_impl->body->setLinearDamping(m_impl->desc.linearDamping);
        m_impl->body->setAngularDamping(m_impl->desc.angularDamping);
    }
}

bool RigidBody::gravityEnabled() const
{
    return m_impl->desc.gravity;
}

void RigidBody::setGravityEnabled(bool enabled)
{
    m_impl->desc.gravity = enabled;
    if (m_impl->body)
        m_impl->body->enableGravity(enabled);
}

bool RigidBody::isSleeping() const
{
    return m_impl->body && m_impl->body->isSleeping();
}

void RigidBody::wakeUp()
{
    if (m_impl->body)
        m_impl->body->setIsSleeping(false);
}

void RigidBody::setCanSleep(bool canSleep)
{
    m_impl->desc.canSleep = canSleep;
    if (m_impl->body)
        m_impl->body->setIsAllowedToSleep(canSleep);
}

float RigidBody::kineticEnergy() const
{
    if (!m_impl->body || m_impl->desc.type != BodyType::Dynamic)
        return 0.0f;
    const glm::vec3 v = linearVelocity();
    const glm::vec3 w = angularVelocity();
    const glm::quat q = rotation();
    const glm::vec3 wl = glm::inverse(q) * w;
    const glm::vec3 inertia = toGlm(m_impl->body->getLocalInertiaTensor());
    return 0.5f * mass() * glm::dot(v, v) + 0.5f * glm::dot(wl * inertia, wl);
}

void RigidBody::setCollisionFilter(std::uint16_t category, std::uint16_t collidesWith)
{
    m_impl->desc.category = category;
    m_impl->desc.collidesWith = collidesWith;
    if (!m_impl->collider)
        return;
    // Wake the dynamic bodies first: the engine (0.10.2) keeps the pairs of sleeping bodies
    // apart and fails when a collider of such a pair asks for a new broad-phase check
    for (const auto &body : m_impl->world->bodies)
        if (body->impl().desc.type == BodyType::Dynamic)
            body->impl().body->setIsSleeping(false);
    m_impl->collider->setCollisionCategoryBits(category);
    m_impl->collider->setCollideWithMaskBits(collidesWith);
}

std::uint16_t RigidBody::collisionCategory() const
{
    return m_impl->desc.category;
}

std::uint16_t RigidBody::collisionMask() const
{
    return m_impl->desc.collidesWith;
}

void RigidBody::setTrigger(bool trigger)
{
    m_impl->desc.trigger = trigger;
    if (!m_impl->collider)
        return;
    m_impl->body->setIsSleeping(false);
    m_impl->collider->setIsTrigger(trigger);
}

bool RigidBody::isTrigger() const
{
    return m_impl->desc.trigger;
}
