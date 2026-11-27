#include "convex_hull.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <numbers>
#include <set>
#include <unordered_map>

using namespace ivfphysics;

namespace {

using dvec3 = glm::dvec3;
using dvec2 = glm::dvec2;

struct Face {
    std::uint32_t v[3];
    dvec3 n;
    double d;
    bool alive{true};
};

std::uint64_t edgeKey(std::uint32_t a, std::uint32_t b)
{
    return (static_cast<std::uint64_t>(a) << 32) | b;
}

class HullBuilder {
public:
    HullBuilder(std::vector<dvec3> points, double tolerance) : m_p(std::move(points)), m_tol(tolerance)
    {}

    bool build(std::string &error)
    {
        const auto n = static_cast<std::uint32_t>(m_p.size());
        if (n < 4)
        {
            error = "at least 4 points are needed";
            return false;
        }

        // Initial tetrahedron from extreme points
        std::uint32_t i0 = 0;
        for (std::uint32_t i = 1; i < n; ++i)
            if (m_p[i].x < m_p[i0].x)
                i0 = i;
        std::uint32_t i1 = i0;
        double best = 0.0;
        for (std::uint32_t i = 0; i < n; ++i)
            if (double dist = glm::length(m_p[i] - m_p[i0]); dist > best)
            {
                best = dist;
                i1 = i;
            }
        if (best <= m_tol)
        {
            error = "all points are at one place";
            return false;
        }
        const dvec3 axis = glm::normalize(m_p[i1] - m_p[i0]);
        std::uint32_t i2 = i0;
        best = 0.0;
        for (std::uint32_t i = 0; i < n; ++i)
        {
            const dvec3 r = m_p[i] - m_p[i0];
            if (double dist = glm::length(r - axis * glm::dot(r, axis)); dist > best)
            {
                best = dist;
                i2 = i;
            }
        }
        if (best <= m_tol)
        {
            error = "all points are on a line";
            return false;
        }
        const dvec3 normal = glm::normalize(glm::cross(m_p[i1] - m_p[i0], m_p[i2] - m_p[i0]));
        std::uint32_t i3 = i0;
        best = 0.0;
        for (std::uint32_t i = 0; i < n; ++i)
            if (double dist = std::abs(glm::dot(m_p[i] - m_p[i0], normal)); dist > best)
            {
                best = dist;
                i3 = i;
            }
        if (best <= m_tol)
        {
            error = "all points are in one plane";
            return false;
        }

        m_interior = 0.25 * (m_p[i0] + m_p[i1] + m_p[i2] + m_p[i3]);
        addOutward(i0, i1, i2);
        addOutward(i0, i1, i3);
        addOutward(i0, i2, i3);
        addOutward(i1, i2, i3);

        // Add the points farthest from the centre first: fewer faces are built and removed
        std::vector<std::uint32_t> order;
        for (std::uint32_t i = 0; i < n; ++i)
            if (i != i0 && i != i1 && i != i2 && i != i3)
                order.push_back(i);
        std::stable_sort(order.begin(), order.end(), [&](std::uint32_t a, std::uint32_t b) {
            return glm::length(m_p[a] - m_interior) > glm::length(m_p[b] - m_interior);
        });
        for (const auto i : order)
            addPoint(i);
        return true;
    }

    /** Merge coplanar triangles into polygons and keep only corner vertices. */
    bool polygons(detail::Polyhedron &hull) const
    {
        std::vector<std::uint32_t> alive;
        for (std::uint32_t f = 0; f < m_faces.size(); ++f)
            if (m_faces[f].alive)
                alive.push_back(f);

        std::vector<int> group(m_faces.size(), -1);
        std::vector<std::vector<std::uint32_t>> polys;
        for (const auto seed : alive)
        {
            if (group[seed] >= 0)
                continue;
            const int g = static_cast<int>(polys.size());
            const Face &s = m_faces[seed];
            std::vector<std::uint32_t> stack{seed};
            std::set<std::uint32_t> verts;
            group[seed] = g;
            while (!stack.empty())
            {
                const Face &f = m_faces[stack.back()];
                stack.pop_back();
                for (int e = 0; e < 3; ++e)
                {
                    verts.insert(f.v[e]);
                    const auto it = m_edges.find(edgeKey(f.v[(e + 1) % 3], f.v[e]));
                    if (it == m_edges.end() || group[it->second] >= 0)
                        continue;
                    const Face &other = m_faces[it->second];
                    bool coplanar = glm::dot(other.n, s.n) > 0.0;
                    for (int k = 0; k < 3 && coplanar; ++k)
                        coplanar = std::abs(glm::dot(m_p[other.v[k]], s.n) - s.d) <= m_tol;
                    if (coplanar)
                    {
                        group[it->second] = g;
                        stack.push_back(it->second);
                    }
                }
            }
            polys.push_back(outline(s.n, verts));
        }

        // Every edge must be shared by exactly two faces, in opposite directions
        std::map<std::pair<std::uint32_t, std::uint32_t>, int> edges;
        for (const auto &poly : polys)
        {
            if (poly.size() < 3)
                return false;
            for (std::size_t k = 0; k < poly.size(); ++k)
                edges[{poly[k], poly[(k + 1) % poly.size()]}]++;
        }
        std::set<std::uint32_t> used;
        for (const auto &[edge, count] : edges)
        {
            if (count != 1 || edges.find({edge.second, edge.first}) == edges.end())
                return false;
            used.insert(edge.first);
        }
        if (used.size() + polys.size() != edges.size() / 2 + 2) // Euler: V - E + F = 2
            return false;

        emit(polys, used, hull);
        return true;
    }

    /** The hull's triangles as faces (always consistent; used if merging fails). */
    void triangles(detail::Polyhedron &hull) const
    {
        std::vector<std::vector<std::uint32_t>> polys;
        std::set<std::uint32_t> used;
        for (const auto &f : m_faces)
            if (f.alive)
            {
                polys.push_back({f.v[0], f.v[1], f.v[2]});
                used.insert(f.v, f.v + 3);
            }
        emit(polys, used, hull);
    }

private:
    void addFace(std::uint32_t a, std::uint32_t b, std::uint32_t c)
    {
        Face f{{a, b, c}, dvec3(0.0), 0.0, true};
        const dvec3 cr = glm::cross(m_p[b] - m_p[a], m_p[c] - m_p[a]);
        const double len = glm::length(cr);
        f.n = len > 0.0 ? cr / len : dvec3(0.0);
        f.d = glm::dot(f.n, m_p[a]);
        const auto index = static_cast<std::uint32_t>(m_faces.size());
        m_faces.push_back(f);
        m_edges[edgeKey(a, b)] = index;
        m_edges[edgeKey(b, c)] = index;
        m_edges[edgeKey(c, a)] = index;
    }

    void addOutward(std::uint32_t a, std::uint32_t b, std::uint32_t c)
    {
        const dvec3 n = glm::cross(m_p[b] - m_p[a], m_p[c] - m_p[a]);
        if (glm::dot(n, m_interior - m_p[a]) > 0.0)
            addFace(a, c, b);
        else
            addFace(a, b, c);
    }

    void addPoint(std::uint32_t i)
    {
        const dvec3 &p = m_p[i];
        std::vector<std::uint32_t> visible;
        for (std::uint32_t f = 0; f < m_faces.size(); ++f)
            if (m_faces[f].alive && glm::dot(m_faces[f].n, p) - m_faces[f].d > m_tol)
                visible.push_back(f);
        if (visible.empty())
            return; // inside (or within the tolerance of the surface)

        std::set<std::uint32_t> visibleSet(visible.begin(), visible.end());
        std::vector<std::pair<std::uint32_t, std::uint32_t>> horizon;
        for (const auto f : visible)
            for (int e = 0; e < 3; ++e)
            {
                const std::uint32_t a = m_faces[f].v[e];
                const std::uint32_t b = m_faces[f].v[(e + 1) % 3];
                const auto twin = m_edges.find(edgeKey(b, a));
                if (twin != m_edges.end() && !visibleSet.contains(twin->second))
                    horizon.emplace_back(a, b);
            }
        for (const auto f : visible)
        {
            m_faces[f].alive = false;
            for (int e = 0; e < 3; ++e)
            {
                const auto it = m_edges.find(edgeKey(m_faces[f].v[e], m_faces[f].v[(e + 1) % 3]));
                if (it != m_edges.end() && it->second == f)
                    m_edges.erase(it);
            }
        }
        for (const auto &[a, b] : horizon)
            addFace(a, b, i);
    }

    /** Corners of the convex polygon of @p verts in the plane with normal @p n, counter-clockwise. */
    std::vector<std::uint32_t> outline(const dvec3 &n, const std::set<std::uint32_t> &verts) const
    {
        const dvec3 helper = std::abs(n.x) < 0.6 ? dvec3(1.0, 0.0, 0.0) : dvec3(0.0, 1.0, 0.0);
        const dvec3 u = glm::normalize(glm::cross(helper, n));
        const dvec3 v = glm::cross(n, u); // (u, v, n) right-handed: counter-clockwise seen from outside
        struct P {
            dvec2 q;
            std::uint32_t id;
        };
        std::vector<P> pts;
        for (const auto id : verts)
            pts.push_back({{glm::dot(m_p[id], u), glm::dot(m_p[id], v)}, id});
        std::sort(pts.begin(), pts.end(), [](const P &a, const P &b) {
            return a.q.x < b.q.x || (a.q.x == b.q.x && (a.q.y < b.q.y || (a.q.y == b.q.y && a.id < b.id)));
        });
        // Andrew's monotone chain; points within the tolerance of an edge are dropped
        auto turnsLeft = [&](const P &o, const P &a, const P &b) {
            const dvec2 ob = b.q - o.q;
            const double cr = (a.q.x - o.q.x) * ob.y - (a.q.y - o.q.y) * ob.x;
            return cr > m_tol * glm::length(ob);
        };
        std::vector<P> chain;
        for (int pass = 0; pass < 2; ++pass)
        {
            const std::size_t start = chain.size();
            for (std::size_t k = 0; k < pts.size(); ++k)
            {
                const P &p = pass == 0 ? pts[k] : pts[pts.size() - 1 - k];
                while (chain.size() >= start + 2 && !turnsLeft(chain[chain.size() - 2], chain.back(), p))
                    chain.pop_back();
                chain.push_back(p);
            }
            chain.pop_back(); // the last point starts the other half
        }
        std::vector<std::uint32_t> result;
        for (const auto &p : chain)
            result.push_back(p.id);
        return result;
    }

    void emit(const std::vector<std::vector<std::uint32_t>> &polys, const std::set<std::uint32_t> &used,
              detail::Polyhedron &hull) const
    {
        std::unordered_map<std::uint32_t, std::uint32_t> remap;
        hull.vertices.clear();
        hull.faces.clear();
        for (const auto id : used)
        {
            remap[id] = static_cast<std::uint32_t>(hull.vertices.size());
            hull.vertices.emplace_back(m_p[id]);
        }
        for (const auto &poly : polys)
        {
            std::vector<std::uint32_t> face;
            for (const auto id : poly)
                face.push_back(remap.at(id));
            hull.faces.push_back(std::move(face));
        }
    }

    std::vector<dvec3> m_p;
    double m_tol;
    dvec3 m_interior{0.0};
    std::vector<Face> m_faces;
    std::unordered_map<std::uint64_t, std::uint32_t> m_edges; ///< Directed edge -> face.
};

/** Extreme points of @p points in @p count directions spread evenly over the sphere. */
std::vector<dvec3> supportPoints(const std::vector<glm::vec3> &points, std::size_t count)
{
    std::set<std::size_t> chosen;
    const double golden = std::numbers::pi * (3.0 - std::sqrt(5.0));
    for (std::size_t k = 0; k < count; ++k)
    {
        const double z = 1.0 - 2.0 * (static_cast<double>(k) + 0.5) / static_cast<double>(count);
        const double r = std::sqrt(std::max(1.0 - z * z, 0.0));
        const double phi = golden * static_cast<double>(k);
        const dvec3 dir(r * std::cos(phi), r * std::sin(phi), z);
        std::size_t best = 0;
        double bestDot = -std::numeric_limits<double>::max();
        for (std::size_t i = 0; i < points.size(); ++i)
            if (const double d = glm::dot(dvec3(points[i]), dir); d > bestDot)
            {
                bestDot = d;
                best = i;
            }
        chosen.insert(best);
    }
    std::vector<dvec3> result;
    for (const auto i : chosen)
        result.emplace_back(points[i]);
    return result;
}

} // namespace

bool detail::convexHull(const std::vector<glm::vec3> &points, Polyhedron &hull, std::string &error,
                        std::size_t maxInputPoints)
{
    std::vector<dvec3> input;
    if (points.size() > maxInputPoints)
        input = supportPoints(points, maxInputPoints);
    else
        for (const auto &p : points)
            input.emplace_back(p);

    dvec3 lo(std::numeric_limits<double>::max());
    dvec3 hi(std::numeric_limits<double>::lowest());
    for (const auto &p : input)
    {
        lo = glm::min(lo, p);
        hi = glm::max(hi, p);
    }
    const double extent = input.empty() ? 0.0 : glm::length(hi - lo);
    HullBuilder builder(std::move(input), std::max(1e-5 * extent, 1e-9));
    if (!builder.build(error))
        return false;
    if (!builder.polygons(hull))
        builder.triangles(hull);
    return true;
}
