#include <ivf/solid_polyline.h>

#include <ivf/extrusion_profile.h>
#include <ivf/path_frames.h>
#include <ivf/extrusion_builder.h>

using namespace ivf;

SolidPolyLine::SolidPolyLine(double radius, int segments) : m_radius(radius), m_segments(segments)
{
    this->doSetup();
    this->setName("SolidPolyLine");
}

std::shared_ptr<SolidPolyLine> SolidPolyLine::create(double radius, int segments)
{
    return std::make_shared<SolidPolyLine>(radius, segments);
}

void SolidPolyLine::doSetup()
{
    this->clear();

    if (m_points.size() < 2)
        return;

    ExtrusionProfile profile = ExtrusionProfile::circle(float(m_radius), m_segments);

    std::vector<PathFrame> frames =
        buildPathFrames(m_points, SpineInterp::Polyline, JoinStyle::Angle, 64, FrameMethod::RotationMinimizing);

    ExtrusionOptions options;
    options.capStart = true;
    options.capEnd = true;

    MeshData data = ExtrusionBuilder::build(profile, frames, options);
    this->createFromMeshData(data);
}

void SolidPolyLine::setRadius(double radius)
{
    m_radius = radius;
    this->refresh();
}

void ivf::SolidPolyLine::addPoint(const gml::dvec3 &point)
{
    m_points.push_back(glm::vec3(float(point[0]), float(point[1]), float(point[2])));
}

void ivf::SolidPolyLine::addPoint(double x, double y, double z)
{
    m_points.push_back(glm::vec3(float(x), float(y), float(z)));
}

void ivf::SolidPolyLine::setPoints(const std::vector<gml::dvec3> &points)
{
    m_points.clear();
    for (const auto &p : points)
        m_points.push_back(glm::vec3(float(p[0]), float(p[1]), float(p[2])));
}

void ivf::SolidPolyLine::clearPoints()
{
    m_points.clear();
}

void ivf::SolidPolyLine::setSegments(int segments)
{
    m_segments = segments;
}

void ivf::SolidPolyLine::setPoint(int index, const gml::dvec3 &point)
{
    if (index >= 0 && index < int(m_points.size()))
        m_points[index] = glm::vec3(float(point[0]), float(point[1]), float(point[2]));
}

gml::dvec3 ivf::SolidPolyLine::point(int index) const
{
    if (index >= 0 && index < int(m_points.size()))
        return gml::dvec3(m_points[index].x, m_points[index].y, m_points[index].z);
    return gml::dvec3(0.0, 0.0, 0.0);
}

int ivf::SolidPolyLine::pointCount()
{
    return int(m_points.size());
}
