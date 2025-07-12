#include <ivf/cone.h>

#include <generator/ConeMesh.hpp>

using namespace ivf;
using namespace generator;

Cone::Cone(double radius, double size, int slices, int segments, double start, double sweep)
    : m_radius(radius), m_size(size), m_slices(slices), m_segments(segments), m_start(start), m_sweep(sweep)
{
    this->doSetup();
    this->setName("Cone");
}

std::shared_ptr<Cone> Cone::create(double radius, double size, int slices, int segments, double start, double sweep)
{
    return std::make_shared<Cone>(radius, size, slices, segments, start, sweep);
}

void ivf::Cone::set(double radius, double size, int slices, int segments, double start, double sweep)
{
    m_radius = radius;
    m_size = size;
    m_slices = slices;
    m_segments = segments;
    m_start = start;
    m_sweep = sweep;
    this->refresh();
}

void Cone::doSetup()
{
    ConeMesh cone(m_radius, m_size, m_slices, m_segments, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = cone.vertices();
    AnyGenerator<Triangle> triangles = cone.triangles();

    this->createFromGenerator(vertices, triangles);
}
