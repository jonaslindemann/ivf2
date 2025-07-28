#include <ivf/box.h>

#include <generator/BoxMesh.hpp>

using namespace ivf;
using namespace generator;

Box::Box(glm::vec3 size, glm::vec3 segments) : m_size(size), m_segments(segments.x, segments.y, segments.z)
{
    this->doSetup();
    this->setName("Box");
}

Box::Box(double w, double h, double d, int i, int j, int k)
{
    m_size.x = float(w);
    m_size.y = float(h);
    m_size.z = float(d);

    m_segments[0] = i;
    m_segments[1] = j;
    m_segments[2] = k;

    this->doSetup();
}

Box::~Box()
{}

Box::Box(const Box &other)
{
    m_size.x = float(other.m_size.x);
    m_size.y = float(other.m_size.y);
    m_size.z = float(other.m_size.z);

    m_segments[0] = other.m_segments[0];
    m_segments[1] = other.m_segments[1];
    m_segments[2] = other.m_segments[2];
}

Box &Box::operator=(const Box &other)
{
    m_size.x = float(other.m_size.x);
    m_size.y = float(other.m_size.y);
    m_size.z = float(other.m_size.z);

    m_segments[0] = other.m_segments[0];
    m_segments[1] = other.m_segments[1];
    m_segments[2] = other.m_segments[2];

    this->doSetup();

    return *this;
}

Box::Box(Box &&other) : m_size(std::move(other.m_size))
{
    std::copy(std::begin(other.m_segments), std::end(other.m_segments), std::begin(m_segments));
    other.clear();
    this->doSetup();
}
Box &Box::operator=(Box &&other)
{
    m_size = std::move(other.m_size);
    std::copy(std::begin(other.m_segments), std::end(other.m_segments), std::begin(m_segments));
    other.clear();
    this->doSetup();
    return *this;
}

std::shared_ptr<Box> Box::create(glm::vec3 size, glm::vec3 segments)
{
    return std::make_shared<Box>(size, segments);
}

void Box::setSegments(glm::uvec3 segments)
{
    m_segments[0] = segments.x;
    m_segments[1] = segments.y;
    m_segments[2] = segments.z;
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

glm::uvec3 Box::segments()
{
    return glm::uvec3(m_segments[0], m_segments[1], m_segments[2]);
}

void Box::doSetup()
{
    gml::dvec3 si;
    gml::ivec3 sg;

    si[0] = m_size.x;
    si[1] = m_size.y;
    si[2] = m_size.z;

    sg[0] = m_segments[0];
    sg[1] = m_segments[1];
    sg[2] = m_segments[2];

    BoxMesh box(si, sg);

    AnyGenerator<MeshVertex> vertices = box.vertices();
    AnyGenerator<Triangle> triangles = box.triangles();

    this->createFromGenerator(vertices, triangles);
}

void ivf::Box::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Size", &m_size, "Geometry");

    addProperty("Segments X", &m_segments[0], 1, 64, "Geometry");
    addProperty("Segments Y", &m_segments[1], 1, 64, "Geometry");
    addProperty("Segments Z", &m_segments[2], 1, 64, "Geometry");
}

void ivf::Box::onPropertyChanged(const std::string &propertyName)
{
    this->refresh();
}
