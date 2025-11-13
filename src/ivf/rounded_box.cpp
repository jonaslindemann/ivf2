#include <ivf/rounded_box.h>

#include <generator/RoundedBoxMesh.hpp>

using namespace ivf;
using namespace generator;

RoundedBox::RoundedBox(glm::vec3 size, glm::vec3 segments, double radius, int slices)
    : m_size(size), m_segments(segments), m_radius(radius), m_slices(slices)
{
    this->doSetup();
    this->setName("RoundedBox");
}

std::shared_ptr<RoundedBox> RoundedBox::create(glm::vec3 size, glm::vec3 segments, double radius, int slices)
{
    return std::make_shared<RoundedBox>(size, segments, radius, slices);
}

void RoundedBox::set(glm::vec3 size, glm::vec3 segments, double radius, int slices)
{
    m_size = size;
    m_segments = segments;
    m_radius = radius;
    m_slices = slices;
    this->refresh();
}

void RoundedBox::setSize(double w, double h, double d)
{
    m_size.x = float(w);
    m_size.y = float(h);
    m_size.z = float(d);
}

void RoundedBox::setSize(glm::vec3 size)
{
    m_size = size;
}

glm::vec3 RoundedBox::size()
{
    return glm::vec3();
}

void RoundedBox::setSegments(int i, int j, int k)
{
    m_segments.x = i;
    m_segments.y = j;
    m_segments.z = k;
}

void RoundedBox::setSegments(glm::uvec3 segments)
{
    m_segments = segments;
}

glm::uvec3 RoundedBox::segments()
{
    return m_segments;
}

void RoundedBox::setRadius(double radius)
{
    m_radius = radius;
}

double RoundedBox::radius()
{
    return m_radius;
}

void RoundedBox::setSlices(int slices)
{
    m_slices = slices;
}

int RoundedBox::slices()
{
    return m_slices;
}

void RoundedBox::doSetup()
{
    gml::dvec3 si;
    gml::ivec3 sg;

    si[0] = m_size.x;
    si[1] = m_size.y;
    si[2] = m_size.z;

    sg[0] = m_segments.x;
    sg[1] = m_segments.y;
    sg[2] = m_segments.z;

    RoundedBoxMesh rbox(m_radius, si, m_slices, sg);

    AnyGenerator<MeshVertex> vertices = rbox.vertices();
    AnyGenerator<Triangle> triangles = rbox.triangles();

    this->createFromGenerator(vertices, triangles);

    // Set bounding box for the rounded box
    glm::vec3 halfSize = m_size * 0.5f;
    setLocalBoundingBox(BoundingBox(-halfSize, halfSize));
}

void ivf::RoundedBox::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Size", &m_size, "Geometry");
    addProperty("Segments", &m_segments, "Geometry");
    addProperty("Rounding radius", &m_radius, "Geometry");
    addProperty("Slices", &m_slices, "Geometry");
}

void ivf::RoundedBox::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
