#include <ivfvis/surface_data.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <unordered_map>

using namespace ivfvis;

float SurfaceData::area() const
{
    float sum = 0.0f;
    for (const glm::uvec3 &t : indices)
        sum += 0.5f * glm::length(glm::cross(positions[t.y] - positions[t.x], positions[t.z] - positions[t.x]));
    return sum;
}

namespace {

// ---------------------------------------------------------------------------------------
// Marching cubes cases, generated from the cube's geometry
//
// Corner c of a cell is at (c & 1, c >> 1 & 1, c >> 2 & 1). A corner is "inside" when its
// value is above the iso value. On each face the surface crosses the edges whose corners
// differ; a face with four crossings (diagonal corners alike) is split so that it cuts off
// the inside corners - the same in both cells sharing the face, so the surface has no
// cracks. Each face segment is directed with the inside corners on its left (seen from
// outside the cube), so the segments chain into consistently oriented loops. Loops are
// triangulated as fans whose diagonals avoid the cube's faces (a diagonal in a face could
// coincide with the neighbouring cell's), or around a centre vertex where no such fan exists.
// ---------------------------------------------------------------------------------------

struct Edge {
    int a, b; // corners, a < b
};

/** A closed polygon of edge crossings, triangulated as a fan from @c start (-1: around its centre) */
struct Loop {
    std::vector<std::uint8_t> edges;
    int start{0};
};

struct Cases {
    std::array<Edge, 12> edges{};
    std::array<std::vector<Loop>, 256> loops; // per case
};

glm::vec3 cornerPos(int c)
{
    return {static_cast<float>(c & 1), static_cast<float>((c >> 1) & 1), static_cast<float>((c >> 2) & 1)};
}

Cases makeCases()
{
    Cases cases;
    int edgeOf[8][8];
    for (auto &row : edgeOf)
        std::fill(std::begin(row), std::end(row), -1);
    int n = 0;
    for (int a = 0; a < 8; ++a)
        for (int bit = 1; bit < 8; bit <<= 1)
            if (!(a & bit))
            {
                const int b = a | bit;
                cases.edges[static_cast<std::size_t>(n)] = {a, b};
                edgeOf[a][b] = edgeOf[b][a] = n++;
            }

    // Faces as corner cycles, with their outward normals
    std::array<std::array<int, 4>, 6> faces{};
    std::array<glm::vec3, 6> faceNormals{};
    int f = 0;
    for (int axis = 0; axis < 3; ++axis)
    {
        const int u = (axis + 1) % 3, v = (axis + 2) % 3;
        for (int side = 0; side < 2; ++side)
        {
            const int base = side << axis;
            faces[static_cast<std::size_t>(f)] = {base, base | (1 << u), base | (1 << u) | (1 << v), base | (1 << v)};
            glm::vec3 n(0.0f);
            n[axis] = side ? 1.0f : -1.0f;
            faceNormals[static_cast<std::size_t>(f++)] = n;
        }
    }

    auto mid = [&](int edge) {
        const Edge &ed = cases.edges[static_cast<std::size_t>(edge)];
        return 0.5f * (cornerPos(ed.a) + cornerPos(ed.b));
    };

    std::array<std::vector<std::vector<int>>, 256> loops;
    for (int mask = 0; mask < 256; ++mask)
    {
        auto inside = [mask](int c) { return (mask >> c) & 1; };

        // Directed segments between crossed edges, face by face: seen from outside the
        // cube, the inside corners are on the left of each segment
        std::vector<std::pair<int, int>> segments;
        for (std::size_t fi = 0; fi < faces.size(); ++fi)
        {
            const auto &face = faces[fi];
            std::array<int, 4> e{};
            std::array<bool, 4> crossed{};
            int count = 0;
            for (int k = 0; k < 4; ++k)
            {
                const int c0 = face[static_cast<std::size_t>(k)], c1 = face[static_cast<std::size_t>((k + 1) % 4)];
                e[static_cast<std::size_t>(k)] = edgeOf[c0][c1];
                crossed[static_cast<std::size_t>(k)] = inside(c0) != inside(c1);
                count += crossed[static_cast<std::size_t>(k)] ? 1 : 0;
            }
            auto addDirected = [&](int p, int q, int insideCorner) {
                const glm::vec3 P = mid(p), Q = mid(q);
                const float side = glm::dot(glm::cross(faceNormals[fi], Q - P), cornerPos(insideCorner) - P);
                if (side >= 0.0f)
                    segments.emplace_back(p, q);
                else
                    segments.emplace_back(q, p);
            };
            if (count == 2)
            {
                int first = -1, second = -1;
                for (int k = 0; k < 4; ++k)
                    if (crossed[static_cast<std::size_t>(k)])
                        (first < 0 ? first : second) = e[static_cast<std::size_t>(k)];
                int insideCorner = -1;
                for (int c : face)
                    if (inside(c))
                        insideCorner = c;
                addDirected(first, second, insideCorner);
            }
            else if (count == 4)
            {
                // Cut off each inside corner: connect its two edges
                for (int k = 0; k < 4; ++k)
                    if (inside(face[static_cast<std::size_t>(k)]))
                        addDirected(e[static_cast<std::size_t>((k + 3) % 4)], e[static_cast<std::size_t>(k)],
                                    face[static_cast<std::size_t>(k)]);
            }
        }

        // Chain the directed segments into loops (each crossed edge starts and ends one)
        std::vector<bool> used(segments.size(), false);
        for (std::size_t s = 0; s < segments.size(); ++s)
        {
            if (used[s])
                continue;
            used[s] = true;
            std::vector<int> loop{segments[s].first};
            int current = segments[s].second;
            while (current != loop.front())
            {
                loop.push_back(current);
                bool advanced = false;
                for (std::size_t t = 0; t < segments.size(); ++t)
                    if (!used[t] && segments[t].first == current)
                    {
                        used[t] = true;
                        current = segments[t].second;
                        advanced = true;
                        break;
                    }
                if (!advanced)
                    break; // cannot happen for consistently directed segments
            }
            loops[static_cast<std::size_t>(mask)].push_back(std::move(loop));
        }
    }

    // One orientation for all: with corner 0 alone inside, the normal points away from it
    auto newell = [&](const std::vector<int> &loop) {
        glm::vec3 n(0.0f);
        for (std::size_t i = 0; i < loop.size(); ++i)
        {
            const glm::vec3 p = mid(loop[i]), q = mid(loop[(i + 1) % loop.size()]);
            n += glm::vec3((p.y - q.y) * (p.z + q.z), (p.z - q.z) * (p.x + q.x), (p.x - q.x) * (p.y + q.y));
        }
        return n;
    };
    const bool flip = glm::dot(newell(loops[1].front()), glm::vec3(1.0f)) < 0.0f;

    // Two edges on one cube face: a diagonal between them would lie in the face, where the
    // neighbouring cell's surface may run along the same line
    auto sameFace = [&](int e1, int e2) {
        const Edge &a = cases.edges[static_cast<std::size_t>(e1)], &b = cases.edges[static_cast<std::size_t>(e2)];
        const int all = a.a & a.b & b.a & b.b, any = a.a | a.b | b.a | b.b;
        return all != 0 || any != 7;
    };

    for (int mask = 0; mask < 256; ++mask)
        for (auto loop : loops[static_cast<std::size_t>(mask)])
        {
            if (flip)
                std::reverse(loop.begin(), loop.end());
            Loop out;
            for (int e : loop)
                out.edges.push_back(static_cast<std::uint8_t>(e));
            // A fan start whose diagonals leave the cube's faces; else triangulate around the centre
            const int n = static_cast<int>(loop.size());
            out.start = n <= 3 ? 0 : -1;
            for (int start = 0; start < n && out.start < 0; ++start)
            {
                bool ok = true;
                for (int j = 2; j < n - 1 && ok; ++j)
                    ok = !sameFace(loop[static_cast<std::size_t>(start)],
                                   loop[static_cast<std::size_t>((start + j) % n)]);
                if (ok)
                    out.start = start;
            }
            cases.loops[static_cast<std::size_t>(mask)].push_back(std::move(out));
        }
    return cases;
}

const Cases &marchingCases()
{
    static const Cases cases = makeCases(); // CPU data only, shared by all contexts
    return cases;
}

// ---------------------------------------------------------------------------------------
// Slices: polygon clipping against the grid's box
// ---------------------------------------------------------------------------------------

/** Keep the part of @p polygon with p[axis] * sign >= limit * sign */
std::vector<glm::vec3> clip(const std::vector<glm::vec3> &polygon, int axis, float limit, float sign)
{
    std::vector<glm::vec3> out;
    const std::size_t n = polygon.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        const glm::vec3 &a = polygon[i], &b = polygon[(i + 1) % n];
        const float da = (a[axis] - limit) * sign, db = (b[axis] - limit) * sign;
        if (da >= 0.0f)
            out.push_back(a);
        if ((da >= 0.0f) != (db >= 0.0f))
            out.push_back(a + (b - a) * (da / (da - db)));
    }
    return out;
}

} // namespace

SurfaceData ivfvis::extractIsosurface(const UniformGrid &grid, std::string_view scalars, float isoValue,
                                      std::string_view colorBy)
{
    SurfaceData surface;
    const glm::ivec3 dims = grid.dimensions();
    if (dims.x < 2 || dims.y < 2 || dims.z < 2)
        return surface;

    const auto values = grid.scalars(scalars);
    const std::span<const float> colors = colorBy.empty() ? std::span<const float>() : grid.scalars(colorBy);
    const Cases &cases = marchingCases();
    const glm::vec3 spacing = grid.spacing();
    const auto nx = static_cast<std::size_t>(dims.x), ny = static_cast<std::size_t>(dims.y);

    // Gradient at a grid point (central differences; one-sided at the boundary)
    auto gradient = [&](int i, int j, int k) {
        glm::vec3 g(0.0f);
        const int p[3] = {i, j, k};
        for (int a = 0; a < 3; ++a)
        {
            int lo[3] = {i, j, k}, hi[3] = {i, j, k};
            lo[a] = std::max(p[a] - 1, 0);
            hi[a] = std::min(p[a] + 1, dims[a] - 1);
            const float h = static_cast<float>(hi[a] - lo[a]) * spacing[a];
            if (h != 0.0f)
                g[a] = (values[grid.index(hi[0], hi[1], hi[2])] - values[grid.index(lo[0], lo[1], lo[2])]) / h;
        }
        return g;
    };

    std::unordered_map<std::uint64_t, std::uint32_t> vertexOf;
    const auto pointCount = static_cast<std::uint64_t>(grid.pointCount());

    for (int k = 0; k + 1 < dims.z; ++k)
        for (int j = 0; j + 1 < dims.y; ++j)
            for (int i = 0; i + 1 < dims.x; ++i)
            {
                int ci[8][3];
                std::size_t gi[8];
                float v[8];
                int mask = 0;
                for (int c = 0; c < 8; ++c)
                {
                    ci[c][0] = i + (c & 1);
                    ci[c][1] = j + ((c >> 1) & 1);
                    ci[c][2] = k + ((c >> 2) & 1);
                    gi[c] = static_cast<std::size_t>(ci[c][0]) +
                            nx * (static_cast<std::size_t>(ci[c][1]) + ny * static_cast<std::size_t>(ci[c][2]));
                    v[c] = values[gi[c]];
                    if (v[c] > isoValue)
                        mask |= 1 << c;
                }
                const auto &loops = cases.loops[static_cast<std::size_t>(mask)];
                if (loops.empty())
                    continue;

                auto vertex = [&](int edge) {
                    const Edge &e = cases.edges[static_cast<std::size_t>(edge)];
                    const std::uint64_t key = static_cast<std::uint64_t>(gi[e.a]) * pointCount + gi[e.b];
                    if (auto it = vertexOf.find(key); it != vertexOf.end())
                        return it->second;
                    const float t = (isoValue - v[e.a]) / (v[e.b] - v[e.a]);
                    const glm::vec3 pa = grid.position(ci[e.a][0], ci[e.a][1], ci[e.a][2]);
                    const glm::vec3 pb = grid.position(ci[e.b][0], ci[e.b][1], ci[e.b][2]);
                    glm::vec3 g = glm::mix(gradient(ci[e.a][0], ci[e.a][1], ci[e.a][2]),
                                           gradient(ci[e.b][0], ci[e.b][1], ci[e.b][2]), t);
                    const auto index = static_cast<std::uint32_t>(surface.positions.size());
                    surface.positions.push_back(glm::mix(pa, pb, t));
                    surface.normals.push_back(glm::length(g) > 0.0f ? -glm::normalize(g) : glm::vec3(0.0f));
                    surface.values.push_back(colors.empty() ? isoValue
                                                            : glm::mix(colors[gi[e.a]], colors[gi[e.b]], t));
                    vertexOf.emplace(key, index);
                    return index;
                };

                auto addTriangle = [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
                    if (a != b && b != c && a != c)
                        surface.indices.emplace_back(a, b, c);
                };
                for (const Loop &loop : loops)
                {
                    std::uint32_t ids[12];
                    const std::size_t n = loop.edges.size();
                    for (std::size_t q = 0; q < n; ++q)
                        ids[q] = vertex(loop.edges[q]);
                    if (loop.start >= 0)
                    {
                        const auto s0 = static_cast<std::size_t>(loop.start);
                        for (std::size_t q = 1; q + 1 < n; ++q)
                            addTriangle(ids[s0], ids[(s0 + q) % n], ids[(s0 + q + 1) % n]);
                        continue;
                    }
                    // Around a vertex at the loop's centre (inside the cell, not shared)
                    glm::vec3 position(0.0f), normal(0.0f);
                    float value = 0.0f;
                    for (std::size_t q = 0; q < n; ++q)
                    {
                        position += surface.positions[ids[q]];
                        normal += surface.normals[ids[q]];
                        value += surface.values[ids[q]];
                    }
                    const auto centre = static_cast<std::uint32_t>(surface.positions.size());
                    surface.positions.push_back(position / static_cast<float>(n));
                    surface.normals.push_back(glm::length(normal) > 0.0f ? glm::normalize(normal) : glm::vec3(0.0f));
                    surface.values.push_back(value / static_cast<float>(n));
                    for (std::size_t q = 0; q < n; ++q)
                        addTriangle(centre, ids[q], ids[(q + 1) % n]);
                }
            }

    // Vertices with a zero gradient (flat regions): the normal of an adjacent triangle
    for (const glm::uvec3 &t : surface.indices)
    {
        const glm::vec3 n = glm::cross(surface.positions[t.y] - surface.positions[t.x],
                                       surface.positions[t.z] - surface.positions[t.x]);
        if (glm::length(n) == 0.0f)
            continue;
        for (unsigned idx : {t.x, t.y, t.z})
            if (surface.normals[idx] == glm::vec3(0.0f))
                surface.normals[idx] = glm::normalize(n);
    }
    return surface;
}

SurfaceData ivfvis::extractSlice(const UniformGrid &grid, std::string_view scalars, const glm::vec3 &origin,
                                 const glm::vec3 &planeNormal, int resolution)
{
    SurfaceData surface;
    static_cast<void>(grid.scalars(scalars)); // throws for a missing array
    if (glm::length(planeNormal) == 0.0f)
        return surface;
    const glm::vec3 n = glm::normalize(planeNormal);
    const ivf::BoundingBox box = grid.bounds();
    const glm::vec3 lo = box.min(), hi = box.max();

    // Basis of the plane, and the rectangle covering the box's projection
    const glm::vec3 helper = std::abs(n.x) < 0.9f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    const glm::vec3 u = glm::normalize(glm::cross(helper, n));
    const glm::vec3 v = glm::cross(n, u);
    const glm::vec3 o = origin;
    float umin = 1e30f, umax = -1e30f, vmin = 1e30f, vmax = -1e30f;
    for (int c = 0; c < 8; ++c)
    {
        const glm::vec3 corner(c & 1 ? hi.x : lo.x, c & 2 ? hi.y : lo.y, c & 4 ? hi.z : lo.z);
        const float cu = glm::dot(corner - o, u), cv = glm::dot(corner - o, v);
        umin = std::min(umin, cu);
        umax = std::max(umax, cu);
        vmin = std::min(vmin, cv);
        vmax = std::max(vmax, cv);
    }
    const float step = std::max(umax - umin, vmax - vmin) / static_cast<float>(std::max(resolution, 1));
    if (!(step > 0.0f))
        return surface;
    const int nu = std::max(1, static_cast<int>(std::ceil((umax - umin) / step)));
    const int nv = std::max(1, static_cast<int>(std::ceil((vmax - vmin) / step)));

    // Squares fully inside the box share vertices; squares on its border are clipped
    const float eps = 1e-5f * glm::length(hi - lo);
    auto insideBox = [&](const glm::vec3 &p) {
        return p.x >= lo.x - eps && p.y >= lo.y - eps && p.z >= lo.z - eps && p.x <= hi.x + eps &&
               p.y <= hi.y + eps && p.z <= hi.z + eps;
    };
    auto at = [&](int i, int j) {
        return o + u * (umin + static_cast<float>(i) * step) + v * (vmin + static_cast<float>(j) * step);
    };
    std::vector<std::int64_t> shared(static_cast<std::size_t>((nu + 1) * (nv + 1)), -1);
    auto addVertex = [&](const glm::vec3 &p) {
        surface.positions.push_back(p);
        surface.normals.push_back(n);
        surface.values.push_back(grid.sample(scalars, p));
        return static_cast<std::uint32_t>(surface.positions.size() - 1);
    };
    auto sharedVertex = [&](int i, int j) {
        auto &slot = shared[static_cast<std::size_t>(j * (nu + 1) + i)];
        if (slot < 0)
            slot = addVertex(at(i, j));
        return static_cast<std::uint32_t>(slot);
    };

    for (int j = 0; j < nv; ++j)
        for (int i = 0; i < nu; ++i)
        {
            const glm::vec3 corners[4] = {at(i, j), at(i + 1, j), at(i + 1, j + 1), at(i, j + 1)};
            const bool all = insideBox(corners[0]) && insideBox(corners[1]) && insideBox(corners[2]) &&
                             insideBox(corners[3]);
            if (all)
            {
                const std::uint32_t a = sharedVertex(i, j), b = sharedVertex(i + 1, j), c = sharedVertex(i + 1, j + 1),
                                    d = sharedVertex(i, j + 1);
                surface.indices.emplace_back(a, b, c);
                surface.indices.emplace_back(a, c, d);
                continue;
            }
            std::vector<glm::vec3> polygon(corners, corners + 4);
            for (int axis = 0; axis < 3 && polygon.size() >= 3; ++axis)
            {
                polygon = clip(polygon, axis, lo[axis], 1.0f);
                if (polygon.size() >= 3)
                    polygon = clip(polygon, axis, hi[axis], -1.0f);
            }
            if (polygon.size() < 3)
                continue;
            const std::uint32_t first = addVertex(polygon[0]);
            std::uint32_t previous = addVertex(polygon[1]);
            for (std::size_t p = 2; p < polygon.size(); ++p)
            {
                const std::uint32_t next = addVertex(polygon[p]);
                surface.indices.emplace_back(first, previous, next);
                previous = next;
            }
        }
    // Drop slivers left by clipping
    std::erase_if(surface.indices, [&](const glm::uvec3 &t) {
        return glm::length(glm::cross(surface.positions[t.y] - surface.positions[t.x],
                                      surface.positions[t.z] - surface.positions[t.x])) <= eps * eps;
    });
    return surface;
}

std::vector<glm::vec3> ivfvis::contourLines(const SurfaceData &surface, std::span<const float> levels)
{
    std::vector<glm::vec3> lines;
    if (surface.values.size() != surface.positions.size())
        return lines;
    for (float level : levels)
        for (const glm::uvec3 &t : surface.indices)
        {
            const unsigned idx[3] = {t.x, t.y, t.z};
            glm::vec3 points[2];
            int found = 0;
            for (int e = 0; e < 3 && found < 2; ++e)
            {
                const unsigned a = idx[e], b = idx[(e + 1) % 3];
                const float va = surface.values[a], vb = surface.values[b];
                if ((va > level) == (vb > level))
                    continue;
                const float s = (level - va) / (vb - va);
                points[found++] = glm::mix(surface.positions[a], surface.positions[b], s);
            }
            if (found == 2)
            {
                lines.push_back(points[0]);
                lines.push_back(points[1]);
            }
        }
    return lines;
}
