#include <ivf/extrusion.h>

#include <ivf/mesh_manager.h>

using namespace ivf;
using namespace std;

ivf::Extrusion::Extrusion()
{
    this->setUseTexture(false);
    this->setUseMaterial(true);
    this->setName("Extrusion");
}

std::shared_ptr<Extrusion> ivf::Extrusion::create()
{
    return std::make_shared<Extrusion>();
}

void ivf::Extrusion::setProfile(const ExtrusionProfile &profile)
{
    m_profile = profile;
    m_profileExplicit = true;
}

const ExtrusionProfile &ivf::Extrusion::profile() const
{
    return m_profile;
}

void ivf::Extrusion::setSpine(const std::vector<glm::vec3> &points, SpineInterp interp, int samples)
{
    m_spinePoints = points;
    m_spineInterp = static_cast<int>(interp);
    m_samples = samples;
}

void ivf::Extrusion::setClosedSpine(bool closed)
{
    m_closedSpine = closed;
}

void ivf::Extrusion::setCaps(bool start, bool end)
{
    m_capStart = start;
    m_capEnd = end;
}

void ivf::Extrusion::setJoinStyle(JoinStyle style)
{
    m_joinStyle = static_cast<int>(style);
}

void ivf::Extrusion::setFrameMethod(FrameMethod method)
{
    m_frameMethod = static_cast<int>(method);
}

void ivf::Extrusion::setSpineInterp(SpineInterp interp)
{
    m_spineInterp = static_cast<int>(interp);
}

void ivf::Extrusion::setNormalStyle(NormalStyle style)
{
    m_normalStyle = static_cast<int>(style);
}

void ivf::Extrusion::setTexMode(TexMode mode)
{
    m_texMode = static_cast<int>(mode);
}

void ivf::Extrusion::setUpVector(const glm::vec3 &up)
{
    m_upVector = up;
}

void ivf::Extrusion::setCornerRadius(double radius)
{
    m_cornerRadius = radius;
}

void ivf::Extrusion::setCornerSegments(int segments)
{
    m_cornerSegments = segments;
}

void ivf::Extrusion::setSamples(int samples)
{
    m_samples = samples;
}

void ivf::Extrusion::setSectionTransform(std::function<SectionTransform(float)> fn)
{
    m_sectionFn = std::move(fn);
}

void ivf::Extrusion::addPathPoint(glm::vec3 p)
{
    m_spinePoints.push_back(p);
}

void ivf::Extrusion::clearPath()
{
    m_spinePoints.clear();
}

void ivf::Extrusion::addSectionPoint(glm::vec2 p)
{
    m_profile.addPoint(p);
    m_profile.setClosed(true);
    m_profileExplicit = true;
}

void ivf::Extrusion::clearSection()
{
    m_profile.clear();
    m_profileExplicit = false;
}

void ivf::Extrusion::createCircleSection(float r, int segments)
{
    m_radius = r;
    m_profileSegments = segments;
    m_profile = ExtrusionProfile::circle(r, segments);
    m_profileExplicit = true;
}

void ivf::Extrusion::doSetup()
{
    this->clear();

    if (m_spinePoints.size() < 2)
        return;

    ExtrusionProfile profile =
        m_profileExplicit ? m_profile : ExtrusionProfile::circle(float(m_radius), m_profileSegments);

    if (profile.pointCount() < 2)
        return;

    std::vector<PathFrame> frames = buildPathFrames(
        m_spinePoints, static_cast<SpineInterp>(m_spineInterp), static_cast<JoinStyle>(m_joinStyle), m_samples,
        static_cast<FrameMethod>(m_frameMethod), m_upVector, m_closedSpine, float(m_cornerRadius), m_cornerSegments);

    if (frames.size() < 2)
        return;

    ExtrusionOptions options;
    options.capStart = m_capStart;
    options.capEnd = m_capEnd;
    options.closedPath = m_closedSpine;
    options.normalStyle = static_cast<NormalStyle>(m_normalStyle);
    options.texMode = static_cast<TexMode>(m_texMode);
    options.sectionFn = m_sectionFn;

    MeshData data = ExtrusionBuilder::build(profile, frames, options);

    this->createFromMeshData(data);
}

void ivf::Extrusion::setupProperties()
{
    MeshNode::setupProperties();
    addProperty("Radius", &m_radius, "Geometry");
    addProperty("Profile segments", &m_profileSegments, "Geometry");
    addProperty("Cap start", &m_capStart, "Geometry");
    addProperty("Cap end", &m_capEnd, "Geometry");
    addProperty("Closed spine", &m_closedSpine, "Geometry");
    addProperty("Spine interp (0=poly,1=spline)", &m_spineInterp, "Path");
    addProperty("Samples", &m_samples, "Path");
    addProperty("Join (0=raw,1=angle,2=cut,3=round)", &m_joinStyle, "Path");
    addProperty("Corner radius", &m_cornerRadius, "Path");
    addProperty("Corner segments", &m_cornerSegments, "Path");
    addProperty("Frame (0=rmf,1=frenet,2=fixed)", &m_frameMethod, "Path");
    addProperty("Normal (0=smooth,1=facet)", &m_normalStyle, "Surface");
    addProperty("Tex (0=norm,1=length)", &m_texMode, "Surface");
}

void ivf::Extrusion::onPropertyChanged(const std::string &name)
{
    this->refresh();
}
