#include <ivfvis/glyphs.h>

#include <ivfvis/scalar_coloring.h>

#include <ivf/extrusion_builder.h>
#include <ivf/logger.h>
#include <ivf/material.h>
#include <ivf/mesh.h>

#include <algorithm>
#include <cmath>

using namespace ivfvis;

namespace {

constexpr int Sides = 10;
constexpr float Pi = 3.14159265358979f;

/** Orthonormal u, w perpendicular to unit vector d (u, w, d right-handed) */
void basis(const glm::vec3 &d, glm::vec3 &u, glm::vec3 &w)
{
    const glm::vec3 helper = std::abs(d.x) < 0.9f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    u = glm::normalize(glm::cross(helper, d));
    w = glm::cross(d, u);
}

struct Builder {
    ivf::MeshData &data;
    std::vector<float> &magnitudes;
    float magnitude{0.0f};

    std::uint32_t vertex(const glm::vec3 &p, const glm::vec3 &n)
    {
        data.positions.push_back(p);
        data.normals.push_back(n);
        data.texCoords.emplace_back(0.0f);
        data.colors.emplace_back(1.0f);
        magnitudes.push_back(magnitude);
        return static_cast<std::uint32_t>(data.positions.size() - 1);
    }
    void triangle(std::uint32_t a, std::uint32_t b, std::uint32_t c)
    {
        data.indices.emplace_back(a, b, c);
    }

    /** Flat disc at @p c facing -d */
    void disc(const glm::vec3 &c, const glm::vec3 &d, const glm::vec3 &u, const glm::vec3 &w, float r)
    {
        const std::uint32_t centre = vertex(c, -d);
        std::uint32_t ring[Sides];
        for (int i = 0; i < Sides; ++i)
        {
            const float a = 2.0f * Pi * static_cast<float>(i) / Sides;
            ring[i] = vertex(c + r * (std::cos(a) * u + std::sin(a) * w), -d);
        }
        for (int i = 0; i < Sides; ++i)
            triangle(centre, ring[(i + 1) % Sides], ring[i]);
    }

    /** Cone from a ring of radius @p r at @p c to the tip at c + h * d */
    void cone(const glm::vec3 &c, const glm::vec3 &d, const glm::vec3 &u, const glm::vec3 &w, float r, float h)
    {
        std::uint32_t ring[Sides], tips[Sides];
        auto normal = [&](float a) {
            const glm::vec3 radial = std::cos(a) * u + std::sin(a) * w;
            return glm::normalize(radial * h + d * r);
        };
        for (int i = 0; i < Sides; ++i)
        {
            const float a = 2.0f * Pi * static_cast<float>(i) / Sides;
            ring[i] = vertex(c + r * (std::cos(a) * u + std::sin(a) * w), normal(a));
            tips[i] = vertex(c + h * d, normal(a + Pi / Sides));
        }
        for (int i = 0; i < Sides; ++i)
            triangle(ring[i], ring[(i + 1) % Sides], tips[i]);
    }

    /** Open cylinder from @p c, length @p h */
    void cylinder(const glm::vec3 &c, const glm::vec3 &d, const glm::vec3 &u, const glm::vec3 &w, float r, float h)
    {
        std::uint32_t a0[Sides], a1[Sides];
        for (int i = 0; i < Sides; ++i)
        {
            const float a = 2.0f * Pi * static_cast<float>(i) / Sides;
            const glm::vec3 radial = std::cos(a) * u + std::sin(a) * w;
            a0[i] = vertex(c + r * radial, radial);
            a1[i] = vertex(c + h * d + r * radial, radial);
        }
        for (int i = 0; i < Sides; ++i)
        {
            const int j = (i + 1) % Sides;
            triangle(a0[i], a0[j], a1[j]);
            triangle(a0[i], a1[j], a1[i]);
        }
    }
};

} // namespace

Glyphs::Glyphs(VectorFieldPtr field) : m_field(std::move(field))
{
    setName("Glyphs");
    rebuild();
}

std::shared_ptr<Glyphs> Glyphs::create(VectorFieldPtr field)
{
    return std::make_shared<Glyphs>(std::move(field));
}

void Glyphs::setField(VectorFieldPtr field)
{
    m_field = std::move(field);
    rebuild();
}

VectorFieldPtr Glyphs::field() const
{
    return m_field;
}

void Glyphs::setSampleGrid(const glm::ivec3 &counts)
{
    m_sampleGrid = glm::max(counts, glm::ivec3(1));
    m_ownPositions = false;
    rebuild();
}

void Glyphs::setPositions(std::vector<glm::vec3> positions)
{
    m_positions = std::move(positions);
    m_ownPositions = true;
    rebuild();
}

const std::vector<glm::vec3> &Glyphs::positions() const
{
    return m_positions;
}

void Glyphs::setShape(GlyphShape shape)
{
    m_shape = shape;
    rebuild();
}

GlyphShape Glyphs::shape() const
{
    return m_shape;
}

void Glyphs::setScaling(GlyphScaling scaling)
{
    m_scaling = scaling;
    rebuild();
}

GlyphScaling Glyphs::scaling() const
{
    return m_scaling;
}

void Glyphs::setScale(float scale)
{
    m_scale = std::max(scale, 0.0f);
    rebuild();
}

float Glyphs::scale() const
{
    return m_scale;
}

void Glyphs::setAnchor(GlyphAnchor anchor)
{
    m_anchor = anchor;
    rebuild();
}

GlyphAnchor Glyphs::anchor() const
{
    return m_anchor;
}

void Glyphs::setThickness(float thickness)
{
    m_thickness = std::max(thickness, 0.0f);
    rebuild();
}

float Glyphs::thickness() const
{
    return m_thickness;
}

void Glyphs::setMinMagnitude(float magnitude)
{
    m_minMagnitude = std::max(magnitude, 0.0f);
    rebuild();
}

void Glyphs::setColormap(ColormapPtr colormap)
{
    m_colormap = std::move(colormap);
    m_coloredWith = nullptr;
    recolor();
}

ColormapPtr Glyphs::colormap() const
{
    return m_colormap;
}

ValueRange Glyphs::magnitudeRange() const
{
    ValueRange range;
    for (const glm::vec3 &v : m_glyphVector)
        range.add(glm::length(v));
    return range;
}

std::size_t Glyphs::glyphCount() const
{
    return m_glyphSample.size();
}

int Glyphs::glyphIndex(int triangle) const
{
    if (triangle < 0 || m_trianglesPerGlyph <= 0)
        return -1;
    const int index = triangle / m_trianglesPerGlyph;
    return index < static_cast<int>(m_glyphSample.size()) ? index : -1;
}

glm::vec3 Glyphs::glyphPosition(std::size_t index) const
{
    return m_positions[m_glyphSample[index]];
}

glm::vec3 Glyphs::glyphVector(std::size_t index) const
{
    return m_glyphVector[index];
}

void Glyphs::update()
{
    rebuild();
}

void Glyphs::rebuild()
{
    // Sample positions: a grid over the field's bounds unless given
    if (!m_ownPositions)
    {
        m_positions.clear();
        const ivf::BoundingBox box = m_field ? m_field->bounds() : ivf::BoundingBox();
        if (m_field && !box.isValid())
            ivf::logWarning("The vector field has no bounds: use setPositions() for its glyphs", "Glyphs");
        if (box.isValid())
            for (int k = 0; k < m_sampleGrid.z; ++k)
                for (int j = 0; j < m_sampleGrid.y; ++j)
                    for (int i = 0; i < m_sampleGrid.x; ++i)
                    {
                        const glm::ivec3 ijk(i, j, k);
                        glm::vec3 t(0.5f);
                        for (int a = 0; a < 3; ++a)
                            if (m_sampleGrid[a] > 1)
                                t[a] = static_cast<float>(ijk[a]) / static_cast<float>(m_sampleGrid[a] - 1);
                        m_positions.push_back(box.min() + t * box.size());
                    }
    }

    m_glyphSample.clear();
    m_glyphVector.clear();
    m_vertexMagnitude.clear();
    ivf::MeshData data;
    std::vector<glm::vec3> lines;
    Builder builder{data, m_vertexMagnitude};

    for (std::size_t s = 0; m_field && s < m_positions.size(); ++s)
    {
        const glm::vec3 p = m_positions[s];
        const glm::vec3 v = m_field->sample(p);
        const float magnitude = glm::length(v);
        if (!(magnitude > 0.0f) || magnitude < m_minMagnitude || !std::isfinite(magnitude))
            continue;
        const float length = m_scaling == GlyphScaling::Magnitude ? m_scale * magnitude : m_scale;
        if (!(length > 0.0f))
            continue;
        const glm::vec3 d = v / magnitude;
        const glm::vec3 base = m_anchor == GlyphAnchor::Tail ? p : p - 0.5f * length * d;
        m_glyphSample.push_back(s);
        m_glyphVector.push_back(v);
        builder.magnitude = magnitude;

        if (m_shape == GlyphShape::Line)
        {
            lines.push_back(base);
            lines.push_back(base + length * d);
            m_vertexMagnitude.push_back(magnitude);
            m_vertexMagnitude.push_back(magnitude);
            continue;
        }
        glm::vec3 u, w;
        basis(d, u, w);
        if (m_shape == GlyphShape::Arrow)
        {
            const float shaft = 0.72f * length;
            const float rs = 0.05f * length * m_thickness, rh = 0.12f * length * m_thickness;
            builder.disc(base, d, u, w, rs);
            builder.cylinder(base, d, u, w, rs, shaft);
            builder.disc(base + shaft * d, d, u, w, rh);
            builder.cone(base + shaft * d, d, u, w, rh, length - shaft);
        }
        else
        {
            const float r = 0.25f * length * m_thickness;
            builder.disc(base, d, u, w, r);
            builder.cone(base, d, u, w, r, length);
        }
    }

    m_trianglesPerGlyph = m_shape == GlyphShape::Arrow ? 5 * Sides : (m_shape == GlyphShape::Cone ? 2 * Sides : 0);

    if (m_shape == GlyphShape::Line)
    {
        clear();
        if (!lines.empty())
        {
            newMesh(static_cast<int>(lines.size()), 0, GL_LINES);
            auto mesh = currentMesh();
            mesh->setGenerateNormals(false);
            mesh->begin(GL_LINES);
            for (const glm::vec3 &p : lines)
            {
                mesh->color4f(1.0f, 1.0f, 1.0f, 1.0f);
                mesh->normal3f(0.0f, 0.0f, 1.0f);
                mesh->vertex3f(p);
            }
            mesh->end();
            auto lineMaterial = ivf::Material::create();
            lineMaterial->setUseLighting(false);
            lineMaterial->setUseVertexColor(true);
            mesh->setMaterial(lineMaterial);
        }
        updateBoundingBox();
    }
    else
    {
        createFromMeshData(data);
    }
    if (m_glyphSample.empty())
        setLocalBoundingBox(ivf::BoundingBox());

    m_coloredWith = nullptr;
    recolor();
}

void Glyphs::recolor()
{
    const bool lines = m_shape == GlyphShape::Line;
    if (!lines)
        useVertexColors(*this, m_colormap != nullptr);
    if (meshes().empty())
        return;
    if (m_colormap)
    {
        colorVertices(*mesh(), m_vertexMagnitude, *m_colormap);
        m_coloredWith = m_colormap.get();
        m_coloredVersion = m_colormap->version();
    }
}

void Glyphs::collect(ivf::RenderQueue &queue, const ivf::CollectState &state)
{
    if (m_colormap && (m_coloredWith != m_colormap.get() || m_coloredVersion != m_colormap->version()))
        recolor();
    MeshNode::collect(queue, state);
}
