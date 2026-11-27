#pragma once

/**
 * @file interaction.h
 * @brief Grabbing, dragging and throwing bodies with camera rays.
 */

#include <ivfphysics/physics_world.h>

#include <ivf/ray.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivfphysics {

/** @brief How a `DragController` pulls (a damped spring, independent of the body's mass). */
struct DragSettings {
    float frequency{5.0f};         ///< Spring frequency (Hz): how fast the body follows.
    float dampingRatio{0.9f};      ///< 1: no overshoot; lower values swing.
    float maxAcceleration{200.0f}; ///< Limit of the spring's pull (units/s^2), so heavy stacks are not flung.
    float angularDamping{3.0f};    ///< Angular damping while dragged (calms spinning; restored on release).
};

/**
 * @class DragController
 * @brief Drags a body with the mouse: a damped spring pulls the grabbed point towards a target.
 *
 * GL-free: give it camera rays (`GLFWSceneWindow::cameraRay(x, y)`). `grab()` finds the
 * dynamic body under the ray and remembers the point that was hit; `drag()` moves the
 * target along later rays at the same distance from the camera. The spring acts before
 * every fixed step (`PhysicsWorld::addStepCallback()`), so dragging is smooth and the same at
 * any frame rate; the body still collides and pushes others.
 *
 * @code
 * m_drag = ivfphysics::DragController::create(m_world);
 * // mouse down:
 * m_drag->grab(cameraRay(x, y));
 * // mouse move:
 * if (m_drag->isDragging())
 *     m_drag->drag(cameraRay(x, y));
 * // mouse up:
 * m_drag->release();
 * @endcode
 */
class DragController {
public:
    /** @brief A controller for bodies of @p world. */
    [[nodiscard]] static std::shared_ptr<DragController> create(const PhysicsWorldPtr &world,
                                                                const DragSettings &settings = {});

    ~DragController();
    DragController(const DragController &) = delete;
    DragController &operator=(const DragController &) = delete;

    /**
     * @brief Grab the nearest body hit by @p ray if it is dynamic.
     * @return False (and nothing grabbed) if the nearest hit is not a dynamic body or there is none.
     */
    bool grab(const ivf::Ray &ray, const RaycastOptions &options = {});

    /**
     * @brief Grab @p body at the world point @p point; later rays keep the distance
     *        between @p rayOrigin and the point.
     */
    bool grab(const RigidBodyPtr &body, const glm::vec3 &point, const glm::vec3 &rayOrigin);

    /** @brief Move the target to the point of @p ray at the grab distance. */
    void drag(const ivf::Ray &ray);

    /** @brief Move the target to @p target (world coordinates). */
    void setTarget(const glm::vec3 &target);

    /** @brief Change the distance from the ray origin to the target (e.g. with the scroll wheel). */
    void setDistance(float distance);

    /** @brief Let go. */
    void release();

    /** @brief True while a body is held. */
    [[nodiscard]] bool isDragging() const;

    /** @brief The body held, or nullptr. */
    [[nodiscard]] RigidBodyPtr body() const;

    /** @brief Where the grabbed point is pulled to. */
    [[nodiscard]] glm::vec3 target() const;

    /** @brief The grabbed point on the body (world coordinates, latest step). */
    [[nodiscard]] glm::vec3 anchor() const;

    /** @brief Distance from the ray origin to the target. */
    [[nodiscard]] float distance() const;

    /** @brief Spring settings. */
    [[nodiscard]] const DragSettings &settings() const;
    void setSettings(const DragSettings &settings);

private:
    DragController(const PhysicsWorldPtr &world, const DragSettings &settings);
    void applySpring(double h);

    std::weak_ptr<PhysicsWorld> m_world;
    DragSettings m_settings;
    int m_callback{0};
    RigidBodyPtr m_body;
    glm::vec3 m_localAnchor{0.0f}; ///< Grabbed point relative to the body (body coordinates).
    glm::vec3 m_target{0.0f};
    glm::vec3 m_rayOrigin{0.0f};
    glm::vec3 m_rayDirection{0.0f, 0.0f, -1.0f};
    float m_distance{0.0f};
    float m_savedAngularDamping{0.0f};
};

using DragControllerPtr = std::shared_ptr<DragController>;

/**
 * @brief Launch @p body from the origin of @p ray along its direction.
 *
 * Moves the body to the ray origin, sets its velocity to @p speed along the ray and its
 * angular velocity to @p spin, and wakes it. Typically a sphere created for the throw:
 *
 * @code
 * auto ball = ivf::Sphere::create(0.25);
 * scene->add(ball);
 * auto body = world->addBody(ball, ivfphysics::Shape::sphere(0.25f), {.mass = 2.0f});
 * ivfphysics::throwBody(body, cameraRay(mouseX(), mouseY()), 20.0f);
 * @endcode
 */
void throwBody(const RigidBodyPtr &body, const ivf::Ray &ray, float speed, const glm::vec3 &spin = glm::vec3(0.0f));

} // namespace ivfphysics
