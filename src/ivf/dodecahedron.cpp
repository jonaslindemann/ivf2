#include <ivf/dodecahedron.h>

#include <generator/DodecahedronMesh.hpp>

using namespace ivf;
using namespace generator;

Dodecahedron::Dodecahedron(double radius, int segments, int rings)
    :m_radius(radius),
     m_segments(segments),
     m_rings(rings)
{
    this->doSetup();
}

std::shared_ptr<Dodecahedron> Dodecahedron::create(double radius, int segments, int rings)
{
    return std::make_shared<Dodecahedron>(radius, segments, rings);
}

void ivf::Dodecahedron::set(double radius, int segments, int rings)
{
    m_radius = radius;
    m_segments = segments;
    m_rings = rings;
    this->refresh();
}

void Dodecahedron::doSetup()
{
    DodecahedronMesh dodecahedron(m_radius, m_segments, m_rings);

    AnyGenerator<MeshVertex> vertices = dodecahedron.vertices();
    AnyGenerator<Triangle> triangles = dodecahedron.triangles();

    this->createFromGenerator(vertices, triangles);
}


