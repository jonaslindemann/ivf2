#pragma once

/**
 * @file debug_view.h
 * @brief Colliders, contacts and velocities drawn as lines through `ivf::DebugDraw`.
 */

#include <ivfphysics/interaction.h>
#include <ivfphysics/physics_world.h>

#include <glm/glm.hpp>

namespace ivf {
class DebugDraw;
}

namespace ivfphysics {

/** @brief What `drawDebug()` shows. */
struct DebugViewOptions {
    bool colliders{true};      ///< Collision shapes as wireframes, coloured by body type.
    bool staticBodies{true};   ///< Include static bodies (the ground plane is large).
    bool contacts{true};       ///< Contact points with their normals.
    bool velocities{false};    ///< Linear velocity arrows from the centres of mass.
    bool centersOfMass{false}; ///< Small axes at the centres of mass.
    float contactSize{0.05f};  ///< Size of the contact point crosses.
    float normalLength{0.3f};  ///< Length of the contact normals.
    float velocityScale{0.1f}; ///< Arrow length per unit of speed.
    int circleSegments{24};    ///< Segments of sphere and capsule circles.
    int maxTriangles{20000};   ///< Triangle meshes with more triangles are drawn as their bounds.
    glm::vec4 dynamicColor{0.3f, 1.0f, 0.4f, 1.0f};
    glm::vec4 sleepingColor{0.5f, 0.5f, 0.55f, 1.0f};
    glm::vec4 kinematicColor{1.0f, 0.75f, 0.2f, 1.0f};
    glm::vec4 staticColor{0.35f, 0.6f, 1.0f, 1.0f};
    glm::vec4 triggerColor{1.0f, 0.35f, 1.0f, 1.0f};
    glm::vec4 contactColor{1.0f, 0.2f, 0.2f, 1.0f};
    glm::vec4 velocityColor{1.0f, 1.0f, 0.3f, 1.0f};
};

/**
 * @brief Draw the bodies of @p world for one frame.
 *
 * Shapes are drawn where their nodes are shown (interpolated), contacts as of the latest
 * step. In a `GLFWSceneWindow` pass `*renderContext()->debugDraw()` from `onUpdate()`; the
 * window draws the lines after the scene.
 */
void drawDebug(const PhysicsWorld &world, ivf::DebugDraw &draw, const DebugViewOptions &options = {});

/** @brief Draw the spring of @p drag (grabbed point to target) while it holds a body. */
void drawDebug(const DragController &drag, ivf::DebugDraw &draw, const glm::vec4 &color = {1.0f, 1.0f, 1.0f, 1.0f});

} // namespace ivfphysics
