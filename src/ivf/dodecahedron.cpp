#include <ivf/dodecahedron.h>

#include <generator/DodecahedronMesh.hpp>

using namespace ivf;
using namespace generator;

Dodecahedron::Dodecahedron(double radius, int segments, int rings)
    : m_radius(radius), m_segments(segments), m_rings(rings)
{
    this->doSetup();
    this->setName("Dodecahedron");
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

void ivf::Dodecahedron::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Radius", &m_radius, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Rings", &m_rings, "Geometry");
}

void ivf::Dodecahedron::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
