#pragma once

/**
 * @file picking.h
 * @brief Ray casting against the scene: which node is under the mouse, and where exactly.
 *
 * @code
 * // In a GLFWSceneWindow: the nearest node under the mouse
 * if (auto hit = pick(mouseX(), mouseY()))
 *     logInfo(logStr(hit.node->name(), " at ", hit.position.x, ", ", hit.position.y, ", ", hit.position.z));
 *
 * // Any queue and ray
 * ivf::Ray ray{{0.0f, 10.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};
 * ivf::RayHit ground = ivf::pick(queue, ray);
 * @endcode
 *
 * Meshes are tested triangle by triangle through their bounding volume hierarchy
 * (Mesh::bvh()), in the world transform they are drawn with, so instanced copies, deformed
 * and skinned meshes are hit where they appear. The picking happens on the CPU and needs no
 * rendering, unlike the selection buffer.
 */

#include <ivf/ray.h>
#include <ivf/render_queue.h>

#include <functional>
#include <limits>
#include <vector>

namespace ivf {

class Mesh;
class Node;

/** @brief Result of a ray cast. Converts to false if nothing was hit. */
struct RayHit {
    Node *node{nullptr};        ///< The node that was hit (the drawn node, as in the selection buffer).
    Mesh *mesh{nullptr};        ///< Its mesh that was hit (nullptr for a bounding-box hit).
    float distance{0.0f};       ///< Distance from the ray origin (t of a unit-direction ray).
    glm::vec3 position{0.0f};   ///< Hit point in world space.
    glm::vec3 normal{0.0f};     ///< Surface normal in world space, facing the ray's origin.
    int triangle{-1};           ///< Triangle of the mesh (-1 for a bounding-box hit).
    glm::vec2 barycentric{0.0f}; ///< Weights of the triangle's second and third vertex.
    bool exact{true};           ///< False if only the node's bounding box was hit (no mesh).

    explicit operator bool() const
    {
        return node != nullptr;
    }
};

/** @brief What pick() tests. */
struct PickOptions {
    /// Nodes without a mesh (or with lines only) are hit at their bounding box.
    bool boundsFallback{true};
    /// Include transparent items (default true; background and overlay items are never picked).
    bool includeTransparent{true};
    /// Ignore hits farther than this.
    float maxDistance{std::numeric_limits<float>::infinity()};
    /// Leave items out (return true to skip), e.g. the object being dragged.
    std::function<bool(const DrawItem &)> skip;
};

/** @brief The nearest hit of @p ray among the items of @p queue (nothing: an empty RayHit). */
[[nodiscard]] RayHit pick(const RenderQueue &queue, const Ray &ray, const PickOptions &options = {});

/** @brief All hits of @p ray, nearest first (at most one per item). */
[[nodiscard]] std::vector<RayHit> pickAll(const RenderQueue &queue, const Ray &ray, const PickOptions &options = {});

} // namespace ivf
