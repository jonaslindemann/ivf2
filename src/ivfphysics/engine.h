#pragma once

// Private to ivfphysics: the ReactPhysics3D side of shapes, bodies and worlds.

#include <ivfphysics/physics_world.h>
#include <ivfphysics/rigid_body.h>
#include <ivfphysics/shape.h>

#include <reactphysics3d/reactphysics3d.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
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

} // namespace ivfphysics::detail

struct ivfphysics::Shape::Impl {
    std::shared_ptr<detail::Engine> engine{detail::engine()};
    ShapeType type{ShapeType::Box};
    glm::vec3 halfExtents{0.0f};
    float radius{0.0f};
    float height{0.0f};
    std::vector<glm::vec3> points;
    std::vector<glm::uvec3> triangles;
    glm::vec3 offsetPosition{0.0f};
    glm::quat offsetRotation{1.0f, 0.0f, 0.0f, 0.0f};

    rp3d::CollisionShape *shape{nullptr};
    rp3d::ConvexMesh *convexMesh{nullptr};
    rp3d::TriangleMesh *triangleMesh{nullptr};

    ~Impl();
};

struct ivfphysics::RigidBody::Impl {
    PhysicsWorld::Impl *world{nullptr}; ///< Null once removed.
    rp3d::RigidBody *body{nullptr};
    rp3d::Collider *collider{nullptr};
    std::shared_ptr<ivf::TransformNode> node;
    ShapePtr shape;
    BodyDesc desc;
    rp3d::Transform previous;   ///< Transform before the last step (interpolation).
    rp3d::Transform initial;    ///< Transform when added (reset()).

    /** Set the density from desc.mass and update mass, centre of mass and inertia. */
    void applyMass();
    /** World inverse inertia tensor applied to @p v. */
    [[nodiscard]] glm::vec3 inverseInertiaWorld(const glm::vec3 &v) const;
};

struct ivfphysics::PhysicsWorld::Impl {
    std::shared_ptr<detail::Engine> engine{detail::engine()};
    rp3d::PhysicsWorld *world{nullptr};
    PhysicsSettings settings;
    std::vector<RigidBodyPtr> bodies;
    double accumulator{0.0};
    double time{0.0};
    std::uint64_t steps{0};
    double alpha{1.0};
    double timeScale{1.0};
    bool paused{false};
};
