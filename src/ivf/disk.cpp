#include <ivf/disk.h>

#include <generator/DiskMesh.hpp>

using namespace ivf;
using namespace generator;

Disk::Disk(double radius, double innerRadius, int slices, int rings, double start, double sweep)
    :m_radius(radius),
     m_innerRadius(innerRadius),
     m_slices(slices),
     m_rings(rings),
     m_start(start),
     m_sweep(sweep)
{
    this->doSetup();
}

std::shared_ptr<Disk> Disk::create(double radius, double innerRadius, int slices, int rings, double start, double sweep)
{
    return std::make_shared<Disk>(radius, innerRadius, slices, rings, start, sweep);
}

void Disk::set(double radius, double innerRadius, int slices, int rings, double start, double sweep)
{
    m_radius = radius;
    m_innerRadius = innerRadius;
    m_slices = slices;
    m_rings = rings;
    m_start = start;
    m_sweep = sweep;
    this->refresh();
}

void Disk::doSetup()
{
    DiskMesh diskMesh(m_radius, m_innerRadius, m_slices, m_rings, m_start, m_sweep);

    AnyGenerator<MeshVertex> vertices = diskMesh.vertices();
    AnyGenerator<Triangle> triangles = diskMesh.triangles();

    this->createFromGenerator(vertices, triangles);
}


