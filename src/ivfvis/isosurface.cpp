#include <ivfvis/isosurface.h>

#include <ivfvis/scalar_coloring.h>

#include <ivf/extrusion_builder.h>
#include <ivf/mesh.h>

using namespace ivfvis;

namespace {

ivf::MeshData toMeshData(const SurfaceData &surface)
{
    ivf::MeshData data;
    data.positions = surface.positions;
    data.normals = surface.normals;
    data.texCoords.assign(surface.positions.size(), glm::vec2(0.0f));
    data.colors.assign(surface.positions.size(), glm::vec4(1.0f));
    data.indices = surface.indices;
    return data;
}

} // namespace

Isosurface::Isosurface(UniformGridPtr grid, std::string scalars, float isoValue)
    : m_grid(std::move(grid)), m_scalars(std::move(scalars)), m_isoValue(isoValue)
{
    setName("Isosurface");
    update();
}

std::shared_ptr<Isosurface> Isosurface::create(UniformGridPtr grid, std::string scalars, float isoValue)
{
    return std::make_shared<Isosurface>(std::move(grid), std::move(scalars), isoValue);
}

void Isosurface::setGrid(UniformGridPtr grid)
{
    m_grid = std::move(grid);
    update();
}

UniformGridPtr Isosurface::grid() const
{
    return m_grid;
}

void Isosurface::setScalars(const std::string &scalars)
{
    m_scalars = scalars;
    update();
}

const std::string &Isosurface::scalars() const
{
    return m_scalars;
}

void Isosurface::setIsoValue(float isoValue)
{
    m_isoValue = isoValue;
    update();
}

float Isosurface::isoValue() const
{
    return m_isoValue;
}

void Isosurface::setColorBy(const std::string &name, ColormapPtr colormap)
{
    m_colorBy = name;
    m_colormap = std::move(colormap);
    useVertexColors(*this, true);
    update();
}

void Isosurface::clearColorBy()
{
    m_colorBy.clear();
    useVertexColors(*this, false);
    update();
}

const std::string &Isosurface::colorBy() const
{
    return m_colorBy;
}

ColormapPtr Isosurface::colormap() const
{
    return m_colormap;
}

void Isosurface::update()
{
    SurfaceData surface;
    if (m_grid && !m_scalars.empty())
        surface = extractIsosurface(*m_grid, m_scalars, m_isoValue, m_colorBy);
    m_triangles = surface.triangleCount();
    m_values = std::move(surface.values);
    createFromMeshData(toMeshData(surface));
    if (surface.indices.empty())
        setLocalBoundingBox(ivf::BoundingBox());
    m_coloredWith = nullptr;
    recolor();
}

std::size_t Isosurface::triangleCount() const
{
    return m_triangles;
}

void Isosurface::recolor()
{
    if (m_colorBy.empty() || !m_colormap || meshes().empty())
        return;
    colorVertices(*mesh(), m_values, *m_colormap);
    m_coloredWith = m_colormap.get();
    m_coloredVersion = m_colormap->version();
}

void Isosurface::collect(ivf::RenderQueue &queue, const ivf::CollectState &state)
{
    // Follow changes of the colormap
    if (m_colormap && !m_colorBy.empty() &&
        (m_coloredWith != m_colormap.get() || m_coloredVersion != m_colormap->version()))
        recolor();
    MeshNode::collect(queue, state);
}
