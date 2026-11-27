#include "engine.h"

#include <glm/gtc/constants.hpp>

#include <algorithm>

using namespace ivfphysics;
using detail::toGlm;
using detail::toRp;

const char *ivfphysics::toString(JointType type)
{
    switch (type)
    {
    case JointType::BallSocket:
        return "ball and socket";
    case JointType::Hinge:
        return "hinge";
    case JointType::Slider:
        return "slider";
    case JointType::Fixed:
        break;
    }
    return "fixed";
}

// ---------------------------------------------------------------------------
// Joint::Impl
// ---------------------------------------------------------------------------

void Joint::Impl::create()
{
    if (!world || joint)
        return;
    // The engine measures angles, translations, motors and cones of its second body relative to
    // its first: a body held to the world is the second, after the world body
    rp3d::RigidBody *own = body1->impl().body;
    rp3d::RigidBody *b1 = body2 ? own : worldBody;
    rp3d::RigidBody *b2 = body2 ? body2->impl().body : own;
    const glm::vec3 p1 = toGlm(own->getTransform().getPosition());
    const glm::quat q1 = toGlm(own->getTransform().getOrientation());
    const rp3d::Vector3 anchor = toRp(p1 + q1 * localAnchor);
    const rp3d::Vector3 axis = toRp(glm::normalize(q1 * localAxis));
    auto *engineWorld = world->world;

    switch (type)
    {
    case JointType::BallSocket: {
        rp3d::BallAndSocketJointInfo info(b1, b2, anchor);
        info.isCollisionEnabled = desc.collideConnected;
        auto *j = static_cast<rp3d::BallAndSocketJoint *>(engineWorld->createJoint(info));
        if (limits)
        {
            j->setConeLimitHalfAngle(cone);
            j->enableConeLimit(true);
        }
        joint = j;
        break;
    }
    case JointType::Hinge: {
        rp3d::HingeJointInfo info(b1, b2, anchor, axis);
        info.isCollisionEnabled = desc.collideConnected;
        info.isLimitEnabled = limits;
        info.minAngleLimit = lower;
        info.maxAngleLimit = upper;
        info.isMotorEnabled = false; // driven by PhysicsWorld::runStep() (the engine's diverges)
        joint = engineWorld->createJoint(info);
        break;
    }
    case JointType::Slider: {
        rp3d::SliderJointInfo info(b1, b2, anchor, axis);
        info.isCollisionEnabled = desc.collideConnected;
        info.isLimitEnabled = limits;
        info.minTranslationLimit = lower;
        info.maxTranslationLimit = upper;
        info.isMotorEnabled = motor;
        info.motorSpeed = motorSpeed;
        info.maxMotorForce = maxMotor;
        joint = engineWorld->createJoint(info);
        break;
    }
    case JointType::Fixed: {
        rp3d::FixedJointInfo info(b1, b2, anchor);
        info.isCollisionEnabled = desc.collideConnected;
        joint = engineWorld->createJoint(info);
        break;
    }
    }
    broken = false;
}

void Joint::Impl::destroyJoint()
{
    if (world && joint)
        world->world->destroyJoint(joint);
    joint = nullptr;
}

// ---------------------------------------------------------------------------
// Joint
// ---------------------------------------------------------------------------

namespace {

rp3d::HingeJoint *hinge(const Joint::Impl &j)
{
    return j.type == JointType::Hinge && j.joint ? static_cast<rp3d::HingeJoint *>(j.joint) : nullptr;
}

rp3d::SliderJoint *slider(const Joint::Impl &j)
{
    return j.type == JointType::Slider && j.joint ? static_cast<rp3d::SliderJoint *>(j.joint) : nullptr;
}

rp3d::BallAndSocketJoint *ballSocket(const Joint::Impl &j)
{
    return j.type == JointType::BallSocket && j.joint ? static_cast<rp3d::BallAndSocketJoint *>(j.joint) : nullptr;
}

} // namespace

Joint::Joint(std::unique_ptr<Impl> impl) : m_impl(std::move(impl))
{}

Joint::~Joint() = default;

JointType Joint::type() const
{
    return m_impl->type;
}

RigidBodyPtr Joint::body1() const
{
    return m_impl->body1;
}

RigidBodyPtr Joint::body2() const
{
    return m_impl->body2;
}

bool Joint::isValid() const
{
    return m_impl->world != nullptr;
}

bool Joint::isBroken() const
{
    return m_impl->broken;
}

glm::vec3 Joint::anchor() const
{
    const auto &b = m_impl->body1;
    return b->position() + b->rotation() * m_impl->localAnchor;
}

glm::vec3 Joint::axis() const
{
    return glm::normalize(m_impl->body1->rotation() * m_impl->localAxis);
}

glm::vec3 Joint::reactionForce() const
{
    return m_impl->force;
}

glm::vec3 Joint::reactionTorque() const
{
    return m_impl->torque;
}

float Joint::angle() const
{
    auto *j = hinge(*m_impl);
    return j ? static_cast<float>(j->getAngle()) : 0.0f;
}

float Joint::translation() const
{
    auto *j = slider(*m_impl);
    return j ? static_cast<float>(j->getTranslation()) : 0.0f;
}

float Joint::coneAngle() const
{
    auto *j = ballSocket(*m_impl);
    return j ? static_cast<float>(j->getConeHalfAngle()) : 0.0f;
}

void Joint::setLimits(float lower, float upper)
{
    auto &j = *m_impl;
    if (j.type == JointType::Hinge)
    {
        const float turn = 2.0f * glm::pi<float>();
        j.lower = std::clamp(lower, -turn, 0.0f);
        j.upper = std::clamp(upper, 0.0f, turn);
    }
    else if (j.type == JointType::Slider)
    {
        j.lower = std::min(lower, 0.0f);
        j.upper = std::max(upper, 0.0f);
    }
    else
        return;
    j.limits = true;
    if (auto *h = hinge(j))
    {
        h->setMinAngleLimit(j.lower);
        h->setMaxAngleLimit(j.upper);
        h->enableLimit(true);
    }
    if (auto *s = slider(j))
    {
        s->setMinTranslationLimit(j.lower);
        s->setMaxTranslationLimit(j.upper);
        s->enableLimit(true);
    }
}

void Joint::setConeLimit(float halfAngle)
{
    auto &j = *m_impl;
    if (j.type != JointType::BallSocket)
        return;
    j.cone = std::clamp(halfAngle, 0.0f, glm::pi<float>());
    j.limits = true;
    if (auto *b = ballSocket(j))
    {
        b->setConeLimitHalfAngle(j.cone);
        b->enableConeLimit(true);
    }
}

void Joint::setLimitsEnabled(bool enabled)
{
    auto &j = *m_impl;
    if (j.type == JointType::Fixed)
        return;
    j.limits = enabled;
    if (auto *h = hinge(j))
        h->enableLimit(enabled);
    if (auto *s = slider(j))
        s->enableLimit(enabled);
    if (auto *b = ballSocket(j))
        b->enableConeLimit(enabled);
}

bool Joint::limitsEnabled() const
{
    return m_impl->limits;
}

float Joint::lowerLimit() const
{
    return m_impl->lower;
}

float Joint::upperLimit() const
{
    return m_impl->upper;
}

float Joint::coneLimit() const
{
    return m_impl->cone;
}

void Joint::setMotor(float speed, float maxForce)
{
    auto &j = *m_impl;
    if (j.type != JointType::Hinge && j.type != JointType::Slider)
        return;
    j.motorSpeed = speed;
    j.maxMotor = std::max(maxForce, 0.0f);
    j.motor = true;
    if (hinge(j))
        j.body1->wakeUp(); // hinge motors are applied by the world before each step
    if (auto *s = slider(j))
    {
        s->setMotorSpeed(j.motorSpeed);
        s->setMaxMotorForce(j.maxMotor);
        s->enableMotor(true);
    }
}

void Joint::setMotorEnabled(bool enabled)
{
    auto &j = *m_impl;
    if (j.type != JointType::Hinge && j.type != JointType::Slider)
        return;
    j.motor = enabled;
    if (hinge(j))
        j.body1->wakeUp();
    if (auto *s = slider(j))
        s->enableMotor(enabled);
}

bool Joint::motorEnabled() const
{
    return m_impl->motor;
}

float Joint::motorSpeed() const
{
    return m_impl->motorSpeed;
}

float Joint::maxMotorForce() const
{
    return m_impl->maxMotor;
}

void Joint::setBreakForce(float force)
{
    m_impl->desc.breakForce = std::max(force, 0.0f);
}

void Joint::setBreakTorque(float torque)
{
    m_impl->desc.breakTorque = std::max(torque, 0.0f);
}

float Joint::breakForce() const
{
    return m_impl->desc.breakForce;
}

float Joint::breakTorque() const
{
    return m_impl->desc.breakTorque;
}
