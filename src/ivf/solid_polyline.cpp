#include <ivf/solid_polyline.h>

#include <generator/SphereMesh.hpp>

using namespace ivf;
using namespace generator;

SolidPolyLine::SolidPolyLine(double radius, int segments) : m_radius(radius), m_polyPath(segments)
{
    this->doSetup();
    this->setName("SolidPolyLine");
}

std::shared_ptr<SolidPolyLine> SolidPolyLine::create(double radius, int segments)
{
    return std::make_shared<SolidPolyLine>(radius);
}

void SolidPolyLine::doSetup()
{
    gml::dvec3 n(1.0, 0.0, 0.0);

    // LinePath line(p0, p1, n, 8);
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

void ivf::SolidPolyLine::addPoint(const gml::dvec3 &point)
{
    m_polyPath.addPoint(gml::dvec3(point[0], point[2], point[1]));
}

void ivf::SolidPolyLine::addPoint(double x, double y, double z)
{
    m_polyPath.addPoint(gml::dvec3(x, z, y));
}

void ivf::SolidPolyLine::setPoints(const std::vector<gml::dvec3> &points)
{
    m_polyPath.setPoints(points);
}

void ivf::SolidPolyLine::clearPoints()
{
    m_polyPath.clearPoints();
}

void ivf::SolidPolyLine::setSegments(int segments)
{
    m_polyPath.setSegments(segments);
}

void ivf::SolidPolyLine::setPoint(int index, const gml::dvec3 &point)
{
    m_polyPath.setPoint(index, point);
}

gml::dvec3 ivf::SolidPolyLine::point(int index) const
{
    return m_polyPath.point(index);
}

int ivf::SolidPolyLine::pointCount()
{
    return m_polyPath.pointCount();
}
