#pragma once

/**
 * @file events.h
 * @brief Contacts, trigger overlaps, raycast results and statistics of a `PhysicsWorld`.
 */

#include <ivfphysics/rigid_body.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <functional>
#include <vector>

namespace ivfphysics {

/** @brief Where in its life a contact or overlap is. */
enum class ContactPhase {
    Begin, ///< The bodies started touching (overlapping) in this step.
    Stay,  ///< They touched before and still do (reported only with `PhysicsSettings::stayEvents`).
    End,   ///< They touched before and no longer do (or one of them was removed).
};

/** @brief "Begin", "Stay" or "End". */
[[nodiscard]] const char *toString(ContactPhase phase);

/** @brief One point of a contact, in world coordinates. */
struct ContactPoint {
    glm::vec3 position{0.0f}; ///< Point on the surface of the first body.
    glm::vec3 normal{0.0f};   ///< Unit normal, from the first body towards the second.
    float depth{0.0f};        ///< Penetration depth.
};

/**
 * @brief A contact between two colliding bodies (`PhysicsWorld::setContactCallback()`).
 */
struct Contact {
    ContactPhase phase{ContactPhase::Begin};
    RigidBodyPtr body1;
    RigidBodyPtr body2;
    std::vector<ContactPoint> points; ///< Contact points (none for End).
    /**
     * @brief Speed at which the bodies approached each other along the normal before the
     *        step (>= 0): how hard they hit. Useful for sounds and damage.
     */
    float impactSpeed{0.0f};

    /** @brief The other body of the contact, seen from @p body (nullptr if @p body is in neither). */
    [[nodiscard]] RigidBodyPtr other(const RigidBodyPtr &body) const
    {
        return body == body1 ? body2 : body == body2 ? body1 : nullptr;
    }
};

/** @brief A body entering or leaving a trigger (`PhysicsWorld::setTriggerCallback()`). */
struct TriggerEvent {
    ContactPhase phase{ContactPhase::Begin};
    RigidBodyPtr trigger; ///< The trigger body.
    RigidBodyPtr other;   ///< The body that entered or left it (may be a trigger too).
};

using ContactCallback = std::function<void(const Contact &)>;
using TriggerCallback = std::function<void(const TriggerEvent &)>;

/** @brief What `PhysicsWorld::raycast()` tests. */
struct RaycastOptions {
    float maxDistance{10000.0f}; ///< Ignore hits farther than this from the origin.
    std::uint16_t mask{0xFFFF};  ///< Collision categories that can be hit.
    bool triggers{false};        ///< Also hit trigger bodies.
    /// Leave bodies out (return true to skip), e.g. the one being dragged.
    std::function<bool(const RigidBodyPtr &)> skip;
};

/** @brief A raycast hit. Converts to false if nothing was hit. */
struct RaycastHit {
    RigidBodyPtr body;        ///< The body hit.
    glm::vec3 point{0.0f};    ///< Hit point (world coordinates).
    glm::vec3 normal{0.0f};   ///< Surface normal at the hit point.
    float distance{0.0f};     ///< Distance from the ray origin.

    explicit operator bool() const
    {
        return body != nullptr;
    }
};

/** @brief Statistics of a world (`PhysicsWorld::stats()`). */
struct PhysicsStats {
    int bodies{0};        ///< All bodies.
    int dynamicBodies{0}; ///< Dynamic bodies.
    int awakeBodies{0};   ///< Dynamic bodies not sleeping.
    int contacts{0};      ///< Touching body pairs after the last step.
    int contactPoints{0}; ///< Their contact points.
    int stepsLastUpdate{0}; ///< Fixed steps run by the last `step()` call.
    double updateTimeMs{0.0}; ///< Wall time of the last `step()` call (ms), events included.
    double averageUpdateTimeMs{0.0}; ///< Moving average of `updateTimeMs`.
};

} // namespace ivfphysics
