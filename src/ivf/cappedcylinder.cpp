#include <ivf/cappedcylinder.h>

#include <generator/CappedCylinderMesh.hpp>

using namespace ivf;
using namespace generator;

CappedCylinder::CappedCylinder(double radius, double size, int slices, int segments, int rings, double start, double sweep)
    :m_radius(radius),
     m_size(size),
     m_slices(slices),
     m_segments(segments),
     m_rings(rings),
     m_start(start),
     m_sweep(sweep)
{
    this->doSetup();
}

std::shared_ptr<CappedCylinder> CappedCylinder::create(double radius, double size, int slices, int segments, int rings, double start, double sweep)
{
    return std::make_shared<CappedCylinder>(radius, size, slices, segments, rings, start, sweep);
}

void ivf::CappedCylinder::set(double radius, double size, int slices, int segments, int rings, double start, double sweep)
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
    CappedCylinderMesh cappedCylinder(m_radius, m_size, m_slices, m_segments, m_rings, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = cappedCylinder.vertices();
    AnyGenerator<Triangle> triangles = cappedCylinder.triangles();

    this->createFromGenerator(vertices, triangles);
}


