#include <ivf/capped_cylinder.h>

#include <generator/CappedCylinderMesh.hpp>

using namespace ivf;
using namespace generator;

CappedCylinder::CappedCylinder(double radius, double size, int slices, int segments, int rings, double start,
                               double sweep)
    : m_radius(radius), m_size(size), m_slices(slices), m_segments(segments), m_rings(rings), m_start(start),
      m_sweep(sweep)
{
    this->doSetup();
    this->setName("CappedCylinder");
}

std::shared_ptr<CappedCylinder> CappedCylinder::create(double radius, double size, int slices, int segments, int rings,
                                                       double start, double sweep)
{
    return std::make_shared<CappedCylinder>(radius, size, slices, segments, rings, start, sweep);
}

void ivf::CappedCylinder::set(double radius, double size, int slices, int segments, int rings, double start,
                              double sweep)
{
    m_radius = radius;
    m_size = size;
    m_slices = slices;
    m_segments = segments;
    m_rings = rings;
    m_start = start;
    m_sweep = sweep;
    this->refresh();
}

void CappedCylinder::doSetup()
{
    CappedCylinderMesh cappedCylinder(m_radius, m_size / 2.0, m_slices, m_segments, m_rings, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = cappedCylinder.vertices();
    AnyGenerator<Triangle> triangles = cappedCylinder.triangles();

    this->createFromGenerator(vertices, triangles);

    // Set bounding box for the capped cylinder (axis-aligned along Y)
    double halfHeight = m_size * 0.5;
    setLocalBoundingBox(BoundingBox(glm::vec3(-m_radius, -halfHeight, -m_radius),
                                    glm::vec3(m_radius, halfHeight, m_radius)));
}

void ivf::CappedCylinder::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Radius", &m_radius, "Geometry");
    addProperty("Height", &m_size, "Geometry");
    addProperty("Slices", &m_slices, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Rings", &m_rings, "Geometry");
    addProperty("Start angle", &m_start, "Geometry");
}

void ivf::CappedCylinder::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
