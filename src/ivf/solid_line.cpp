#include <ivf/solid_line.h>

#include <generator/SphereMesh.hpp>

using namespace ivf;
using namespace generator;

SolidLine::SolidLine(glm::vec3 p0, glm::vec3 p1, double radius) : m_radius(radius), m_p0(p0), m_p1(p1)
{
    this->doSetup();
}

std::shared_ptr<SolidLine> SolidLine::create(glm::vec3 p0, glm::vec3 p1, double radius)
{
    return std::make_shared<SolidLine>(p0, p1, radius);
}

void SolidLine::doSetup()
{
    gml::dvec3 p0(m_p0.x, m_p0.y, m_p0.z);
    gml::dvec3 p1(m_p1.x, m_p1.y, m_p1.z);
    gml::dvec3 n(1.0, 0.0, 0.0);

    LinePath line(p0, p1, n, 8);
    CircleShape circle(m_radius, 32);

    ExtrudeMesh<CircleShape, LinePath> extrudeMesh(circle, line);

    AnyGenerator<MeshVertex> vertices = extrudeMesh.vertices();
    AnyGenerator<Triangle> triangles = extrudeMesh.triangles();

    this->createFromGenerator(vertices, triangles);
}

void SolidLine::setRadius(double radius)
{
    m_radius = radius;
    this->refresh();
}

void ivf::SolidLine::setP0(glm::vec3 p)
{
    m_p0 = p;
    this->refresh();
}

void ivf::SolidLine::setP1(glm::vec3 p)
{
    m_p1 = p;
    this->refresh();
}

void ivf::SolidLine::setPoints(glm::vec3 p0, glm::vec3 p1)
{
    m_p0 = p0;
    m_p1 = p1;
    this->refresh();
}

void SolidLine::set(glm::vec3 p0, glm::vec3 p1, double radius)
{
    m_radius = radius;
    m_p0 = p0;
    m_p1 = p1;
    this->refresh();
}
