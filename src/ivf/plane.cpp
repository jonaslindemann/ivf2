#include <ivf/plane.h>

#include <generator/PlaneMesh.hpp>

using namespace ivf;
using namespace generator;

Plane::Plane(double width, double depth, int rows, int cols)
    : m_width(width), m_depth(depth), m_rows(rows), m_cols(cols)
{
    this->doSetup();
    this->setName("Plane");
}

std::shared_ptr<Plane> Plane::create(double width, double depth, int rows, int cols)
{
    return std::make_shared<Plane>(width, depth, rows, cols);
}

void Plane::set(double width, double depth, int rows, int cols)
{
    m_width = width;
    m_depth = depth;
    m_rows = rows;
    m_cols = cols;
    this->refresh();
}

void Plane::doSetup()
{
    PlaneMesh planeMesh(gml::dvec2(m_width, m_depth), gml::ivec2(m_rows, m_cols));

    AnyGenerator<MeshVertex> vertices = planeMesh.vertices();
    AnyGenerator<Triangle> triangles = planeMesh.triangles();

    this->createFromGenerator(vertices, triangles);
}
