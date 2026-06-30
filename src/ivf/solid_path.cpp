#include <ivf/solid_path.h>

#include <ivf/extrusion_profile.h>
#include <ivf/path_frames.h>
#include <ivf/extrusion_builder.h>

#include <algorithm>

using namespace ivf;

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
    MeshNode::clear();

    if (m_points.size() < 2)
        return;

    ExtrusionProfile profile = ExtrusionProfile::circle(float(m_radius), m_segments);

    int samples = std::max(64, int(m_points.size()) * 16);
    std::vector<PathFrame> frames =
        buildPathFrames(m_points, SpineInterp::CatmullRom, JoinStyle::Angle, samples, FrameMethod::RotationMinimizing);

    ExtrusionOptions options;
    options.capStart = true;
    options.capEnd = true;

    MeshData data = ExtrusionBuilder::build(profile, frames, options);
    this->createFromMeshData(data);
}

void SolidPath::setRadius(double radius)
{
    m_radius = radius;
    this->refresh();
}
