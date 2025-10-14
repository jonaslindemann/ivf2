/**
 * @file physics_object.cpp
 * @brief Implementation of PhysicsObject base class
 * @author Jonas Lindemann
 */

#include "physics_object.h"
#include <glm/gtc/quaternion.hpp>

PhysicsObject::PhysicsObject(rp3d::PhysicsWorld* world, 
                             const glm::vec3& position,
                             const glm::quat& rotation,
                             float mass)
    : m_world(world)
    , m_body(nullptr)
    , m_collider(nullptr)
    , m_position(position)
    , m_rotation(rotation)
    , m_mass(mass)
    , m_restitution(0.5f)
    , m_friction(0.3f)
    , m_isStatic(mass == 0.0f)
{
}

PhysicsObject::~PhysicsObject()
{
    if (m_world && m_body) {
        m_world->destroyRigidBody(m_body);
        m_body = nullptr;
        m_collider = nullptr;
    }
}

void PhysicsObject::initialize()
{
    // Create transform
    rp3d::Vector3 rp3dPos = toRp3dVec3(m_position);
    rp3d::Quaternion rp3dRot = toRp3dQuat(m_rotation);
    rp3d::Transform transform(rp3dPos, rp3dRot);
    
    // Create rigid body
    m_body = m_world->createRigidBody(transform);
    
    // Set body type
    if (m_isStatic) {
        m_body->setType(rp3d::BodyType::STATIC);
    } else {
        m_body->setType(rp3d::BodyType::DYNAMIC);
    }
    
    // Create collision shape (implemented by derived classes)
    rp3d::CollisionShape* shape = createCollisionShape();
    
    if (shape) {
        // Add collider to body
        m_collider = m_body->addCollider(shape, rp3d::Transform::identity());
        
        // Set material properties
        rp3d::Material& material = m_collider->getMaterial();
        material.setBounciness(m_restitution);
        material.setFrictionCoefficient(m_friction);
        
        // Set mass if dynamic
        if (!m_isStatic && m_mass > 0.0f) {
            m_body->setMass(m_mass);
        }
    }
}

void PhysicsObject::updateFromPhysics()
{
    if (!m_body) return;
    
    const rp3d::Transform& transform = m_body->getTransform();
    m_position = toGlmVec3(transform.getPosition());
    m_rotation = toGlmQuat(transform.getOrientation());
}

glm::vec3 PhysicsObject::getPosition() const
{
    return m_position;
}

glm::quat PhysicsObject::getRotation() const
{
    return m_rotation;
}

glm::vec3 PhysicsObject::getEulerAngles() const
{
    return glm::eulerAngles(m_rotation);
}

void PhysicsObject::setPosition(const glm::vec3& position)
{
    m_position = position;
    if (m_body) {
        rp3d::Transform transform = m_body->getTransform();
        transform.setPosition(toRp3dVec3(position));
        m_body->setTransform(transform);
    }
}

void PhysicsObject::setRotation(const glm::quat& rotation)
{
    m_rotation = rotation;
    if (m_body) {
        rp3d::Transform transform = m_body->getTransform();
        transform.setOrientation(toRp3dQuat(rotation));
        m_body->setTransform(transform);
    }
}

void PhysicsObject::setEulerAngles(const glm::vec3& euler)
{
    setRotation(glm::quat(euler));
}

void PhysicsObject::applyForce(const glm::vec3& force)
{
    if (m_body && !m_isStatic) {
        m_body->applyWorldForceAtCenterOfMass(toRp3dVec3(force));
    }
}

void PhysicsObject::applyForceAtPoint(const glm::vec3& force, const glm::vec3& point)
{
    if (m_body && !m_isStatic) {
        m_body->applyWorldForceAtWorldPosition(toRp3dVec3(force), toRp3dVec3(point));
    }
}

void PhysicsObject::applyTorque(const glm::vec3& torque)
{
    if (m_body && !m_isStatic) {
        m_body->applyWorldTorque(toRp3dVec3(torque));
    }
}

void PhysicsObject::setVelocity(const glm::vec3& velocity)
{
    if (m_body && !m_isStatic) {
        m_body->setLinearVelocity(toRp3dVec3(velocity));
    }
}

glm::vec3 PhysicsObject::getVelocity() const
{
    if (m_body) {
        return toGlmVec3(m_body->getLinearVelocity());
    }
    return glm::vec3(0.0f);
}

void PhysicsObject::setAngularVelocity(const glm::vec3& angularVelocity)
{
    if (m_body && !m_isStatic) {
        m_body->setAngularVelocity(toRp3dVec3(angularVelocity));
    }
}

glm::vec3 PhysicsObject::getAngularVelocity() const
{
    if (m_body) {
        return toGlmVec3(m_body->getAngularVelocity());
    }
    return glm::vec3(0.0f);
}

void PhysicsObject::setMass(float mass)
{
    m_mass = mass;
    m_isStatic = (mass == 0.0f);
    
    if (m_body) {
        if (m_isStatic) {
            m_body->setType(rp3d::BodyType::STATIC);
        } else {
            m_body->setType(rp3d::BodyType::DYNAMIC);
            m_body->setMass(mass);
        }
    }
}

void PhysicsObject::setRestitution(float restitution)
{
    m_restitution = restitution;
    if (m_collider) {
        rp3d::Material& material = m_collider->getMaterial();
        material.setBounciness(restitution);
    }
}

void PhysicsObject::setFriction(float friction)
{
    m_friction = friction;
    if (m_collider) {
        rp3d::Material& material = m_collider->getMaterial();
        material.setFrictionCoefficient(friction);
    }
}

// Helper conversion functions
rp3d::Vector3 PhysicsObject::toRp3dVec3(const glm::vec3& v)
{
    return rp3d::Vector3(v.x, v.y, v.z);
}

glm::vec3 PhysicsObject::toGlmVec3(const rp3d::Vector3& v)
{
    return glm::vec3(v.x, v.y, v.z);
}

rp3d::Quaternion PhysicsObject::toRp3dQuat(const glm::quat& q)
{
    return rp3d::Quaternion(q.x, q.y, q.z, q.w);
}

glm::quat PhysicsObject::toGlmQuat(const rp3d::Quaternion& q)
{
    return glm::quat(q.w, q.x, q.y, q.z);
}
