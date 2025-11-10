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

void ivf::Dodecahedron::setRadius(double radius)
{
    m_radius = radius;
    this->refresh();
}

void ivf::Dodecahedron::setSegments(int segments)
{
    m_segments = segments;
    this->refresh();
}

void ivf::Dodecahedron::setRings(int rings)
{
    m_rings = rings;
    this->refresh();
}

double ivf::Dodecahedron::radius() const
{
    return m_radius;
}

int ivf::Dodecahedron::segments() const
{
    return m_segments;
}

int ivf::Dodecahedron::rings() const
{
    return m_rings;
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
