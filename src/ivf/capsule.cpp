#include <ivf/capsule.h>

#include <generator/CapsuleMesh.hpp>

using namespace ivf;
using namespace generator;

Capsule::Capsule(double radius, double size, int slices, int segments, int rings, double start, double sweep)
    : m_radius(1.0), m_size(0.5), m_slices(32), m_segments(4), m_rings(8), m_start(0.0), m_sweep(gml::radians(360.0))
{
    this->doSetup();
    this->setName("Capsule");
}

std::shared_ptr<Capsule> Capsule::create(double radius, double size, int slices, int segments, int rings, double start,
                                         double sweep)
{
    return std::make_shared<Capsule>(radius, size, slices, segments, rings, start, sweep);
}

void ivf::Capsule::set(double radius, double size, int slices, int segments, int rings, double start, double sweep)
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

void Capsule::doSetup()
{
    CapsuleMesh capsule(m_radius, m_size, m_slices, m_segments, m_rings, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = capsule.vertices();
    AnyGenerator<Triangle> triangles = capsule.triangles();

    this->createFromGenerator(vertices, triangles);
}

void ivf::Capsule::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Radius", &m_radius, "Geometry");
    addProperty("Height", &m_size, "Geometry");
    addProperty("Slices", &m_slices, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Rings", &m_rings, "Geometry");
    addProperty("Start angle", &m_start, "Geometry");
    addProperty("Sweep angle", &m_sweep, "Geometry");
}

void ivf::Capsule::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
