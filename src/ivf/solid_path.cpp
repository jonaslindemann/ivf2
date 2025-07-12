#include <ivf/solid_path.h>

#include <ivf/point_path.h>

#include <generator/SphereMesh.hpp>

using namespace ivf;
using namespace generator;

SolidPath::SolidPath(double radius, int segments) : m_radius(radius), m_segments(segments)
{
    this->setName("SolidPath");
}

std::shared_ptr<SolidPath> SolidPath::create(double radius, int segments)
{
    return std::make_shared<SolidPath>(radius, segments);
}

void ivf::SolidPath::addPoint(glm::vec3 p)
{
    m_points.push_back(p);
}

void ivf::SolidPath::clear()
{
    m_points.clear();
}

void SolidPath::doSetup()
{
    PointPath path(32, m_points);

    CircleShape circle(m_radius, m_segments);
    ExtrudeMesh<CircleShape, PointPath> extrudeMesh(circle, path);

    AnyGenerator<MeshVertex> vertices = extrudeMesh.vertices();
    AnyGenerator<Triangle> triangles = extrudeMesh.triangles();

    this->createFromGenerator(vertices, triangles);
    // this->debugFromGenerator(vertices, triangles);

    /*
     for (auto& p : m_points)
     {
         gml::dvec3 p0(m_p0.x, m_p0.y, m_p0.z);
         gml::dvec3 p1(m_p1.x, m_p1.y, m_p1.z);
         gml::dvec3 n(1.0, 0.0, 0.0);

     }

     LinePath line(p0, p1, n, 8);
     CircleShape circle(m_radius, 32);

     ExtrudeMesh<CircleShape, LinePath> extrudeMesh(circle, line);

     AnyGenerator<MeshVertex> vertices = extrudeMesh.vertices();
     AnyGenerator<Triangle> triangles = extrudeMesh.triangles();

     this->createFromGenerator(vertices, triangles);
     */
}

void SolidPath::setRadius(double radius)
{
    m_radius = radius;
    this->refresh();
}
