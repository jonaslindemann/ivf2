#pragma once

/**
 * @file rigid_body.h
 * @brief Rigid bodies attached to scene nodes.
 */

#include <ivfphysics/shape.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>

namespace ivf {
class TransformNode;
}

namespace ivfphysics {

class PhysicsWorld;

/** @brief How a body moves. */
enum class BodyType {
    Static,    ///< Never moves (ground, walls). Infinite mass.
    Kinematic, ///< Moved by the application through its node; pushes dynamic bodies.
    Dynamic,   ///< Moved by the simulation; drives its node.
};

/** @brief Properties of a new body (`PhysicsWorld::addBody()`). */
struct BodyDesc {
    BodyType type{BodyType::Dynamic}; ///< How the body moves.
    float mass{1.0f};                 ///< Mass of a dynamic body (kg); inertia follows from the shape.
    float friction{0.5f};             ///< Friction coefficient (>= 0).
    float bounciness{0.1f};           ///< Restitution, 0 (no bounce) to 1.
    float linearDamping{0.0f};        ///< Linear velocity damping (>= 0).
    float angularDamping{0.05f};      ///< Angular velocity damping (>= 0).
    bool gravity{true};               ///< Affected by the world's gravity.
    bool canSleep{true};              ///< May go to sleep when it comes to rest.
};

/**
 * @class RigidBody
 * @brief A simulated body attached to a `TransformNode`.
 *
 * Created by `PhysicsWorld::addBody()` and owned by the world. A dynamic body writes its
 * position and rotation to the node after each step (`TransformNode::setPos()`,
 * `setRotation()`; Euler angles and axis rotations of the node are folded into the
 * rotation when the body is added). A kinematic body follows its node: move the node and
 * the body gets the velocity that takes it there during the next step. Positions are
 * world coordinates, so the node should not have transformed parents.
 *
 * All values are those of the latest simulation step; the node shows the interpolated
 * state between the last two steps.
 */
class RigidBody {
public:
    struct Impl;

    ~RigidBody();
    RigidBody(const RigidBody &) = delete;
    RigidBody &operator=(const RigidBody &) = delete;

    /** @brief The node the body drives (or follows). */
    [[nodiscard]] std::shared_ptr<ivf::TransformNode> node() const;

    /** @brief The collision shape. */
    [[nodiscard]] ShapePtr shape() const;

    /** @brief False once the body was removed or its world destroyed. */
    [[nodiscard]] bool isValid() const;

    /** @brief How the body moves. */
    [[nodiscard]] BodyType type() const;

    /** @brief Change how the body moves (a triangle mesh cannot become dynamic). */
    void setType(BodyType type);

    /** @brief Position of the body origin (world coordinates). */
    [[nodiscard]] glm::vec3 position() const;

    /** @brief Rotation of the body. */
    [[nodiscard]] glm::quat rotation() const;

    /** @brief Move the body to @p position (teleport; wakes it). The node follows at once. */
    void setPosition(const glm::vec3 &position);

    /** @brief Rotate the body to @p rotation (teleport; wakes it). The node follows at once. */
    void setRotation(const glm::quat &rotation);

    /** @brief Linear velocity (units per second). */
    [[nodiscard]] glm::vec3 linearVelocity() const;

    /** @brief Set the linear velocity (dynamic bodies; wakes the body). */
    void setLinearVelocity(const glm::vec3 &velocity);

    /** @brief Angular velocity (radians per second around each world axis). */
    [[nodiscard]] glm::vec3 angularVelocity() const;

    /** @brief Set the angular velocity (dynamic bodies; wakes the body). */
    void setAngularVelocity(const glm::vec3 &velocity);

    /** @brief Velocity of the body at the world point @p point. */
    [[nodiscard]] glm::vec3 velocityAt(const glm::vec3 &point) const;

    /** @brief Apply @p force (N) at the centre of mass during the next step. */
    void applyForce(const glm::vec3 &force);

    /** @brief Apply @p force (N) at the world point @p point during the next step. */
    void applyForceAt(const glm::vec3 &force, const glm::vec3 &point);

    /** @brief Apply @p torque (N m) during the next step. */
    void applyTorque(const glm::vec3 &torque);

    /** @brief Change the momentum at once by @p impulse (N s) at the centre of mass. */
    void applyImpulse(const glm::vec3 &impulse);

    /** @brief Change the momentum at once by @p impulse (N s) at the world point @p point. */
    void applyImpulseAt(const glm::vec3 &impulse, const glm::vec3 &point);

    /** @brief Mass (0 for static and kinematic bodies). */
    [[nodiscard]] float mass() const;

    /** @brief Set the mass of a dynamic body; the inertia is scaled with it. */
    void setMass(float mass);

    /** @brief Centre of mass in world coordinates. */
    [[nodiscard]] glm::vec3 centerOfMass() const;

    /** @brief Friction coefficient. */
    [[nodiscard]] float friction() const;
    void setFriction(float friction);

    /** @brief Restitution (0 to 1). */
    [[nodiscard]] float bounciness() const;
    void setBounciness(float bounciness);

    /** @brief Linear and angular damping. */
    [[nodiscard]] float linearDamping() const;
    [[nodiscard]] float angularDamping() const;
    void setDamping(float linear, float angular);

    /** @brief Whether the world's gravity acts on the body. */
    [[nodiscard]] bool gravityEnabled() const;
    void setGravityEnabled(bool enabled);

    /** @brief True if the body has come to rest and is not simulated until woken. */
    [[nodiscard]] bool isSleeping() const;

    /** @brief Wake the body (it also wakes when touched or changed). */
    void wakeUp();

    /** @brief Allow or forbid sleeping. */
    void setCanSleep(bool canSleep);

    /** @brief Kinetic energy, 0.5 m v^2 + 0.5 w^T I w. */
    [[nodiscard]] float kineticEnergy() const;

    /** @brief Engine data (implementation detail of ivfphysics). */
    [[nodiscard]] Impl &impl() const
    {
        return *m_impl;
    }

private:
    friend class PhysicsWorld;
    explicit RigidBody(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> m_impl;
};

using RigidBodyPtr = std::shared_ptr<RigidBody>;

} // namespace ivfphysics
