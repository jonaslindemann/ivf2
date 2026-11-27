#include "convex_hull.h"
#include "engine.h"

#include <ivf/box.h>
#include <ivf/capsule.h>
#include <ivf/cube.h>
#include <ivf/logger.h>
#include <ivf/mesh.h>
#include <ivf/mesh_bvh.h>
#include <ivf/mesh_node.h>
#include <ivf/plane.h>
#include <ivf/sphere.h>
#include <ivf/transform_node.h>
#include <ivf/vertices.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <string>

using namespace ivfphysics;

namespace {

void requirePositive(float value, const char *what)
{
    if (!(value > 0.0f) || !std::isfinite(value))
        throw std::invalid_argument(std::string("ivfphysics::Shape: ") + what + " must be positive");
}

std::string messagesText(const std::vector<rp3d::Message> &messages)
{
    std::string text;
    for (const auto &m : messages)
    {
        if (m.type == rp3d::Message::Type::Information)
            continue;
        if (!text.empty())
            text += "; ";
        text += m.text;
    }
    return text.empty() ? std::string("unknown error") : text;
}

/** Triangles of a mesh as drawn (through its BVH, which resolves indexing and strips). */
bool meshTriangles(ivf::Mesh &mesh, const glm::vec3 &scale, std::vector<glm::vec3> &positions,
                   std::vector<glm::uvec3> &triangles)
{
    auto bvh = mesh.bvh();
    if (!bvh || bvh->triangleCount() == 0)
        return false;
    std::uint32_t maxIndex = 0;
    for (std::uint32_t t = 0; t < bvh->triangleCount(); ++t)
    {
        const auto tri = bvh->triangle(t);
        maxIndex = std::max({maxIndex, tri.x, tri.y, tri.z});
    }
    const auto base = static_cast<std::uint32_t>(positions.size());
    for (std::uint32_t i = 0; i <= maxIndex; ++i)
        positions.push_back(bvh->position(i) * scale);
    for (std::uint32_t t = 0; t < bvh->triangleCount(); ++t)
        triangles.push_back(bvh->triangle(t) + glm::uvec3(base));
    return true;
}

} // namespace

const char *ivfphysics::toString(ShapeType type)
{
    switch (type)
    {
    case ShapeType::Box:
        return "box";
    case ShapeType::Sphere:
        return "sphere";
    case ShapeType::Capsule:
        return "capsule";
    case ShapeType::ConvexHull:
        return "convex hull";
    case ShapeType::TriangleMesh:
        return "triangle mesh";
    }
    return "unknown";
}

Shape::Impl::~Impl()
{
    auto &common = engine->common;
    if (shape)
    {
        switch (type)
        {
        case ShapeType::Box:
            common.destroyBoxShape(static_cast<rp3d::BoxShape *>(shape));
            break;
        case ShapeType::Sphere:
            common.destroySphereShape(static_cast<rp3d::SphereShape *>(shape));
            break;
        case ShapeType::Capsule:
            common.destroyCapsuleShape(static_cast<rp3d::CapsuleShape *>(shape));
            break;
        case ShapeType::ConvexHull:
            common.destroyConvexMeshShape(static_cast<rp3d::ConvexMeshShape *>(shape));
            break;
        case ShapeType::TriangleMesh:
            common.destroyConcaveMeshShape(static_cast<rp3d::ConcaveMeshShape *>(shape));
            break;
        }
    }
    if (convexMesh)
        common.destroyConvexMesh(convexMesh);
    if (triangleMesh)
        common.destroyTriangleMesh(triangleMesh);
}

Shape::Shape(std::unique_ptr<Impl> impl) : m_impl(std::move(impl))
{}

Shape::~Shape() = default;

ShapePtr Shape::box(const glm::vec3 &halfExtents)
{
    requirePositive(halfExtents.x, "box half extent x");
    requirePositive(halfExtents.y, "box half extent y");
    requirePositive(halfExtents.z, "box half extent z");
    auto impl = std::make_unique<Impl>();
    impl->type = ShapeType::Box;
    impl->halfExtents = halfExtents;
    impl->shape = impl->engine->common.createBoxShape(detail::toRp(halfExtents));
    return ShapePtr(new Shape(std::move(impl)));
}

ShapePtr Shape::sphere(float radius)
{
    requirePositive(radius, "sphere radius");
    auto impl = std::make_unique<Impl>();
    impl->type = ShapeType::Sphere;
    impl->radius = radius;
    impl->shape = impl->engine->common.createSphereShape(radius);
    return ShapePtr(new Shape(std::move(impl)));
}

ShapePtr Shape::capsule(float radius, float height)
{
    requirePositive(radius, "capsule radius");
    requirePositive(height, "capsule height");
    auto impl = std::make_unique<Impl>();
    impl->type = ShapeType::Capsule;
    impl->radius = radius;
    impl->height = height;
    impl->shape = impl->engine->common.createCapsuleShape(radius, height);
    return ShapePtr(new Shape(std::move(impl)));
}

ShapePtr Shape::plane(const glm::vec3 &normal, float offset, float size)
{
    requirePositive(size, "plane size");
    if (glm::dot(normal, normal) < 1e-12f)
        throw std::invalid_argument("ivfphysics::Shape::plane: the normal must not be zero");
    const glm::vec3 n = glm::normalize(normal);
    constexpr float halfThickness = 0.5f;
    auto shape = box({0.5f * size, halfThickness, 0.5f * size});
    // The box's +Y face is the plane: rotate +Y onto the normal, move the centre below it
    shape->setOffset(n * (offset - halfThickness), detail::rotationBetween(glm::vec3(0.0f, 1.0f, 0.0f), n));
    return shape;
}

ivf::Result<ShapePtr> Shape::convexHull(const std::vector<glm::vec3> &points)
{
    if (points.size() < 4)
        return ivf::makeError(ivf::ErrorCode::InvalidArgument,
                              "A convex hull needs at least 4 points, got " + std::to_string(points.size()),
                              "Physics");
    detail::Polyhedron hull;
    std::string error;
    if (!detail::convexHull(points, hull, error))
        return ivf::makeError(ivf::ErrorCode::InvalidArgument,
                              "No convex hull of " + std::to_string(points.size()) + " points: " + error, "Physics");

    // The engine gets the finished polyhedron: its own hull builder fails on coplanar points
    auto impl = std::make_unique<Impl>();
    impl->type = ShapeType::ConvexHull;
    impl->points = hull.vertices;
    std::vector<std::uint32_t> indices;
    std::vector<rp3d::PolygonVertexArray::PolygonFace> faces;
    for (const auto &face : hull.faces)
    {
        faces.push_back({static_cast<rp3d::uint32>(face.size()), static_cast<rp3d::uint32>(indices.size())});
        indices.insert(indices.end(), face.begin(), face.end());
    }
    rp3d::PolygonVertexArray array(static_cast<rp3d::uint32>(impl->points.size()), impl->points.data(),
                                   sizeof(glm::vec3), indices.data(), sizeof(std::uint32_t),
                                   static_cast<rp3d::uint32>(faces.size()), faces.data(),
                                   rp3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
                                   rp3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    std::vector<rp3d::Message> messages;
    impl->convexMesh = impl->engine->common.createConvexMesh(array, messages);
    if (!impl->convexMesh)
        return ivf::makeError(ivf::ErrorCode::InvalidData,
                              "Could not build a convex hull of " + std::to_string(points.size()) +
                                  " points: " + messagesText(messages),
                              "Physics");
    impl->shape = impl->engine->common.createConvexMeshShape(impl->convexMesh);
    return ShapePtr(new Shape(std::move(impl)));
}

ivf::Result<ShapePtr> Shape::convexHull(const std::shared_ptr<ivf::Mesh> &mesh, const glm::vec3 &scale)
{
    if (!mesh || !mesh->vertices())
        return ivf::makeError(ivf::ErrorCode::InvalidArgument, "Convex hull of a mesh without vertices", "Physics");
    auto vertices = mesh->vertices();
    std::vector<glm::vec3> points;
    points.reserve(vertices->rows());
    for (GLuint i = 0; i < vertices->rows(); ++i)
        points.push_back(vertices->vertex(i) * scale);
    return convexHull(points);
}

ivf::Result<ShapePtr> Shape::triangleMesh(const std::vector<glm::vec3> &positions,
                                          const std::vector<glm::uvec3> &triangles)
{
    if (positions.size() < 3 || triangles.empty())
        return ivf::makeError(ivf::ErrorCode::InvalidArgument, "A triangle mesh needs vertices and triangles",
                              "Physics");
    for (const auto &t : triangles)
        if (t.x >= positions.size() || t.y >= positions.size() || t.z >= positions.size())
            return ivf::makeError(ivf::ErrorCode::InvalidArgument, "Triangle mesh index out of range", "Physics");

    auto impl = std::make_unique<Impl>();
    impl->type = ShapeType::TriangleMesh;
    impl->points = positions;
    impl->triangles = triangles;
    rp3d::TriangleVertexArray array(
        static_cast<rp3d::uint32>(impl->points.size()), impl->points.data(), sizeof(glm::vec3),
        static_cast<rp3d::uint32>(impl->triangles.size()), impl->triangles.data(), sizeof(glm::uvec3),
        rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
        rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    std::vector<rp3d::Message> messages;
    impl->triangleMesh = impl->engine->common.createTriangleMesh(array, messages);
    if (!impl->triangleMesh)
        return ivf::makeError(ivf::ErrorCode::InvalidData,
                              "Could not build a triangle mesh of " + std::to_string(triangles.size()) +
                                  " triangles: " + messagesText(messages),
                              "Physics");
    impl->shape = impl->engine->common.createConcaveMeshShape(impl->triangleMesh);
    return ShapePtr(new Shape(std::move(impl)));
}

ivf::Result<ShapePtr> Shape::triangleMesh(const std::shared_ptr<ivf::Mesh> &mesh, const glm::vec3 &scale)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::uvec3> triangles;
    if (!mesh || !meshTriangles(*mesh, scale, positions, triangles))
        return ivf::makeError(ivf::ErrorCode::InvalidArgument, "Triangle mesh of a mesh without triangles",
                              "Physics");
    return triangleMesh(positions, triangles);
}

ivf::Result<ShapePtr> Shape::fromNode(const std::shared_ptr<ivf::TransformNode> &node, bool concave)
{
    if (!node)
        return ivf::makeError(ivf::ErrorCode::InvalidArgument, "Shape::fromNode() without a node", "Physics");

    const glm::vec3 scale = node->scale();
    const ivf::BoundingBox bbox = node->localBoundingBox();
    const bool primitive = std::dynamic_pointer_cast<ivf::Sphere>(node) || std::dynamic_pointer_cast<ivf::Cube>(node) ||
                           std::dynamic_pointer_cast<ivf::Box>(node) || std::dynamic_pointer_cast<ivf::Capsule>(node) ||
                           std::dynamic_pointer_cast<ivf::Plane>(node);
    if (primitive && !concave && bbox.isValid())
    {
        const glm::vec3 half = 0.5f * bbox.size() * scale;
        const glm::vec3 center = bbox.center() * scale;
        ShapePtr shape;
        if (std::dynamic_pointer_cast<ivf::Sphere>(node))
            shape = sphere(std::max({half.x, half.y, half.z}));
        else if (std::dynamic_pointer_cast<ivf::Capsule>(node))
        {
            const float r = std::max(half.x, half.z);
            shape = capsule(r, std::max(2.0f * (half.y - r), 1e-4f));
        }
        else if (std::dynamic_pointer_cast<ivf::Plane>(node))
        {
            // A thin box under the plane's surface
            constexpr float halfThickness = 0.05f;
            shape = box({std::max(half.x, 1e-3f), halfThickness, std::max(half.z, 1e-3f)});
            shape->setOffset(center - glm::vec3(0.0f, halfThickness, 0.0f));
            return shape;
        }
        else
            shape = box(glm::max(half, glm::vec3(1e-3f)));
        shape->setOffset(center);
        return shape;
    }

    auto meshNode = std::dynamic_pointer_cast<ivf::MeshNode>(node);
    if (!meshNode || meshNode->meshes().empty())
        return ivf::makeError(ivf::ErrorCode::InvalidArgument,
                              "Shape::fromNode(): the node has no geometry (" + node->name() + ")", "Physics");

    std::vector<glm::vec3> positions;
    std::vector<glm::uvec3> triangles;
    for (const auto &mesh : meshNode->meshes())
    {
        if (!mesh)
            continue;
        if (concave)
            meshTriangles(*mesh, scale, positions, triangles);
        else if (auto vertices = mesh->vertices())
            for (GLuint i = 0; i < vertices->rows(); ++i)
                positions.push_back(vertices->vertex(i) * scale);
    }
    return concave ? triangleMesh(positions, triangles) : convexHull(positions);
}

ShapeType Shape::type() const
{
    return m_impl->type;
}

glm::vec3 Shape::halfExtents() const
{
    return m_impl->halfExtents;
}

float Shape::radius() const
{
    return m_impl->radius;
}

float Shape::height() const
{
    return m_impl->height;
}

const std::vector<glm::vec3> &Shape::points() const
{
    return m_impl->points;
}

const std::vector<glm::uvec3> &Shape::triangles() const
{
    return m_impl->triangles;
}

float Shape::volume() const
{
    const auto &s = *m_impl;
    switch (s.type)
    {
    case ShapeType::Box:
        return 8.0f * s.halfExtents.x * s.halfExtents.y * s.halfExtents.z;
    case ShapeType::Sphere:
        return 4.0f / 3.0f * std::numbers::pi_v<float> * s.radius * s.radius * s.radius;
    case ShapeType::Capsule:
        return std::numbers::pi_v<float> * s.radius * s.radius * (s.height + 4.0f / 3.0f * s.radius);
    case ShapeType::ConvexHull:
    case ShapeType::TriangleMesh:
        return static_cast<float>(s.shape->getVolume());
    }
    return 0.0f;
}

std::pair<glm::vec3, glm::vec3> Shape::bounds() const
{
    const rp3d::AABB local = m_impl->shape->getLocalBounds();
    const glm::vec3 lo = detail::toGlm(local.getMin());
    const glm::vec3 hi = detail::toGlm(local.getMax());
    glm::vec3 bmin(std::numeric_limits<float>::max());
    glm::vec3 bmax(std::numeric_limits<float>::lowest());
    for (int i = 0; i < 8; ++i)
    {
        const glm::vec3 corner((i & 1) ? hi.x : lo.x, (i & 2) ? hi.y : lo.y, (i & 4) ? hi.z : lo.z);
        const glm::vec3 p = m_impl->offsetPosition + m_impl->offsetRotation * corner;
        bmin = glm::min(bmin, p);
        bmax = glm::max(bmax, p);
    }
    return {bmin, bmax};
}

void Shape::setOffset(const glm::vec3 &position, const glm::quat &rotation)
{
    m_impl->offsetPosition = position;
    m_impl->offsetRotation = glm::normalize(rotation);
}

glm::vec3 Shape::offsetPosition() const
{
    return m_impl->offsetPosition;
}

glm::quat Shape::offsetRotation() const
{
    return m_impl->offsetRotation;
}
