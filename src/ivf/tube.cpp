#include <ivf/tube.h>

#include <generator/TubeMesh.hpp>

using namespace ivf;
using namespace generator;

Tube::Tube(double radius, double innerRadius, double size, int slices, int segments, double start, double sweep)
    : m_radius(radius), m_innerRadius(innerRadius), m_size(size), m_slices(slices), m_segments(segments),
      m_start(start), m_sweep(sweep)
{
    this->doSetup();
    this->setName("Tube");
}

std::shared_ptr<Tube> Tube::create(double radius, double innerRadius, double size, int slices, int segments,
                                   double start, double sweep)
{
    return std::make_shared<Tube>(radius, innerRadius, size, slices, segments, start, sweep);
}

void Tube::set(double radius, double innerRadius, double size, int slices, int segments, double start, double sweep)
{
    m_radius = radius;
    m_innerRadius = innerRadius;
    m_size = size;
    m_slices = slices;
    m_segments = segments;
    m_start = start;
    m_sweep = sweep;
    this->refresh();
}

void Tube::doSetup()
{
    TubeMesh tube(m_radius, m_innerRadius, m_size, m_slices, m_segments, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = tube.vertices();
    AnyGenerator<Triangle> triangles = tube.triangles();

    this->createFromGenerator(vertices, triangles);
}

void ivf::Tube::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Outer Radius", &m_radius, "Geometry");
    addProperty("Inner Radius", &m_innerRadius, "Geometry");
    addProperty("Size", &m_size, "Geometry");
    addProperty("Slices", &m_slices, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Start Angle", &m_start, "Geometry");
    addProperty("Sweep Angle", &m_sweep, "Geometry");
}

void ivf::Tube::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
