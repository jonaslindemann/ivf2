#include <ivf/capped_tube.h>

#include <generator/CappedTubeMesh.hpp>

using namespace ivf;
using namespace generator;

CappedTube::CappedTube(double radius, double innerRadius, double size, int slices, int segments, int rings,
                       double start, double sweep)
    : m_radius(radius), m_innerRadius(innerRadius), m_size(size), m_slices(slices), m_segments(segments),
      m_rings(rings), m_start(start), m_sweep(sweep)
{
    this->doSetup();
    this->setName("CappedTube");
}

std::shared_ptr<CappedTube> CappedTube::create(double radius, double innerRadius, double size, int slices, int segments,
                                               int rings, double start, double sweep)
{
    return std::make_shared<CappedTube>(radius, innerRadius, size, slices, segments, rings, start, sweep);
}

void CappedTube::set(double radius, double innerRadius, double size, int slices, int segments, int rings, double start,
                     double sweep)
{
    m_radius = radius;
    m_innerRadius = innerRadius;
    m_size = size;
    m_slices = slices;
    m_segments = segments;
    m_rings = rings;
    m_start = start;
    m_sweep = sweep;
    this->refresh();
}

void CappedTube::setRadius(double radius)
{
    m_radius = radius;
    this->refresh();
}

void CappedTube::setInnerRadius(double innerRadius)
{
    m_innerRadius = innerRadius;
    this->refresh();
}

void CappedTube::setHeight(double size)
{
    m_size = size;
    this->refresh();
}

void CappedTube::setSlices(int slices)
{
    m_slices = slices;
    this->refresh();
}

void CappedTube::setSegments(int segments)
{
    m_segments = segments;
    this->refresh();
}

void CappedTube::setRings(int rings)
{
    m_rings = rings;
    this->refresh();
}

void CappedTube::setStartAngle(double start)
{
    m_start = start;
    this->refresh();
}

void CappedTube::setSweepAngle(double sweep)
{
    m_sweep = sweep;
    this->refresh();
}

double CappedTube::radius() const
{
    return m_radius;
}

double CappedTube::innerRadius() const
{
    return m_innerRadius;
}

double CappedTube::height() const
{
    return m_size;
}

int CappedTube::slices() const
{
    return m_slices;
}

int CappedTube::segments() const
{
    return m_segments;
}

int CappedTube::rings() const
{
    return m_rings;
}

double CappedTube::startAngle() const
{
    return m_start;
}

double CappedTube::sweepAngle() const
{
    return m_sweep;
}

void CappedTube::doSetup()
{
    CappedTubeMesh cappedCylinder(m_radius, m_innerRadius, m_size / 2.0, m_slices, m_segments, m_rings, m_start,
                                  m_sweep);

    AnyGenerator<MeshVertex> vertices = cappedCylinder.vertices();
    AnyGenerator<Triangle> triangles = cappedCylinder.triangles();

    this->createFromGenerator(vertices, triangles);
}

void ivf::CappedTube::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Outer radius", &m_radius, "Geometry");
    addProperty("Inner radius", &m_innerRadius, "Geometry");
    addProperty("Height", &m_size, "Geometry");
    addProperty("Slices", &m_slices, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Rings", &m_rings, "Geometry");
    addProperty("Start angle", &m_start, "Geometry");
    addProperty("Sweep angle", &m_sweep, "Geometry");
}

void ivf::CappedTube::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
