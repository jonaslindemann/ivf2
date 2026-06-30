#include <ivf/solid_line.h>

#include <ivf/extrusion_profile.h>
#include <ivf/path_frames.h>
#include <ivf/extrusion_builder.h>

using namespace ivf;

SolidLine::SolidLine(glm::vec3 p0, glm::vec3 p1, double radius) : m_p0(p0), m_p1(p1), m_radius(radius)
{
    this->doSetup();
    this->setName("SolidLine");
}

std::shared_ptr<SolidLine> SolidLine::create(glm::vec3 p0, glm::vec3 p1, double radius)
{
    return std::make_shared<SolidLine>(p0, p1, radius);
}

void SolidLine::doSetup()
{
    this->clear();

    ExtrusionProfile profile = ExtrusionProfile::circle(float(m_radius), 32);

    std::vector<glm::vec3> spine = {m_p0, m_p1};
    std::vector<PathFrame> frames =
        buildPathFrames(spine, SpineInterp::Polyline, JoinStyle::Angle, 2, FrameMethod::RotationMinimizing);

    ExtrusionOptions options;
    options.capStart = true;
    options.capEnd = true;

    MeshData data = ExtrusionBuilder::build(profile, frames, options);
    this->createFromMeshData(data);
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
