#include <ivf/picking.h>

#include <ivf/mesh.h>
#include <ivf/mesh_node.h>
#include <ivf/node.h>

#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>
#include <cmath>

using namespace ivf;

namespace {

/// Hit of @p ray (world space) with @p mesh drawn with @p world, nearer than @p maxT
bool hitMesh(Mesh &mesh, const glm::mat4 &world, const Ray &ray, float maxT, RayHit &hit)
{
    auto bvh = mesh.bvh();
    if (!bvh)
        return false;
    const Ray local = ray.transformed(glm::inverse(world)); // same t as in world space
    const auto result = bvh->intersect(local, maxT);
    if (!result)
        return false;

    hit.mesh = &mesh;
    hit.distance = result->t;
    hit.position = ray.at(result->t);
    hit.triangle = static_cast<int>(result->triangle);
    hit.barycentric = {result->u, result->v};
    glm::vec3 n = glm::inverseTranspose(glm::mat3(world)) * bvh->faceNormal(result->triangle);
    n = glm::length(n) > 0.0f ? glm::normalize(n) : -ray.direction;
    hit.normal = glm::dot(n, ray.direction) > 0.0f ? -n : n;
    hit.exact = true;
    return true;
}

/// Normal of the box face a ray enters at @p point
glm::vec3 boxNormal(const BoundingBox &box, const glm::vec3 &point)
{
    const glm::vec3 c = box.center();
    const glm::vec3 half = glm::max(box.size() * 0.5f, glm::vec3(1e-6f));
    const glm::vec3 d = (point - c) / half;
    const glm::vec3 a = glm::abs(d);
    if (a.x >= a.y && a.x >= a.z)
        return {d.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f};
    if (a.y >= a.z)
        return {0.0f, d.y > 0.0f ? 1.0f : -1.0f, 0.0f};
    return {0.0f, 0.0f, d.z > 0.0f ? 1.0f : -1.0f};
}

/// Hit of one item, nearer than @p maxT
bool hitItem(const DrawItem &item, const Ray &ray, float maxT, const PickOptions &options, RayHit &hit)
{
    if (item.node == nullptr)
        return false;
    if (item.layer == RenderLayer::Background || item.layer == RenderLayer::Overlay)
        return false;
    if (item.layer == RenderLayer::Transparent && !options.includeTransparent)
        return false;
    if (options.skip && options.skip(item))
        return false;

    // Cheap rejection with the item's world bounds
    float tNear = 0.0f, tFar = 0.0f;
    if (item.bounds.isValid() && (!intersectBox(ray, item.bounds, tNear, tFar) || tNear > maxT))
        return false;

    bool found = false;
    bool anyTriangles = false;
    auto test = [&](Mesh &mesh) {
        if (!mesh.enabled())
            return;
        if (mesh.bvh())
            anyTriangles = true;
        RayHit candidate;
        if (hitMesh(mesh, item.world, ray, found ? hit.distance : maxT, candidate))
        {
            hit = candidate;
            found = true;
        }
    };
    if (item.mesh != nullptr)
        test(*item.mesh);
    else if (auto *meshNode = dynamic_cast<MeshNode *>(item.node))
        for (auto &mesh : meshNode->meshes())
            test(*mesh);

    if (!found && !anyTriangles && options.boundsFallback && item.bounds.isValid())
    {
        const float t = std::max(tNear, 0.0f);
        if (t <= maxT)
        {
            hit.mesh = nullptr;
            hit.distance = t;
            hit.position = ray.at(t);
            hit.normal = boxNormal(item.bounds, hit.position);
            hit.triangle = -1;
            hit.barycentric = glm::vec2(0.0f);
            hit.exact = false;
            found = true;
        }
    }
    if (found)
        hit.node = item.node;
    return found;
}

} // namespace

RayHit ivf::pick(const RenderQueue &queue, const Ray &ray, const PickOptions &options)
{
    RayHit best;
    float maxT = options.maxDistance;
    for (const DrawItem &item : queue.items())
    {
        RayHit hit;
        if (hitItem(item, ray, maxT, options, hit))
        {
            best = hit;
            maxT = hit.distance;
        }
    }
    return best;
}

std::vector<RayHit> ivf::pickAll(const RenderQueue &queue, const Ray &ray, const PickOptions &options)
{
    std::vector<RayHit> hits;
    for (const DrawItem &item : queue.items())
    {
        RayHit hit;
        if (hitItem(item, ray, options.maxDistance, options, hit))
            hits.push_back(hit);
    }
    std::sort(hits.begin(), hits.end(), [](const RayHit &a, const RayHit &b) { return a.distance < b.distance; });
    return hits;
}
