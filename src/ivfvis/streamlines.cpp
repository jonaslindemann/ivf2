#include <ivfvis/streamlines.h>

#include <ivfvis/scalar_coloring.h>

#include <ivf/extrusion_builder.h>
#include <ivf/material.h>
#include <ivf/mesh.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <random>

using namespace ivfvis;

float Streamline::length() const
{
    float sum = 0.0f;
    for (std::size_t i = 1; i < points.size(); ++i)
        sum += glm::length(points[i] - points[i - 1]);
    return sum;
}

namespace {

/** Trace one way from @p seed (not included); @p sign: +1 along, -1 against the field */
void traceOneWay(const VectorField &field, const glm::vec3 &seed, float sign, float h, const StreamlineOptions &o,
                 std::vector<glm::vec3> &points, std::vector<float> &speeds)
{
    auto direction = [&](const glm::vec3 &p) -> std::optional<glm::vec3> {
        if (!field.contains(p))
            return std::nullopt;
        const glm::vec3 v = field.sample(p);
        const float speed = glm::length(v);
        if (!(speed >= o.minSpeed) || !std::isfinite(speed) || speed == 0.0f)
            return std::nullopt;
        return sign * v / speed;
    };

    glm::vec3 p = seed;
    float length = 0.0f;
    for (int step = 0; step < o.maxSteps; ++step)
    {
        const auto k1 = direction(p);
        if (!k1)
            break;
        const auto k2 = direction(p + 0.5f * h * *k1);
        if (!k2)
            break;
        const auto k3 = direction(p + 0.5f * h * *k2);
        if (!k3)
            break;
        const auto k4 = direction(p + h * *k3);
        if (!k4)
            break;
        const glm::vec3 next = p + h / 6.0f * (*k1 + 2.0f * *k2 + 2.0f * *k3 + *k4);
        if (!field.contains(next))
            break;
        const float speed = glm::length(field.sample(next));
        if (!(speed >= o.minSpeed))
            break;
        length += glm::length(next - p);
        if (o.maxLength > 0.0f && length > o.maxLength)
            break;
        points.push_back(next);
        speeds.push_back(speed);
        p = next;
    }
}

} // namespace

Streamline ivfvis::traceStreamline(const VectorField &field, const glm::vec3 &seed, const StreamlineOptions &options)
{
    Streamline line;
    if (!field.contains(seed))
        return line;
    const float seedSpeed = glm::length(field.sample(seed));
    if (!(seedSpeed >= options.minSpeed) || seedSpeed == 0.0f)
        return line;

    float h = options.step;
    if (!(h > 0.0f))
    {
        const ivf::BoundingBox box = field.bounds();
        h = box.isValid() && glm::length(box.size()) > 0.0f ? glm::length(box.size()) / 200.0f : 0.01f;
    }

    if (options.direction != TraceDirection::Forward)
    {
        traceOneWay(field, seed, -1.0f, h, options, line.points, line.speeds);
        std::reverse(line.points.begin(), line.points.end());
        std::reverse(line.speeds.begin(), line.speeds.end());
    }
    line.points.push_back(seed);
    line.speeds.push_back(seedSpeed);
    if (options.direction != TraceDirection::Backward)
        traceOneWay(field, seed, 1.0f, h, options, line.points, line.speeds);
    return line;
}

std::vector<Streamline> ivfvis::traceStreamlines(const VectorField &field, std::span<const glm::vec3> seeds,
                                                 const StreamlineOptions &options)
{
    std::vector<Streamline> lines;
    for (const glm::vec3 &seed : seeds)
    {
        Streamline line = traceStreamline(field, seed, options);
        if (line.points.size() >= 2)
            lines.push_back(std::move(line));
    }
    return lines;
}

std::vector<glm::vec3> ivfvis::seedLine(const glm::vec3 &a, const glm::vec3 &b, int count)
{
    std::vector<glm::vec3> seeds;
    for (int i = 0; i < count; ++i)
        seeds.push_back(count > 1 ? glm::mix(a, b, static_cast<float>(i) / static_cast<float>(count - 1))
                                  : 0.5f * (a + b));
    return seeds;
}

std::vector<glm::vec3> ivfvis::seedPlane(const glm::vec3 &origin, const glm::vec3 &u, const glm::vec3 &v, int nu,
                                         int nv)
{
    std::vector<glm::vec3> seeds;
    for (int j = 0; j < nv; ++j)
        for (int i = 0; i < nu; ++i)
        {
            const float s = nu > 1 ? static_cast<float>(i) / static_cast<float>(nu - 1) : 0.5f;
            const float t = nv > 1 ? static_cast<float>(j) / static_cast<float>(nv - 1) : 0.5f;
            seeds.push_back(origin + s * u + t * v);
        }
    return seeds;
}

std::vector<glm::vec3> ivfvis::seedCircle(const glm::vec3 &center, const glm::vec3 &normal, float radius, int count)
{
    std::vector<glm::vec3> seeds;
    if (glm::length(normal) == 0.0f)
        return seeds;
    const glm::vec3 n = glm::normalize(normal);
    const glm::vec3 helper = std::abs(n.x) < 0.9f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    const glm::vec3 u = glm::normalize(glm::cross(helper, n));
    const glm::vec3 w = glm::cross(n, u);
    for (int i = 0; i < count; ++i)
    {
        const float a = 2.0f * 3.14159265358979f * static_cast<float>(i) / static_cast<float>(std::max(count, 1));
        seeds.push_back(center + radius * (std::cos(a) * u + std::sin(a) * w));
    }
    return seeds;
}

std::vector<glm::vec3> ivfvis::seedRandom(const ivf::BoundingBox &bounds, int count, unsigned seed)
{
    std::vector<glm::vec3> seeds;
    if (!bounds.isValid())
        return seeds;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    for (int i = 0; i < count; ++i)
    {
        const glm::vec3 t(unit(rng), unit(rng), unit(rng));
        seeds.push_back(bounds.min() + t * bounds.size());
    }
    return seeds;
}

// ---------------------------------------------------------------------------------------
// Streamlines node
// ---------------------------------------------------------------------------------------

Streamlines::Streamlines(VectorFieldPtr field, std::vector<glm::vec3> seeds)
    : m_field(std::move(field)), m_seeds(std::move(seeds))
{
    setName("Streamlines");
    m_lineMaterial = ivf::Material::create();
    m_lineMaterial->setUseLighting(false);
    m_lineMaterial->setUseVertexColor(true);
    rebuild();
}

std::shared_ptr<Streamlines> Streamlines::create(VectorFieldPtr field, std::vector<glm::vec3> seeds)
{
    return std::make_shared<Streamlines>(std::move(field), std::move(seeds));
}

void Streamlines::setField(VectorFieldPtr field)
{
    m_field = std::move(field);
    rebuild();
}

VectorFieldPtr Streamlines::field() const
{
    return m_field;
}

void Streamlines::setSeeds(std::vector<glm::vec3> seeds)
{
    m_seeds = std::move(seeds);
    rebuild();
}

const std::vector<glm::vec3> &Streamlines::seeds() const
{
    return m_seeds;
}

void Streamlines::setOptions(const StreamlineOptions &options)
{
    m_options = options;
    rebuild();
}

const StreamlineOptions &Streamlines::options() const
{
    return m_options;
}

void Streamlines::setTubes(bool tubes)
{
    m_tubes = tubes;
    rebuild();
}

bool Streamlines::tubes() const
{
    return m_tubes;
}

void Streamlines::setTubeRadius(float radius)
{
    m_tubeRadius = std::max(radius, 0.0f);
    if (m_tubes)
        rebuild();
}

float Streamlines::tubeRadius() const
{
    return m_tubeRadius;
}

void Streamlines::setTubeSides(int sides)
{
    m_tubeSides = std::clamp(sides, 3, 64);
    if (m_tubes)
        rebuild();
}

void Streamlines::setColormap(ColormapPtr colormap)
{
    m_colormap = std::move(colormap);
    m_coloredWith = nullptr;
    rebuild();
}

ColormapPtr Streamlines::colormap() const
{
    return m_colormap;
}

void Streamlines::setColor(const glm::vec3 &color)
{
    m_color = color;
    if (!m_tubes && !m_colormap)
        rebuild();
}

const std::vector<Streamline> &Streamlines::lines() const
{
    return m_lines;
}

ValueRange Streamlines::speedRange() const
{
    ValueRange range;
    for (const Streamline &line : m_lines)
        for (float s : line.speeds)
            range.add(s);
    return range;
}

void Streamlines::update()
{
    rebuild();
}

void Streamlines::rebuild()
{
    m_lines = m_field ? traceStreamlines(*m_field, m_seeds, m_options) : std::vector<Streamline>();
    m_vertexSpeed.clear();

    if (m_tubes)
    {
        // Tubes around each line, with frames transported along it
        ivf::MeshData data;
        const int sides = m_tubeSides;
        for (const Streamline &line : m_lines)
        {
            std::vector<glm::vec3> points;
            std::vector<float> speeds;
            for (std::size_t i = 0; i < line.points.size(); ++i)
                if (points.empty() || glm::length(line.points[i] - points.back()) > 1e-7f)
                {
                    points.push_back(line.points[i]);
                    speeds.push_back(line.speeds[i]);
                }
            const std::size_t n = points.size();
            if (n < 2)
                continue;
            const auto first = static_cast<std::uint32_t>(data.positions.size());
            glm::vec3 normal(0.0f);
            for (std::size_t i = 0; i < n; ++i)
            {
                const glm::vec3 a = points[i == 0 ? 0 : i - 1], b = points[i + 1 < n ? i + 1 : n - 1];
                const glm::vec3 t = glm::normalize(b - a);
                if (i == 0)
                {
                    const glm::vec3 helper = std::abs(t.x) < 0.9f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
                    normal = glm::normalize(glm::cross(helper, t));
                }
                else
                {
                    const glm::vec3 projected = normal - glm::dot(normal, t) * t;
                    if (glm::length(projected) > 1e-6f)
                        normal = glm::normalize(projected);
                }
                const glm::vec3 binormal = glm::cross(t, normal);
                for (int s = 0; s < sides; ++s)
                {
                    const float angle = 2.0f * 3.14159265358979f * static_cast<float>(s) / static_cast<float>(sides);
                    const glm::vec3 radial = std::cos(angle) * normal + std::sin(angle) * binormal;
                    data.positions.push_back(points[i] + m_tubeRadius * radial);
                    data.normals.push_back(radial);
                    data.texCoords.emplace_back(0.0f);
                    data.colors.emplace_back(1.0f);
                    m_vertexSpeed.push_back(speeds[i]);
                }
            }
            for (std::size_t i = 0; i + 1 < n; ++i)
                for (int s = 0; s < sides; ++s)
                {
                    const auto ring = first + static_cast<std::uint32_t>(i * static_cast<std::size_t>(sides));
                    const auto next = ring + static_cast<std::uint32_t>(sides);
                    const auto s0 = static_cast<std::uint32_t>(s), s1 = static_cast<std::uint32_t>((s + 1) % sides);
                    data.indices.emplace_back(ring + s0, ring + s1, next + s1);
                    data.indices.emplace_back(ring + s0, next + s1, next + s0);
                }
        }
        createFromMeshData(data);
        useVertexColors(*this, m_colormap != nullptr);
    }
    else
    {
        clear();
        std::size_t segments = 0;
        for (const Streamline &line : m_lines)
            segments += line.points.size() - 1;
        if (segments > 0)
        {
            newMesh(static_cast<int>(segments * 2), 0, GL_LINES);
            auto mesh = currentMesh();
            mesh->setGenerateNormals(false);
            mesh->begin(GL_LINES);
            for (const Streamline &line : m_lines)
                for (std::size_t i = 0; i + 1 < line.points.size(); ++i)
                    for (std::size_t k : {i, i + 1})
                    {
                        mesh->color4f(m_color.r, m_color.g, m_color.b, 1.0f);
                        mesh->normal3f(0.0f, 0.0f, 1.0f);
                        mesh->vertex3f(line.points[k]);
                        m_vertexSpeed.push_back(line.speeds[k]);
                    }
            mesh->end();
            mesh->setMaterial(m_lineMaterial);
        }
        updateBoundingBox();
    }
    if (m_lines.empty())
        setLocalBoundingBox(ivf::BoundingBox());

    m_coloredWith = nullptr;
    if (m_colormap && !meshes().empty())
    {
        colorVertices(*mesh(), m_vertexSpeed, *m_colormap);
        m_coloredWith = m_colormap.get();
        m_coloredVersion = m_colormap->version();
    }
}

void Streamlines::collect(ivf::RenderQueue &queue, const ivf::CollectState &state)
{
    if (m_colormap && !meshes().empty() &&
        (m_coloredWith != m_colormap.get() || m_coloredVersion != m_colormap->version()))
    {
        colorVertices(*mesh(), m_vertexSpeed, *m_colormap);
        m_coloredWith = m_colormap.get();
        m_coloredVersion = m_colormap->version();
    }
    MeshNode::collect(queue, state);
}
