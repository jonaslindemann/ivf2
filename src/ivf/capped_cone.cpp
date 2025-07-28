#include <ivf/capped_cone.h>

#include <generator/CappedConeMesh.hpp>

using namespace ivf;
using namespace generator;

CappedCone::CappedCone(double radius, double size, int slices, int segments, int rings, double start, double sweep)
    : m_radius(radius), m_size(size), m_slices(slices), m_segments(segments), m_rings(rings), m_start(start),
      m_sweep(sweep)
{
    this->doSetup();
    this->setName("CappedCone");
}

std::shared_ptr<CappedCone> CappedCone::create(double radius, double size, int slices, int segments, int rings,
                                               double start, double sweep)
{
    return std::make_shared<CappedCone>(radius, size, slices, segments, rings, start, sweep);
}

void ivf::CappedCone::set(double radius, double size, int slices, int segments, int rings, double start, double sweep)
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

void CappedCone::doSetup()
{
    CappedConeMesh cappedCone(m_radius, m_size / 2.0, m_slices, m_segments, m_rings, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = cappedCone.vertices();
    AnyGenerator<Triangle> triangles = cappedCone.triangles();

    this->createFromGenerator(vertices, triangles);
}

void ivf::CappedCone::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Base radius", &m_radius, "Geometry");
    addProperty("Height", &m_size, "Geometry");
    addProperty("Slices", &m_slices, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Rings", &m_rings, "Geometry");
    addProperty("Start angle", &m_start, "Geometry");
    addProperty("Sweep angle", &m_sweep, "Geometry");
}

void ivf::CappedCone::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
