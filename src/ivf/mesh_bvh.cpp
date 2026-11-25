#include <ivf/mesh_bvh.h>

#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>
#include <cmath>

using namespace ivf;

// ---- Rays ---------------------------------------------------------------------

Ray ivf::rayFromScreen(float x, float y, float width, float height, const glm::mat4 &view, const glm::mat4 &projection)
{
    const float nx = 2.0f * x / std::max(width, 1.0f) - 1.0f;
    const float ny = 1.0f - 2.0f * y / std::max(height, 1.0f);
    const glm::mat4 inverse = glm::inverse(projection * view);
    glm::vec4 nearPoint = inverse * glm::vec4(nx, ny, -1.0f, 1.0f);
    glm::vec4 farPoint = inverse * glm::vec4(nx, ny, 1.0f, 1.0f);
    const glm::vec3 a = glm::vec3(nearPoint) / nearPoint.w;
    const glm::vec3 b = glm::vec3(farPoint) / farPoint.w;
    const glm::vec3 d = b - a;
    return {a, glm::length(d) > 0.0f ? glm::normalize(d) : glm::vec3(0.0f, 0.0f, -1.0f)};
}

bool ivf::intersectBox(const Ray &ray, const glm::vec3 &boxMin, const glm::vec3 &boxMax, float &tNear, float &tFar)
{
    tNear = -std::numeric_limits<float>::infinity();
    tFar = std::numeric_limits<float>::infinity();
    for (int i = 0; i < 3; ++i)
    {
        const float d = ray.direction[i];
        const float o = ray.origin[i];
        if (std::abs(d) < 1e-20f)
        {
            if (o < boxMin[i] || o > boxMax[i])
                return false;
            continue;
        }
        const float inv = 1.0f / d;
        float t0 = (boxMin[i] - o) * inv;
        float t1 = (boxMax[i] - o) * inv;
        if (t0 > t1)
            std::swap(t0, t1);
        tNear = std::max(tNear, t0);
        tFar = std::min(tFar, t1);
        if (tNear > tFar)
            return false;
    }
    return tFar >= 0.0f;
}

bool ivf::intersectBox(const Ray &ray, const BoundingBox &box, float &tNear, float &tFar)
{
    if (!box.isValid())
        return false;
    return intersectBox(ray, box.min(), box.max(), tNear, tFar);
}

bool ivf::intersectTriangle(const Ray &ray, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, float &t,
                            float &u, float &v)
{
    const glm::vec3 e1 = v1 - v0;
    const glm::vec3 e2 = v2 - v0;
    const glm::vec3 p = glm::cross(ray.direction, e2);
    const float det = glm::dot(e1, p);
    if (std::abs(det) < 1e-12f)
        return false; // parallel or degenerate
    const float invDet = 1.0f / det;
    const glm::vec3 s = ray.origin - v0;
    u = glm::dot(s, p) * invDet;
    if (u < 0.0f || u > 1.0f)
        return false;
    const glm::vec3 q = glm::cross(s, e1);
    v = glm::dot(ray.direction, q) * invDet;
    if (v < 0.0f || u + v > 1.0f)
        return false;
    t = glm::dot(e2, q) * invDet;
    return t > 1e-7f;
}

// ---- BVH ----------------------------------------------------------------------

namespace {
constexpr std::uint32_t LeafSize = 4;
}

MeshBvh::MeshBvh(std::vector<glm::vec3> positions, std::vector<glm::uvec3> triangles)
    : m_positions(std::move(positions)), m_triangles(std::move(triangles))
{
    // Drop triangles that refer to missing vertices
    const auto vertexCount = static_cast<std::uint32_t>(m_positions.size());
    m_triangles.erase(std::remove_if(m_triangles.begin(), m_triangles.end(),
                                     [vertexCount](const glm::uvec3 &t) {
                                         return t.x >= vertexCount || t.y >= vertexCount || t.z >= vertexCount;
                                     }),
                      m_triangles.end());
    if (m_triangles.empty())
        return;

    const auto count = static_cast<std::uint32_t>(m_triangles.size());
    m_order.resize(count);
    std::vector<glm::vec3> centroids(count), lo(count), hi(count);
    for (std::uint32_t i = 0; i < count; ++i)
    {
        m_order[i] = i;
        const glm::vec3 &a = m_positions[m_triangles[i].x], &b = m_positions[m_triangles[i].y],
                        &c = m_positions[m_triangles[i].z];
        lo[i] = glm::min(a, glm::min(b, c));
        hi[i] = glm::max(a, glm::max(b, c));
        centroids[i] = (a + b + c) / 3.0f;
    }
    m_nodes.reserve(2 * count);

    struct Task {
        std::uint32_t node, first, count;
        int depth;
    };
    std::vector<Task> stack;
    m_nodes.push_back({});
    stack.push_back({0, 0, count, 1});
    while (!stack.empty())
    {
        const Task task = stack.back();
        stack.pop_back();
        m_depth = std::max(m_depth, task.depth);

        glm::vec3 bmin(std::numeric_limits<float>::infinity()), bmax(-std::numeric_limits<float>::infinity());
        glm::vec3 cmin = bmin, cmax = bmax;
        for (std::uint32_t i = task.first; i < task.first + task.count; ++i)
        {
            const std::uint32_t t = m_order[i];
            bmin = glm::min(bmin, lo[t]);
            bmax = glm::max(bmax, hi[t]);
            cmin = glm::min(cmin, centroids[t]);
            cmax = glm::max(cmax, centroids[t]);
        }
        Node &node = m_nodes[task.node];
        node.min = bmin;
        node.max = bmax;

        const glm::vec3 extent = cmax - cmin;
        const int axis = extent.x >= extent.y && extent.x >= extent.z ? 0 : (extent.y >= extent.z ? 1 : 2);
        if (task.count <= LeafSize || extent[axis] <= 0.0f)
        {
            node.offset = task.first;
            node.count = task.count;
            continue;
        }

        // Split at the median centroid along the longest axis
        const std::uint32_t half = task.count / 2;
        auto begin = m_order.begin() + task.first;
        std::nth_element(begin, begin + half, begin + task.count, [&](std::uint32_t a, std::uint32_t b) {
            return centroids[a][axis] < centroids[b][axis];
        });

        // The two children are stored next to each other: left at offset, right at offset + 1
        const auto left = static_cast<std::uint32_t>(m_nodes.size());
        m_nodes.push_back({});
        m_nodes.push_back({});
        m_nodes[task.node].offset = left;
        m_nodes[task.node].count = 0;
        stack.push_back({left + 1, task.first + half, task.count - half, task.depth + 1});
        stack.push_back({left, task.first, half, task.depth + 1});
    }
}

template <typename Visit> void MeshBvh::traverse(const Ray &ray, float &maxT, Visit &&visit) const
{
    if (m_nodes.empty())
        return;
    std::uint32_t stack[64];
    int top = 0;
    stack[top++] = 0;
    while (top > 0)
    {
        const Node &node = m_nodes[stack[--top]];
        float tNear, tFar;
        if (!intersectBox(ray, node.min, node.max, tNear, tFar) || tNear > maxT)
            continue;
        if (node.count > 0)
        {
            for (std::uint32_t i = node.offset; i < node.offset + node.count; ++i)
                if (visit(m_order[i]))
                    return; // stop
            continue;
        }
        // Children: nearer one first
        const std::uint32_t a = node.offset, b = node.offset + 1;
        float ta0, ta1, tb0, tb1;
        const bool hitA = intersectBox(ray, m_nodes[a].min, m_nodes[a].max, ta0, ta1) && ta0 <= maxT;
        const bool hitB = intersectBox(ray, m_nodes[b].min, m_nodes[b].max, tb0, tb1) && tb0 <= maxT;
        if (top + 2 > 64)
            continue; // cannot happen for median splits below 2^60 triangles
        if (hitA && hitB)
        {
            if (ta0 <= tb0)
            {
                stack[top++] = b;
                stack[top++] = a;
            }
            else
            {
                stack[top++] = a;
                stack[top++] = b;
            }
        }
        else if (hitA)
            stack[top++] = a;
        else if (hitB)
            stack[top++] = b;
    }
}

std::optional<MeshBvh::Hit> MeshBvh::intersect(const Ray &ray, float maxT) const
{
    std::optional<Hit> best;
    traverse(ray, maxT, [&](std::uint32_t index) {
        const glm::uvec3 &tri = m_triangles[index];
        float t, u, v;
        if (intersectTriangle(ray, m_positions[tri.x], m_positions[tri.y], m_positions[tri.z], t, u, v) && t < maxT)
        {
            maxT = t;
            best = Hit{t, index, u, v};
        }
        return false;
    });
    return best;
}

bool MeshBvh::occluded(const Ray &ray, float maxT) const
{
    bool hit = false;
    traverse(ray, maxT, [&](std::uint32_t index) {
        const glm::uvec3 &tri = m_triangles[index];
        float t, u, v;
        hit = intersectTriangle(ray, m_positions[tri.x], m_positions[tri.y], m_positions[tri.z], t, u, v) && t < maxT;
        return hit;
    });
    return hit;
}

BoundingBox MeshBvh::bounds() const
{
    if (m_nodes.empty())
        return BoundingBox();
    return BoundingBox(m_nodes[0].min, m_nodes[0].max);
}

glm::vec3 MeshBvh::faceNormal(std::uint32_t index) const
{
    const glm::uvec3 &t = m_triangles[index];
    return glm::cross(m_positions[t.y] - m_positions[t.x], m_positions[t.z] - m_positions[t.x]);
}
