#pragma once

// Private to ivfphysics: the ReactPhysics3D side of shapes, bodies and worlds.

#include <ivfphysics/joint.h>
#include <ivfphysics/physics_world.h>
#include <ivfphysics/rigid_body.h>
#include <ivfphysics/shape.h>

#include <reactphysics3d/reactphysics3d.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rp3d = reactphysics3d;

namespace ivf {
class TransformNode;
}

namespace ivfphysics::detail {

/** One PhysicsCommon (the engine's factory) shared by all shapes and worlds while any exist. */
struct Engine {
    rp3d::PhysicsCommon common;
};

std::shared_ptr<Engine> engine();

inline rp3d::Vector3 toRp(const glm::vec3 &v)
{
    return {v.x, v.y, v.z};
}

inline glm::vec3 toGlm(const rp3d::Vector3 &v)
{
    return {v.x, v.y, v.z};
}

inline rp3d::Quaternion toRp(const glm::quat &q)
{
    return {q.x, q.y, q.z, q.w};
}

inline glm::quat toGlm(const rp3d::Quaternion &q)
{
    return {q.w, q.x, q.y, q.z};
}

inline rp3d::BodyType toRp(BodyType type)
{
    switch (type)
    {
    case BodyType::Static:
        return rp3d::BodyType::STATIC;
    case BodyType::Kinematic:
        return rp3d::BodyType::KINEMATIC;
    case BodyType::Dynamic:
        break;
    }
    return rp3d::BodyType::DYNAMIC;
}

/** Shortest rotation taking the unit vector @p from to the unit vector @p to. */
glm::quat rotationBetween(const glm::vec3 &from, const glm::vec3 &to);

/** The node's rotation as one quaternion (Euler angles, axis rotation and quaternion combined). */
glm::quat nodeRotation(const ivf::TransformNode &node);

/** Set the node to @p position and @p rotation (clearing its Euler angles and axis rotation). */
void setNodeTransform(ivf::TransformNode &node, const glm::vec3 &position, const glm::quat &rotation);

/** A shape with an engine collision shape and where it sits in the body. */
struct PlacedShape {
    const Shape *shape{nullptr};
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
};

/** The leaf shapes of @p shape (itself, or the parts of compounds) with their placement in the body. */
std::vector<PlacedShape> placedShapes(const Shape &shape);

} // namespace ivfphysics::detail

struct ivfphysics::Shape::Impl {
    std::shared_ptr<detail::Engine> engine{detail::engine()};
    ShapeType type{ShapeType::Box};
    glm::vec3 halfExtents{0.0f};
    float radius{0.0f};
    float height{0.0f};
    std::vector<glm::vec3> points;
    std::vector<glm::uvec3> triangles;
    std::vector<glm::uvec2> edges; ///< Convex hull edges (debug view).
    std::vector<ShapePtr> parts;   ///< Compound parts.
    glm::vec3 offsetPosition{0.0f};
    glm::quat offsetRotation{1.0f, 0.0f, 0.0f, 0.0f};

    rp3d::CollisionShape *shape{nullptr};
    rp3d::ConvexMesh *convexMesh{nullptr};
    rp3d::TriangleMesh *triangleMesh{nullptr};

    ~Impl();
};

struct ivfphysics::RigidBody::Impl {
    PhysicsWorld::Impl *world{nullptr}; ///< Null once removed.
    std::weak_ptr<RigidBody> self;
    rp3d::RigidBody *body{nullptr};
    std::vector<rp3d::Collider *> colliders; ///< One per (compound) part.
    std::shared_ptr<ivf::TransformNode> node;
    ShapePtr shape;
    BodyDesc desc;
    rp3d::Transform previous;   ///< Transform before the last step (interpolation).
    rp3d::Transform initial;    ///< Transform when added (reset()).
    glm::vec3 previousLinear{0.0f};  ///< Velocities before the last step (impact speeds).
    glm::vec3 previousAngular{0.0f};

    /** Set the density from desc.mass and update mass, centre of mass and inertia. */
    void applyMass();
    /** World inverse inertia tensor applied to @p v. */
    [[nodiscard]] glm::vec3 inverseInertiaWorld(const glm::vec3 &v) const;
};

namespace ivfphysics::detail {

/** Receives the engine's contact and overlap reports during a step and queues them. */
class EventListener : public rp3d::EventListener {
public:
    explicit EventListener(PhysicsWorld::Impl &world) : m_world(world)
    {}
    void onContact(const rp3d::CollisionCallback::CallbackData &data) override;
    void onTrigger(const rp3d::OverlapCallback::CallbackData &data) override;

private:
    PhysicsWorld::Impl &m_world;
};

/** Two bodies touching (or overlapping, for triggers) since an earlier step. */
struct ActivePair {
    RigidBodyPtr body1;
    RigidBodyPtr body2;
    bool trigger{false};
    int colliderPairs{1}; ///< Touching collider pairs (compound bodies touch with several).
};

} // namespace ivfphysics::detail

struct ivfphysics::PhysicsWorld::Impl {
    std::shared_ptr<detail::Engine> engine{detail::engine()};
    detail::EventListener listener{*this};
    rp3d::PhysicsWorld *world{nullptr};
    std::unordered_map<const rp3d::Body *, RigidBodyPtr> lookup; ///< Engine body to body.

    // Events of the current step, delivered after it
    ContactCallback contactCallback;
    TriggerCallback triggerCallback;
    std::vector<Contact> pendingContacts;
    std::vector<TriggerEvent> pendingTriggers;
    std::vector<detail::ActivePair> active; ///< In the order they began (deterministic End order).
    std::vector<ContactPoint> contactPoints;  ///< Of the latest step.
    int contactPairs{0};

    std::vector<JointPtr> joints;
    std::function<void(const JointPtr &)> jointBreakCallback;
    std::vector<JointPtr> pendingBreaks;

    std::vector<std::pair<int, std::function<void(double)>>> stepCallbacks;
    int nextCallbackId{1};

    int stepsLastUpdate{0};
    double updateTimeMs{0.0};
    double averageUpdateTimeMs{0.0};

    [[nodiscard]] RigidBodyPtr find(const rp3d::Body *body) const
    {
        auto it = lookup.find(body);
        return it != lookup.end() ? it->second : nullptr;
    }

    PhysicsSettings settings;
    std::vector<RigidBodyPtr> bodies;
    double accumulator{0.0};
    double time{0.0};
    std::uint64_t steps{0};
    double alpha{1.0};
    double timeScale{1.0};
    bool paused{false};
};

struct ivfphysics::Joint::Impl {
    PhysicsWorld::Impl *world{nullptr}; ///< Null once removed.
    std::weak_ptr<Joint> self;
    JointType type{JointType::Fixed};
    RigidBodyPtr body1;
    RigidBodyPtr body2;                     ///< Null: held to the world.
    rp3d::RigidBody *worldBody{nullptr};    ///< Static engine body at the anchor when body2 is null.
    rp3d::Joint *joint{nullptr};            ///< Null while broken.
    glm::vec3 localAnchor{0.0f};            ///< Anchor in body1 coordinates.
    glm::vec3 localAxis{0.0f, 1.0f, 0.0f};  ///< Axis in body1 coordinates.
    JointDesc desc;
    bool limits{false};
    float lower{0.0f};
    float upper{0.0f};
    float cone{0.0f};
    bool motor{false};
    float motorSpeed{0.0f};
    float maxMotor{0.0f};
    bool broken{false};
    glm::vec3 force{0.0f};
    glm::vec3 torque{0.0f};

    /** Build the engine joint from the settings, at the bodies' current poses. */
    void create();
    /** Destroy the engine joint (not the world body). */
    void destroyJoint();
};
