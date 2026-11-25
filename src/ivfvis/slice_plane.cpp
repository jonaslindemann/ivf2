#include <ivfvis/slice_plane.h>

#include <ivfvis/scalar_coloring.h>

#include <ivf/extrusion_builder.h>
#include <ivf/material.h>
#include <ivf/mesh.h>

#include <algorithm>

using namespace ivfvis;

SlicePlane::SlicePlane(UniformGridPtr grid, std::string scalars, ColormapPtr colormap)
    : m_grid(std::move(grid)), m_scalars(std::move(scalars)), m_colormap(std::move(colormap))
{
    setName("SlicePlane");

    // Unlit: the colours match the colour bar
    auto material = ivf::Material::create();
    material->setUseLighting(false);
    material->setUseVertexColor(true);
    setMaterial(material);

    m_lineMaterial = ivf::Material::create();
    m_lineMaterial->setUseLighting(false);
    m_lineMaterial->setDiffuseColor(glm::vec4(m_contourColor, 1.0f));

    if (m_grid)
        m_origin = m_grid->bounds().center();
    rebuild();
}

std::shared_ptr<SlicePlane> SlicePlane::create(UniformGridPtr grid, std::string scalars, ColormapPtr colormap)
{
    return std::make_shared<SlicePlane>(std::move(grid), std::move(scalars), std::move(colormap));
}

void SlicePlane::setGrid(UniformGridPtr grid)
{
    m_grid = std::move(grid);
    rebuild();
}

UniformGridPtr SlicePlane::grid() const
{
    return m_grid;
}

void SlicePlane::setScalars(const std::string &scalars)
{
    m_scalars = scalars;
    rebuild();
}

const std::string &SlicePlane::scalars() const
{
    return m_scalars;
}

void SlicePlane::setColormap(ColormapPtr colormap)
{
    m_colormap = std::move(colormap);
    m_builtWith = nullptr; // recoloured when next drawn
}

ColormapPtr SlicePlane::colormap() const
{
    return m_colormap;
}

void SlicePlane::setPlane(const glm::vec3 &origin, const glm::vec3 &normal)
{
    m_origin = origin;
    m_normal = normal;
    rebuild();
}

void SlicePlane::setAxis(SliceAxis axis, float position)
{
    glm::vec3 origin = m_grid ? m_grid->bounds().center() : glm::vec3(0.0f);
    glm::vec3 normal(0.0f);
    const auto a = static_cast<int>(axis);
    origin[a] = position;
    normal[a] = 1.0f;
    setPlane(origin, normal);
}

glm::vec3 SlicePlane::planeOrigin() const
{
    return m_origin;
}

glm::vec3 SlicePlane::planeNormal() const
{
    return m_normal;
}

void SlicePlane::setResolution(int resolution)
{
    m_resolution = std::max(resolution, 1);
    rebuild();
}

int SlicePlane::resolution() const
{
    return m_resolution;
}

void SlicePlane::setContours(int count)
{
    m_contourCount = std::max(count, 0);
    m_contourValues.clear();
    m_builtWith = nullptr;
}

void SlicePlane::setContourValues(std::vector<float> values)
{
    m_contourValues = std::move(values);
    m_contourCount = 0;
    m_builtWith = nullptr;
}

std::vector<float> SlicePlane::contourValues() const
{
    if (!m_contourValues.empty() || m_contourCount == 0 || !m_colormap)
        return m_contourValues;
    // Evenly inside the colormap's range (in its scale), excluding the ends
    std::vector<float> values;
    for (int i = 1; i <= m_contourCount; ++i)
        values.push_back(m_colormap->value(static_cast<float>(i) / static_cast<float>(m_contourCount + 1)));
    return values;
}

void SlicePlane::setContourColor(const glm::vec3 &color)
{
    m_contourColor = color;
    m_lineMaterial->setDiffuseColor(glm::vec4(color, 1.0f));
}

void SlicePlane::update()
{
    rebuild();
}

void SlicePlane::rebuild()
{
    m_surface = SurfaceData();
    if (m_grid && !m_scalars.empty())
        m_surface = extractSlice(*m_grid, m_scalars, m_origin, m_normal, m_resolution);
    m_builtWith = nullptr;
    recolor();
}

void SlicePlane::recolor()
{
    // The slice (mesh 0)
    ivf::MeshData data;
    data.positions = m_surface.positions;
    data.normals = m_surface.normals;
    data.texCoords.assign(m_surface.positions.size(), glm::vec2(0.0f));
    data.colors.assign(m_surface.positions.size(), glm::vec4(1.0f));
    data.indices = m_surface.indices;
    createFromMeshData(data);
    if (m_surface.indices.empty())
    {
        setLocalBoundingBox(ivf::BoundingBox());
        return;
    }
    if (m_colormap)
        colorVertices(*mesh(), m_surface.values, *m_colormap);

    // Contour lines (mesh 1), lifted off both sides of the slice so they are not hidden by it
    const std::vector<float> levels = contourValues();
    const std::vector<glm::vec3> lines = contourLines(m_surface, levels);
    if (!lines.empty())
    {
        const ivf::BoundingBox box = m_grid->bounds();
        const glm::vec3 lift = m_surface.normals.front() * (0.002f * glm::length(box.size()));
        newMesh(static_cast<int>(lines.size() * 2), 0, GL_LINES);
        auto lineMesh = currentMesh();
        lineMesh->setGenerateNormals(false);
        lineMesh->begin(GL_LINES);
        for (const glm::vec3 &side : {lift, -lift})
            for (const glm::vec3 &p : lines)
            {
                lineMesh->normal3f(m_surface.normals.front());
                lineMesh->vertex3f(p + side);
            }
        lineMesh->end();
        lineMesh->setMaterial(m_lineMaterial);
        updateBoundingBox();
    }

    m_builtWith = m_colormap.get();
    m_builtVersion = m_colormap ? m_colormap->version() : 0;
}

void SlicePlane::collect(ivf::RenderQueue &queue, const ivf::CollectState &state)
{
    // Follow changes of the colormap (colours and contour levels)
    if (m_colormap && (m_builtWith != m_colormap.get() || m_builtVersion != m_colormap->version()))
        recolor();
    MeshNode::collect(queue, state);
}
