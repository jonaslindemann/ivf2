#include <ivf/box.h>

#include <generator/BoxMesh.hpp>

using namespace ivf;
using namespace generator;

Box::Box(glm::vec3 size, glm::vec3 segments) : m_size(size), m_segments(segments)
{
    this->doSetup();
    this->setName("Box");
}

Box::Box(double w, double h, double d, int i, int j, int k)
{
    m_size.x = float(w);
    m_size.y = float(h);
    m_size.z = float(d);

    m_segments.x = i;
    m_segments.y = j;
    m_segments.z = k;

    this->doSetup();
}

Box::~Box()
{}

Box::Box(const Box &other)
{
    m_size.x = float(other.m_size.x);
    m_size.y = float(other.m_size.y);
    m_size.z = float(other.m_size.z);

    m_segments.x = other.m_segments.x;
    m_segments.y = other.m_segments.y;
    m_segments.z = other.m_segments.z;
}

Box &Box::operator=(const Box &other)
{
    m_size.x = float(other.m_size.x);
    m_size.y = float(other.m_size.y);
    m_size.z = float(other.m_size.z);

    m_segments.x = other.m_segments.x;
    m_segments.y = other.m_segments.y;
    m_segments.z = other.m_segments.z;

    this->doSetup();

    return *this;
}

Box::Box(Box &&other) : m_size(std::move(other.m_size)), m_segments(std::move(other.m_segments))
{
    other.clear();
    this->doSetup();
}

Box &Box::operator=(Box &&other)
{
    m_size = std::move(other.m_size);
    m_segments = std::move(other.m_segments);
    other.clear();
    this->doSetup();
    return *this;
}

std::shared_ptr<Box> Box::create(glm::vec3 size, glm::vec3 segments)
{
    return std::make_shared<Box>(size, segments);
}

void Box::set(glm::vec3 size, glm::vec3 segments)
{
    m_size = size;
    m_segments = segments;
    this->refresh();
}

void Box::setSize(double w, double h, double d)
{
    m_size.x = float(w);
    m_size.y = float(h);
    m_size.z = float(d);
}

void Box::setSize(glm::vec3 size)
{
    m_size = size;
}

glm::vec3 Box::size()
{
    return glm::vec3();
}

void Box::setSegments(int i, int j, int k)
{
    m_segments.x = i;
    m_segments.y = j;
    m_segments.z = k;
}

void Box::setSegments(glm::uvec3 segments)
{
    m_segments = segments;
}

glm::uvec3 Box::segments()
{
    return m_segments;
}

void Box::doSetup()
{
    gml::dvec3 si;
    gml::ivec3 sg;

    si[0] = m_size.x;
    si[1] = m_size.y;
    si[2] = m_size.z;

    sg[0] = m_segments.x;
    sg[1] = m_segments.y;
    sg[2] = m_segments.z;

    BoxMesh box(si, sg);

    AnyGenerator<MeshVertex> vertices = box.vertices();
    AnyGenerator<Triangle> triangles = box.triangles();

    this->createFromGenerator(vertices, triangles);
}

void ivf::Box::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Size", &m_size, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
}

void ivf::Box::onPropertyChanged(const std::string &propertyName)
{
    this->refresh();
}
