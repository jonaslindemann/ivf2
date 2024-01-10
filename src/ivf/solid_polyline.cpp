#include <ivf/solid_polyline.h>

#include <generator/SphereMesh.hpp>

using namespace ivf;
using namespace generator;

SolidPolyLine::SolidPolyLine(double radius) : m_radius(radius)
{
    this->doSetup();
}

std::shared_ptr<SolidPolyLine> SolidPolyLine::create(double radius)
{
    return std::make_shared<SolidPolyLine>(radius);
}

void SolidPolyLine::doSetup()
{
    gml::dvec3 n(1.0, 0.0, 0.0);

    //LinePath line(p0, p1, n, 8);
    CircleShape circle(m_radius, 32);

    ExtrudeMesh<CircleShape, PolyLinePath> extrudeMesh(circle, m_polyPath);

    AnyGenerator<MeshVertex> vertices = extrudeMesh.vertices();
    AnyGenerator<Triangle> triangles = extrudeMesh.triangles();

    this->createFromGenerator(vertices, triangles);
}

void SolidPolyLine::setRadius(double radius)
{
    m_radius = radius;
    this->refresh();
}