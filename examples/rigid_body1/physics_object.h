/**
 * @file physics_object.h
 * @brief Base physics object class for ReactPhysics3D integration
 * @author Jonas Lindemann
 */

#pragma once

#include <memory>
#include <reactphysics3d/reactphysics3d.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace rp3d = reactphysics3d;

/**
 * @class PhysicsObject
 * @brief Base class for physics-enabled objects using ReactPhysics3D
 * 
 * This class wraps ReactPhysics3D's RigidBody and provides a convenient
 * interface for managing physics objects with position, rotation, and
 * collision shapes.
 */
class PhysicsObject {
protected:
    rp3d::RigidBody* m_body;           ///< ReactPhysics3D rigid body
    rp3d::Collider* m_collider;        ///< ReactPhysics3D collider
    rp3d::PhysicsWorld* m_world;       ///< Reference to physics world
    
    glm::vec3 m_size;                  ///< Object size
    glm::vec3 m_position;              ///< Object position
    glm::quat m_rotation;              ///< Object rotation
    
    float m_mass;                      ///< Object mass
    float m_restitution;               ///< Bounciness (0-1)
    float m_friction;                  ///< Friction coefficient
    
    bool m_isStatic;                   ///< Whether object is static (immovable)

public:
    /**
     * @brief Constructor
     * @param world Pointer to ReactPhysics3D world
     * @param position Initial position
     * @param rotation Initial rotation
     * @param mass Mass of the object (0 for static objects)
     */
    PhysicsObject(rp3d::PhysicsWorld* world, 
                  const glm::vec3& position = glm::vec3(0.0f),
                  const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                  float mass = 1.0f);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~PhysicsObject();
    
    /**
     * @brief Create the collision shape (must be implemented by derived classes)
     * @return Pointer to the created collision shape
     */
    virtual rp3d::CollisionShape* createCollisionShape() = 0;
    
    /**
     * @brief Initialize the physics object
     * 
     * Creates the rigid body, collision shape, and collider.
     * Call this after construction to set up the physics.
     */
    virtual void initialize();
    
    /**
     * @brief Update visual position from physics simulation
     */
    void updateFromPhysics();
    
    /**
     * @brief Get the current position
     * @return Current position as glm::vec3
     */
    glm::vec3 getPosition() const;
    
    /**
     * @brief Get the current rotation
     * @return Current rotation as glm::quat
     */
    glm::quat getRotation() const;
    
    /**
     * @brief Get the current rotation as Euler angles
     * @return Rotation as Euler angles (radians)
     */
    glm::vec3 getEulerAngles() const;
    
    /**
     * @brief Set the position
     * @param position New position
     */
    void setPosition(const glm::vec3& position);
    
    /**
     * @brief Set the rotation
     * @param rotation New rotation as quaternion
     */
    void setRotation(const glm::quat& rotation);
    
    /**
     * @brief Set the rotation using Euler angles
     * @param euler Euler angles in radians (pitch, yaw, roll)
     */
    void setEulerAngles(const glm::vec3& euler);
    
    /**
     * @brief Apply a force to the center of mass
     * @param force Force vector
     */
    void applyForce(const glm::vec3& force);
    
    /**
     * @brief Apply a force at a specific point
     * @param force Force vector
     * @param point Point of application (world coordinates)
     */
    void applyForceAtPoint(const glm::vec3& force, const glm::vec3& point);
    
    /**
     * @brief Apply a torque
     * @param torque Torque vector
     */
    void applyTorque(const glm::vec3& torque);
    
    /**
     * @brief Set the velocity
     * @param velocity Velocity vector
     */
    void setVelocity(const glm::vec3& velocity);
    
    /**
     * @brief Get the velocity
     * @return Current velocity
     */
    glm::vec3 getVelocity() const;
    
    /**
     * @brief Set the angular velocity
     * @param angularVelocity Angular velocity vector
     */
    void setAngularVelocity(const glm::vec3& angularVelocity);
    
    /**
     * @brief Get the angular velocity
     * @return Current angular velocity
     */
    glm::vec3 getAngularVelocity() const;
    
    /**
     * @brief Set the mass
     * @param mass New mass (0 for static objects)
     */
    void setMass(float mass);
    
    /**
     * @brief Get the mass
     * @return Current mass
     */
    float getMass() const { return m_mass; }
    
    /**
     * @brief Set the restitution (bounciness)
     * @param restitution Restitution coefficient (0-1)
     */
    void setRestitution(float restitution);
    
    /**
     * @brief Get the restitution
     * @return Current restitution
     */
    float getRestitution() const { return m_restitution; }
    
    /**
     * @brief Set the friction coefficient
     * @param friction Friction coefficient
     */
    void setFriction(float friction);
    
    /**
     * @brief Get the friction
     * @return Current friction
     */
    float getFriction() const { return m_friction; }
    
    /**
     * @brief Get the rigid body pointer
     * @return Pointer to ReactPhysics3D rigid body
     */
    rp3d::RigidBody* getRigidBody() { return m_body; }
    
    /**
     * @brief Get the collider pointer
     * @return Pointer to ReactPhysics3D collider
     */
    rp3d::Collider* getCollider() { return m_collider; }
    
protected:
    /**
     * @brief Convert glm::vec3 to rp3d::Vector3
     */
    static rp3d::Vector3 toRp3dVec3(const glm::vec3& v);
    
    /**
     * @brief Convert rp3d::Vector3 to glm::vec3
     */
    static glm::vec3 toGlmVec3(const rp3d::Vector3& v);
    
    /**
     * @brief Convert glm::quat to rp3d::Quaternion
     */
    static rp3d::Quaternion toRp3dQuat(const glm::quat& q);
    
    /**
     * @brief Convert rp3d::Quaternion to glm::quat
     */
    static glm::quat toGlmQuat(const rp3d::Quaternion& q);
};

using PhysicsObjectPtr = std::shared_ptr<PhysicsObject>;
