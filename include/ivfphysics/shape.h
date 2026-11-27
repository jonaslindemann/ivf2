#pragma once

/**
 * @file shape.h
 * @brief Collision shapes for ivfphysics rigid bodies.
 */

#include <ivf/result.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace ivf {
class Mesh;
class TransformNode;
} // namespace ivf

namespace ivfphysics {

/** @brief Kind of a collision shape. */
enum class ShapeType {
    Box,          ///< Box given by half extents.
    Sphere,       ///< Sphere given by its radius.
    Capsule,      ///< Capsule along the local Y axis.
    ConvexHull,   ///< Convex hull of a point set.
    TriangleMesh, ///< Arbitrary triangles; static and kinematic bodies only.
    Compound,     ///< Several shapes, each placed by its offset (`Shape::compound()`).
};

/** @brief Readable name of a shape type, e.g. "convex hull". */
const char *toString(ShapeType type);

/**
 * @class Shape
 * @brief The geometry a rigid body collides with, in the body's local coordinates.
 *
 * A shape can be shared by any number of bodies, also in different worlds. It may sit at
 * an offset from the body origin (`setOffset()`, read when the shape is added to a body). Sizes are in world units: unlike nodes, shapes are not
 * scaled with the node, so `fromNode()` bakes the node's scale into the shape.
 *
 * @code
 * auto ground = world->addBody(floor, ivfphysics::Shape::plane(), {.type = BodyType::Static});
 * auto crate  = world->addBody(cube, ivfphysics::Shape::box({0.5f, 0.5f, 0.5f}), {.mass = 2.0f});
 * auto rock   = world->addBody(model, *ivfphysics::Shape::fromNode(model));
 * @endcode
 */
class Shape {
public:
    struct Impl;

    /** @brief Box with half extents @p halfExtents (all > 0). */
    [[nodiscard]] static std::shared_ptr<Shape> box(const glm::vec3 &halfExtents);

    /** @brief Sphere of radius @p radius. */
    [[nodiscard]] static std::shared_ptr<Shape> sphere(float radius);

    /**
     * @brief Capsule along the local Y axis.
     * @param radius Radius of the cylinder and the caps.
     * @param height Distance between the centres of the two caps (the total height is
     *               `height + 2 * radius`).
     */
    [[nodiscard]] static std::shared_ptr<Shape> capsule(float radius, float height);

    /**
     * @brief Ground plane through `normal * offset`, solid below.
     *
     * The engine has no infinite planes: this is a box @p size wide and 1 unit thick whose
     * top face is the plane, so bodies are stopped even when they hit it fast.
     */
    [[nodiscard]] static std::shared_ptr<Shape> plane(const glm::vec3 &normal = {0.0f, 1.0f, 0.0f}, float offset = 0.0f,
                                                      float size = 1000.0f);

    /** @brief Convex hull of @p points (at least four, not all in one plane). */
    [[nodiscard]] static ivf::Result<std::shared_ptr<Shape>> convexHull(const std::vector<glm::vec3> &points);

    /** @brief Convex hull of the vertices of @p mesh, scaled by @p scale. */
    [[nodiscard]] static ivf::Result<std::shared_ptr<Shape>> convexHull(const std::shared_ptr<ivf::Mesh> &mesh,
                                                                        const glm::vec3 &scale = glm::vec3(1.0f));

    /**
     * @brief Triangle mesh (concave). Only for static and kinematic bodies.
     * @param positions Vertex positions.
     * @param triangles Vertex indices of each triangle (counter-clockwise seen from outside).
     */
    [[nodiscard]] static ivf::Result<std::shared_ptr<Shape>> triangleMesh(const std::vector<glm::vec3> &positions,
                                                                          const std::vector<glm::uvec3> &triangles);

    /** @brief Triangle mesh from the triangles @p mesh draws, scaled by @p scale. */
    [[nodiscard]] static ivf::Result<std::shared_ptr<Shape>> triangleMesh(const std::shared_ptr<ivf::Mesh> &mesh,
                                                                          const glm::vec3 &scale = glm::vec3(1.0f));

    /**
     * @brief The shape of a node, from its type and local bounding box.
     *
     * `Cube` and `Box` give boxes, `Sphere` a sphere, `Capsule` a capsule, `Plane` a thin
     * box under its surface; any other node with meshes (`Cylinder`, `Cone`, `MeshNode`,
     * ...) gives the convex hull of its vertices, or a triangle mesh with @p concave
     * (static bodies). The node's scale is baked in.
     */
    /**
     * @brief A shape made of @p parts, each placed in the body by its offset (`setOffset()`).
     *
     * For bodies that no single convex shape fits: a table (top and legs), a hammer, an
     * L-shaped bracket. The mass is spread over the parts by volume, so the centre of mass
     * and inertia follow from the parts. The compound's own offset moves all parts; parts may
     * be compounds themselves. A part that is a triangle mesh makes the body static.
     *
     * @code
     * auto top = ivfphysics::Shape::box({1.0f, 0.05f, 0.6f});
     * top->setOffset({0.0f, 0.75f, 0.0f});
     * std::vector<ivfphysics::ShapePtr> parts{top};
     * for (float x : {-0.9f, 0.9f})
     *     for (float z : {-0.5f, 0.5f})
     *     {
     *         auto leg = ivfphysics::Shape::box({0.05f, 0.35f, 0.05f});
     *         leg->setOffset({x, 0.35f, z});
     *         parts.push_back(leg);
     *     }
     * auto table = ivfphysics::Shape::compound(parts);
     * @endcode
     * @throws std::invalid_argument if @p parts is empty or contains nullptr.
     */
    [[nodiscard]] static std::shared_ptr<Shape> compound(const std::vector<std::shared_ptr<Shape>> &parts);

    [[nodiscard]] static ivf::Result<std::shared_ptr<Shape>> fromNode(const std::shared_ptr<ivf::TransformNode> &node,
                                                                      bool concave = false);

    ~Shape();
    Shape(const Shape &) = delete;
    Shape &operator=(const Shape &) = delete;

    /** @brief Kind of shape. */
    [[nodiscard]] ShapeType type() const;

    /** @brief Box half extents (Box), or zero. */
    [[nodiscard]] glm::vec3 halfExtents() const;

    /** @brief Radius (Sphere, Capsule), or zero. */
    [[nodiscard]] float radius() const;

    /** @brief Distance between the cap centres (Capsule), or zero. */
    [[nodiscard]] float height() const;

    /** @brief Hull points (ConvexHull) or vertex positions (TriangleMesh). */
    [[nodiscard]] const std::vector<glm::vec3> &points() const;

    /** @brief Triangles (TriangleMesh), empty otherwise. */
    [[nodiscard]] const std::vector<glm::uvec3> &triangles() const;

    /** @brief The parts of a compound shape, empty otherwise. */
    [[nodiscard]] const std::vector<std::shared_ptr<Shape>> &parts() const;

    /** @brief True if the shape is a triangle mesh or a compound with one among its parts. */
    [[nodiscard]] bool isConcave() const;

    /** @brief Volume (used to turn a body's mass into a density). */
    [[nodiscard]] float volume() const;

    /** @brief Axis-aligned bounds in body coordinates, including the offset. */
    [[nodiscard]] std::pair<glm::vec3, glm::vec3> bounds() const;

    /** @brief Place the shape at @p position and @p rotation relative to the body origin. */
    void setOffset(const glm::vec3 &position, const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

    /** @brief Offset of the shape from the body origin. */
    [[nodiscard]] glm::vec3 offsetPosition() const;

    /** @brief Rotation of the shape relative to the body. */
    [[nodiscard]] glm::quat offsetRotation() const;

    /** @brief Engine data (implementation detail of ivfphysics). */
    [[nodiscard]] Impl &impl() const
    {
        return *m_impl;
    }

private:
    explicit Shape(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> m_impl;
};

using ShapePtr = std::shared_ptr<Shape>;

} // namespace ivfphysics
