#pragma once

/**
 * @file physics_world.h
 * @brief A simulated world of rigid bodies attached to scene nodes.
 */

#include <ivfphysics/rigid_body.h>
#include <ivfphysics/shape.h>

#include <ivf/result.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace ivf {
class TransformNode;
}

namespace ivfphysics {

/** @brief Settings of a world (`PhysicsWorld::create()`). */
struct PhysicsSettings {
    glm::vec3 gravity{0.0f, -9.81f, 0.0f}; ///< Gravity (units per second squared).
    double fixedTimeStep{1.0 / 240.0};     ///< Length of one simulation step (s).
    int maxSubSteps{16};                   ///< Most steps per `step()` call; the rest of the time is dropped.
    int velocityIterations{10};            ///< Velocity solver iterations per step.
    int positionIterations{5};             ///< Position solver iterations per step.
    bool sleeping{true};                   ///< Let bodies at rest sleep.
    bool interpolate{true};                ///< Show nodes between the last two steps (smooth at any frame rate).
};

/**
 * @class PhysicsWorld
 * @brief Rigid bodies, stepped with a fixed time step, driving scene nodes.
 *
 * `step(frameTime)` advances the shown time by the frame time and runs the fixed steps
 * (`PhysicsSettings::fixedTimeStep`) needed to reach it, so the simulation is at most one
 * step ahead; each dynamic body's node is then set to the state at the shown time,
 * interpolated between the last two steps, so motion is smooth at any frame rate. The same
 * sequence of calls gives the same result, which visual tests rely on (use the window's
 * `frameTime()`, which is fixed with `IVF2_FIXED_TIMESTEP`). No OpenGL is involved: worlds
 * can be stepped in tests or on a thread of their own.
 *
 * @code
 * auto world = ivfphysics::PhysicsWorld::create();
 * world->addBody(ground, ivfphysics::Shape::plane(), {.type = ivfphysics::BodyType::Static});
 * auto cube = ivf::Cube::create();
 * cube->setPos({0.0f, 5.0f, 0.0f});
 * scene->add(cube);
 * world->addBody(cube, ivfphysics::Shape::box(glm::vec3(0.5f)), {.mass = 1.0f});
 * // every frame:
 * world->step(frameTime());
 * @endcode
 */
class PhysicsWorld {
public:
    struct Impl;

    /** @brief A world with @p settings. */
    [[nodiscard]] static std::shared_ptr<PhysicsWorld> create(const PhysicsSettings &settings = {});

    ~PhysicsWorld();
    PhysicsWorld(const PhysicsWorld &) = delete;
    PhysicsWorld &operator=(const PhysicsWorld &) = delete;

    /** @brief The settings the world was created with (gravity follows `setGravity()`). */
    [[nodiscard]] const PhysicsSettings &settings() const;

    /**
     * @brief Add a body for @p node with @p shape.
     *
     * The body starts at the node's position and rotation. Adding the same node twice
     * replaces its body. A triangle mesh shape makes a dynamic body static (with a warning).
     */
    RigidBodyPtr addBody(const std::shared_ptr<ivf::TransformNode> &node, const ShapePtr &shape,
                         const BodyDesc &desc = {});

    /** @brief Add a body with the shape of @p node (`Shape::fromNode()`). */
    ivf::Result<RigidBodyPtr> addBody(const std::shared_ptr<ivf::TransformNode> &node, const BodyDesc &desc = {});

    /** @brief Remove @p body; it becomes invalid. The node stays where it is. */
    void removeBody(const RigidBodyPtr &body);

    /** @brief Remove all bodies. */
    void clear();

    /** @brief All bodies, in the order they were added. */
    [[nodiscard]] const std::vector<RigidBodyPtr> &bodies() const;

    /** @brief The body of @p node, or nullptr. */
    [[nodiscard]] RigidBodyPtr bodyOf(const std::shared_ptr<ivf::TransformNode> &node) const;

    /**
     * @brief Advance the simulation by @p dt seconds (times `timeScale()`).
     * @return The number of fixed steps run (0 when paused, or when the last step already
     *         reaches the new time).
     */
    int step(double dt);

    /** @brief Run exactly one fixed step, also when paused, and update the nodes. */
    void stepOnce();

    /** @brief Pause or resume; while paused `step()` does nothing. */
    void setPaused(bool paused);
    [[nodiscard]] bool isPaused() const;

    /** @brief Factor on the time passed to `step()` (slow motion below 1). */
    void setTimeScale(double scale);
    [[nodiscard]] double timeScale() const;

    /**
     * @brief Put every body back where it was added, at rest, and set the time to zero.
     *
     * Body properties changed since (mass, friction, ...) are kept.
     */
    void reset();

    /** @brief Gravity. */
    void setGravity(const glm::vec3 &gravity);
    [[nodiscard]] glm::vec3 gravity() const;

    /** @brief Simulated time (s): number of steps times the fixed step. */
    [[nodiscard]] double time() const;

    /** @brief Number of fixed steps run since creation or `reset()`. */
    [[nodiscard]] std::uint64_t stepCount() const;

    /** @brief Where the shown state lies between the last two steps (0 to 1; 1: the last step). */
    [[nodiscard]] double interpolationFactor() const;

    /** @brief Number of awake dynamic bodies. */
    [[nodiscard]] int awakeBodyCount() const;

    /** @brief Engine data (implementation detail of ivfphysics). */
    [[nodiscard]] Impl &impl() const
    {
        return *m_impl;
    }

private:
    explicit PhysicsWorld(const PhysicsSettings &settings);
    void runStep();
    void syncNodes(double alpha);
    std::unique_ptr<Impl> m_impl;
};

using PhysicsWorldPtr = std::shared_ptr<PhysicsWorld>;

} // namespace ivfphysics
