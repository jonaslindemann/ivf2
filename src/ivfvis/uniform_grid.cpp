#include <ivfvis/uniform_grid.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace ivfvis;

UniformGrid::UniformGrid(const glm::ivec3 &dimensions, const glm::vec3 &origin, const glm::vec3 &spacing)
    : m_dimensions(glm::max(dimensions, glm::ivec3(1))), m_origin(origin), m_spacing(spacing)
{
}

std::shared_ptr<UniformGrid> UniformGrid::create(const glm::ivec3 &dimensions, const glm::vec3 &origin,
                                                 const glm::vec3 &spacing)
{
    return std::make_shared<UniformGrid>(dimensions, origin, spacing);
}

std::shared_ptr<UniformGrid> UniformGrid::create(const glm::ivec3 &dimensions, const ivf::BoundingBox &bounds)
{
    const glm::ivec3 dims = glm::max(dimensions, glm::ivec3(1));
    const glm::vec3 size = bounds.size();
    glm::vec3 spacing(1.0f);
    for (int a = 0; a < 3; ++a)
        spacing[a] = dims[a] > 1 ? size[a] / static_cast<float>(dims[a] - 1) : 1.0f;
    return std::make_shared<UniformGrid>(dims, bounds.min(), spacing);
}

glm::ivec3 UniformGrid::dimensions() const
{
    return m_dimensions;
}

glm::vec3 UniformGrid::origin() const
{
    return m_origin;
}

glm::vec3 UniformGrid::spacing() const
{
    return m_spacing;
}

void UniformGrid::setOrigin(const glm::vec3 &origin)
{
    m_origin = origin;
}

void UniformGrid::setSpacing(const glm::vec3 &spacing)
{
    m_spacing = spacing;
}

std::size_t UniformGrid::pointCount() const
{
    return static_cast<std::size_t>(m_dimensions.x) * static_cast<std::size_t>(m_dimensions.y) *
           static_cast<std::size_t>(m_dimensions.z);
}

glm::vec3 UniformGrid::position(std::size_t index) const
{
    const auto nx = static_cast<std::size_t>(m_dimensions.x);
    const auto ny = static_cast<std::size_t>(m_dimensions.y);
    const auto i = static_cast<int>(index % nx);
    const auto j = static_cast<int>((index / nx) % ny);
    const auto k = static_cast<int>(index / (nx * ny));
    return position(i, j, k);
}

glm::vec3 UniformGrid::position(int i, int j, int k) const
{
    return m_origin + glm::vec3(static_cast<float>(i), static_cast<float>(j), static_cast<float>(k)) * m_spacing;
}

std::size_t UniformGrid::index(int i, int j, int k) const
{
    return static_cast<std::size_t>(i) +
           static_cast<std::size_t>(m_dimensions.x) *
               (static_cast<std::size_t>(j) + static_cast<std::size_t>(m_dimensions.y) * static_cast<std::size_t>(k));
}

ivf::BoundingBox UniformGrid::bounds() const
{
    const glm::vec3 far = position(m_dimensions.x - 1, m_dimensions.y - 1, m_dimensions.z - 1);
    return ivf::BoundingBox(glm::min(m_origin, far), glm::max(m_origin, far));
}

bool UniformGrid::contains(const glm::vec3 &p) const
{
    const ivf::BoundingBox box = bounds();
    const glm::vec3 lo = box.min(), hi = box.max();
    return p.x >= lo.x && p.y >= lo.y && p.z >= lo.z && p.x <= hi.x && p.y <= hi.y && p.z <= hi.z;
}

UniformGrid::Cell UniformGrid::cell(const glm::vec3 &p) const
{
    Cell c{};
    std::size_t *lo[3] = {&c.i0, &c.j0, &c.k0};
    std::size_t *hi[3] = {&c.i1, &c.j1, &c.k1};
    for (int a = 0; a < 3; ++a)
    {
        const int n = m_dimensions[a];
        if (n == 1 || m_spacing[a] == 0.0f)
        {
            *lo[a] = *hi[a] = 0;
            c.t[a] = 0.0f;
            continue;
        }
        float u = (p[a] - m_origin[a]) / m_spacing[a];
        u = std::clamp(u, 0.0f, static_cast<float>(n - 1));
        if (u != u)
            u = 0.0f; // NaN position
        const int i0 = std::min(static_cast<int>(std::floor(u)), n - 2);
        *lo[a] = static_cast<std::size_t>(i0);
        *hi[a] = static_cast<std::size_t>(i0 + 1);
        c.t[a] = u - static_cast<float>(i0);
    }
    return c;
}

namespace {

template <typename T, typename Values>
T trilinear(const Values &v, std::size_t nx, std::size_t ny, std::size_t i0, std::size_t j0, std::size_t k0,
            std::size_t i1, std::size_t j1, std::size_t k1, const glm::vec3 &t)
{
    auto at = [&](std::size_t i, std::size_t j, std::size_t k) { return v[i + nx * (j + ny * k)]; };
    const T c00 = at(i0, j0, k0) * (1.0f - t.x) + at(i1, j0, k0) * t.x;
    const T c10 = at(i0, j1, k0) * (1.0f - t.x) + at(i1, j1, k0) * t.x;
    const T c01 = at(i0, j0, k1) * (1.0f - t.x) + at(i1, j0, k1) * t.x;
    const T c11 = at(i0, j1, k1) * (1.0f - t.x) + at(i1, j1, k1) * t.x;
    const T c0 = c00 * (1.0f - t.y) + c10 * t.y;
    const T c1 = c01 * (1.0f - t.y) + c11 * t.y;
    return c0 * (1.0f - t.z) + c1 * t.z;
}

} // namespace

float UniformGrid::sample(std::string_view name, const glm::vec3 &p) const
{
    const auto values = scalars(name);
    const Cell c = cell(p);
    return trilinear<float>(values, static_cast<std::size_t>(m_dimensions.x), static_cast<std::size_t>(m_dimensions.y),
                            c.i0, c.j0, c.k0, c.i1, c.j1, c.k1, c.t);
}

glm::vec3 UniformGrid::sampleVector(std::string_view name, const glm::vec3 &p) const
{
    const auto values = vectors(name);
    const Cell c = cell(p);
    return trilinear<glm::vec3>(values, static_cast<std::size_t>(m_dimensions.x),
                                static_cast<std::size_t>(m_dimensions.y), c.i0, c.j0, c.k0, c.i1, c.j1, c.k1, c.t);
}

glm::vec3 UniformGrid::gradient(std::string_view name, const glm::vec3 &p) const
{
    glm::vec3 g(0.0f);
    for (int a = 0; a < 3; ++a)
    {
        if (m_dimensions[a] == 1 || m_spacing[a] == 0.0f)
            continue;
        glm::vec3 d(0.0f);
        d[a] = m_spacing[a];
        // One-sided at the boundary (sample() clamps)
        const ivf::BoundingBox box = bounds();
        const float lo = std::max(p[a] - d[a], box.min()[a]);
        const float hi = std::min(p[a] + d[a], box.max()[a]);
        if (hi <= lo)
            continue;
        glm::vec3 pl = p, ph = p;
        pl[a] = lo;
        ph[a] = hi;
        g[a] = (sample(name, ph) - sample(name, pl)) / (hi - lo);
    }
    return g;
}
