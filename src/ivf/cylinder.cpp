#include <ivf/cylinder.h>

#include <generator/CylinderMesh.hpp>

using namespace ivf;
using namespace generator;

Cylinder::Cylinder(double radius, double size, int slices, int segments, double start, double sweep)
    : m_radius(radius), m_size(size), m_slices(slices), m_segments(segments), m_start(start), m_sweep(sweep)
{
    this->doSetup();
    this->setName("Cylinder");
}

std::shared_ptr<Cylinder> Cylinder::create(double radius, double size, int slices, int segments, double start,
                                           double sweep)
{
    return std::make_shared<Cylinder>(radius, size, slices, segments, start, sweep);
}

void ivf::Cylinder::set(double radius, double size, int slices, int segments, double start, double sweep)
{
    m_radius = radius;
    m_size = size;
    m_slices = slices;
    m_segments = segments;
    m_start = start;
    m_sweep = sweep;
    this->refresh();
}

void Cylinder::doSetup()
{
    CylinderMesh cappedCylinder(m_radius, m_size, m_slices, m_segments, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = cappedCylinder.vertices();
    AnyGenerator<Triangle> triangles = cappedCylinder.triangles();

    this->createFromGenerator(vertices, triangles);
}

void ivf::Cylinder::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Base radius", &m_radius, "Geometry");
    addProperty("Height", &m_size, "Geometry");
    addProperty("Slices", &m_slices, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Start angle", &m_start, "Geometry");
    addProperty("Sweep angle", &m_sweep, "Geometry");
}

void ivf::Cylinder::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
