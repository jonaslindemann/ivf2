#include <ivf/extrusion_profile.h>

#include <cmath>

using namespace ivf;

namespace {

// 2D outward edge normal for an edge with direction d (rotate -90 degrees).
glm::vec2 edgeNormal(const glm::vec2 &d)
{
    glm::vec2 n(d.y, -d.x);
    float len = glm::length(n);
    if (len > 1e-8f)
        return n / len;
    return glm::vec2(0.0f, 0.0f);
}

} // namespace

ExtrusionProfile::ExtrusionProfile(std::vector<glm::vec2> points, bool closed)
    : m_points(std::move(points)), m_closed(closed)
{}

ExtrusionProfile ExtrusionProfile::circle(float radius, int segments, float start, float sweep)
{
    if (segments < 3)
        segments = 3;

    bool fullCircle = std::abs(sweep - glm::two_pi<float>()) < 1e-4f;

    std::vector<glm::vec2> points;
    std::vector<glm::vec2> normals;

    // For a full circle we emit `segments` points without duplicating the seam. For a partial
    // sweep we emit segments+1 points so the arc endpoints are represented.
    int count = fullCircle ? segments : segments + 1;
    float step = sweep / static_cast<float>(segments);

    for (int i = 0; i < count; ++i)
    {
        float a = start + step * static_cast<float>(i);
        glm::vec2 p(radius * std::cos(a), radius * std::sin(a));
        points.push_back(p);
        normals.push_back(glm::vec2(std::cos(a), std::sin(a)));
    }

    ExtrusionProfile profile(std::move(points), fullCircle);
    profile.setNormals(normals);
    return profile;
}

ExtrusionProfile ExtrusionProfile::rectangle(float width, float height)
{
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    std::vector<glm::vec2> points = {
        {-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}};

    return ExtrusionProfile(std::move(points), true);
}

ExtrusionProfile ExtrusionProfile::roundedRect(float width, float height, float radius, int cornerSegments)
{
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    radius = std::min(radius, std::min(hw, hh));
    if (cornerSegments < 1)
        cornerSegments = 1;

    std::vector<glm::vec2> points;

    // Corner centers and starting angles (CCW order: bottom-right, top-right, top-left, bottom-left).
    struct Corner
    {
        glm::vec2 center;
        float startAngle;
    };

    const float halfPi = glm::half_pi<float>();
    Corner corners[4] = {
        {{hw - radius, -hh + radius}, -halfPi},
        {{hw - radius, hh - radius}, 0.0f},
        {{-hw + radius, hh - radius}, halfPi},
        {{-hw + radius, -hh + radius}, 2.0f * halfPi}};

    for (const auto &c : corners)
    {
        for (int i = 0; i <= cornerSegments; ++i)
        {
            float a = c.startAngle + halfPi * static_cast<float>(i) / static_cast<float>(cornerSegments);
            points.push_back(c.center + radius * glm::vec2(std::cos(a), std::sin(a)));
        }
    }

    return ExtrusionProfile(std::move(points), true);
}

ExtrusionProfile ExtrusionProfile::regularPolygon(int sides, float radius)
{
    if (sides < 3)
        sides = 3;

    std::vector<glm::vec2> points;
    for (int i = 0; i < sides; ++i)
    {
        float a = glm::two_pi<float>() * static_cast<float>(i) / static_cast<float>(sides);
        points.push_back(radius * glm::vec2(std::cos(a), std::sin(a)));
    }

    return ExtrusionProfile(std::move(points), true);
}

ExtrusionProfile ExtrusionProfile::polyline(const std::vector<glm::vec2> &points, bool closed)
{
    return ExtrusionProfile(points, closed);
}

void ExtrusionProfile::addPoint(const glm::vec2 &p)
{
    m_points.push_back(p);
    m_normals.clear(); // explicit normals no longer valid
}

void ExtrusionProfile::setPoints(const std::vector<glm::vec2> &points)
{
    m_points = points;
    m_normals.clear();
}

void ExtrusionProfile::clear()
{
    m_points.clear();
    m_normals.clear();
}

const std::vector<glm::vec2> &ExtrusionProfile::points() const
{
    return m_points;
}

std::size_t ExtrusionProfile::pointCount() const
{
    return m_points.size();
}

void ExtrusionProfile::setClosed(bool closed)
{
    m_closed = closed;
}

bool ExtrusionProfile::closed() const
{
    return m_closed;
}

void ExtrusionProfile::setNormals(const std::vector<glm::vec2> &normals)
{
    m_normals = normals;
}

const std::vector<glm::vec2> &ExtrusionProfile::normals() const
{
    return m_normals;
}

bool ExtrusionProfile::hasNormals() const
{
    return !m_normals.empty() && m_normals.size() == m_points.size();
}

glm::vec2 ExtrusionProfile::centroid() const
{
    if (m_points.empty())
        return glm::vec2(0.0f);

    glm::vec2 c(0.0f);
    for (const auto &p : m_points)
        c += p;
    return c / static_cast<float>(m_points.size());
}

std::vector<glm::vec2> ExtrusionProfile::computeSmoothNormals() const
{
    std::vector<glm::vec2> normals;
    int n = static_cast<int>(m_points.size());
    if (n == 0)
        return normals;

    glm::vec2 c = centroid();
    normals.resize(n);

    for (int i = 0; i < n; ++i)
    {
        glm::vec2 nn(0.0f);

        if (m_closed)
        {
            glm::vec2 e0 = m_points[i] - m_points[(i - 1 + n) % n];
            glm::vec2 e1 = m_points[(i + 1) % n] - m_points[i];
            nn = edgeNormal(e0) + edgeNormal(e1);
        }
        else
        {
            if (i > 0)
                nn += edgeNormal(m_points[i] - m_points[i - 1]);
            if (i < n - 1)
                nn += edgeNormal(m_points[i + 1] - m_points[i]);
        }

        float len = glm::length(nn);
        if (len > 1e-8f)
            nn /= len;
        else
            nn = glm::normalize(m_points[i] - c);

        // Orient outward from the centroid.
        if (glm::dot(nn, m_points[i] - c) < 0.0f)
            nn = -nn;

        normals[i] = nn;
    }

    return normals;
}
