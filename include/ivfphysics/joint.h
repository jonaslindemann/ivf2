#pragma once

/**
 * @file joint.h
 * @brief Joints between bodies: ball and socket, hinge, slider and fixed.
 */

#include <ivfphysics/rigid_body.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivfphysics {

/** @brief The kind of a joint. */
enum class JointType {
    BallSocket, ///< The bodies share a point and rotate freely about it (optional cone limit).
    Hinge,      ///< Rotation about one axis (door, wheel, pendulum); limits and motor.
    Slider,     ///< Translation along one axis (piston, drawer); limits and motor.
    Fixed,      ///< The bodies move as one (glue); breakable welds.
};

/** @brief Readable name of a joint type, e.g. "hinge". */
[[nodiscard]] const char *toString(JointType type);

/** @brief Properties of a new joint (`PhysicsWorld::addHingeJoint()` etc.). */
struct JointDesc {
    bool collideConnected{false}; ///< Let the two joined bodies collide with each other.
    float breakForce{0.0f};       ///< Break when the joint's force exceeds this (N); 0: never.
    float breakTorque{0.0f};      ///< Break when the joint's torque exceeds this (N m); 0: never.
};

/**
 * @class Joint
 * @brief A constraint between two bodies, or between a body and the world.
 *
 * Created by the `PhysicsWorld::add...Joint()` functions from an anchor point (and an axis)
 * in world coordinates, taken in the bodies' poses at that moment: limits and angles are
 * measured from that pose. Angles are in radians.
 *
 * A joint with a break force or torque breaks when the force (torque) it needs to hold
 * exceeds it: the joint stops acting, `isBroken()` turns true and the world's joint break
 * callback is called after the step. `PhysicsWorld::reset()` mends broken joints.
 */
class Joint {
public:
    struct Impl;

    ~Joint();
    Joint(const Joint &) = delete;
    Joint &operator=(const Joint &) = delete;

    /** @brief The kind of joint. */
    [[nodiscard]] JointType type() const;

    /** @brief The first body. */
    [[nodiscard]] RigidBodyPtr body1() const;

    /** @brief The second body; nullptr if the joint holds the first body to the world. */
    [[nodiscard]] RigidBodyPtr body2() const;

    /** @brief False once removed or its world destroyed (a broken joint stays valid). */
    [[nodiscard]] bool isValid() const;

    /** @brief True after the joint broke (until `PhysicsWorld::reset()`). */
    [[nodiscard]] bool isBroken() const;

    /** @brief The anchor point now (world coordinates, moving with the first body). */
    [[nodiscard]] glm::vec3 anchor() const;

    /** @brief The joint axis now (world coordinates; hinge and slider). */
    [[nodiscard]] glm::vec3 axis() const;

    /**
     * @brief Force the joint applied in the last step (N): on the second body, or on the first
     *        if the joint holds it to the world.
     */
    [[nodiscard]] glm::vec3 reactionForce() const;

    /** @brief Torque the joint applied in the last step (N m), on the same body as `reactionForce()`. */
    [[nodiscard]] glm::vec3 reactionTorque() const;

    /**
     * @brief Hinge angle (radians, -pi to pi) since creation: of the second body relative to the
     *        first, or of the first relative to the world. 0 for other joints.
     */
    [[nodiscard]] float angle() const;

    /** @brief Slider translation since creation (relative as `angle()`); 0 for other joints. */
    [[nodiscard]] float translation() const;

    /** @brief Current angle between the ball and socket joint's rest direction and the bodies (radians). */
    [[nodiscard]] float coneAngle() const;

    /**
     * @brief Limit the hinge angle (radians) or slider translation to [@p lower, @p upper].
     *
     * The range must contain 0 (the pose at creation); values are clamped to it. Hinge
     * limits lie within one turn (-2 pi to 2 pi). No effect on other joints.
     */
    void setLimits(float lower, float upper);

    /** @brief Limit a ball and socket joint to a cone of @p halfAngle (radians) around its rest direction. */
    void setConeLimit(float halfAngle);

    /** @brief Turn the limits (or cone limit) on or off. */
    void setLimitsEnabled(bool enabled);
    [[nodiscard]] bool limitsEnabled() const;
    [[nodiscard]] float lowerLimit() const;
    [[nodiscard]] float upperLimit() const;
    [[nodiscard]] float coneLimit() const;

    /**
     * @brief Drive a hinge at @p speed (radians/s) with at most @p maxForce torque (N m), or a
     *        slider at @p speed (units/s) with at most @p maxForce force (N). Turns the motor on.
     *
     * The speed is that of the second body relative to the first (of the first relative to the
     * world for a joint held to the world), about or along the joint axis.
     */
    void setMotor(float speed, float maxForce);

    /** @brief Turn the motor on or off. */
    void setMotorEnabled(bool enabled);
    [[nodiscard]] bool motorEnabled() const;
    [[nodiscard]] float motorSpeed() const;
    [[nodiscard]] float maxMotorForce() const;

    /** @brief Break thresholds (0: never). */
    void setBreakForce(float force);
    void setBreakTorque(float torque);
    [[nodiscard]] float breakForce() const;
    [[nodiscard]] float breakTorque() const;

    /** @brief Engine data (implementation detail of ivfphysics). */
    [[nodiscard]] Impl &impl() const
    {
        return *m_impl;
    }

private:
    friend class PhysicsWorld;
    explicit Joint(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> m_impl;
};

using JointPtr = std::shared_ptr<Joint>;

} // namespace ivfphysics
