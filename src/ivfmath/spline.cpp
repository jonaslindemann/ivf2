#include <ivfmath/spline.h>

#include <algorithm>

using namespace ivfmath;

ivfmath::Spline::Spline()
{}

std::shared_ptr<Spline> ivfmath::Spline::create()
{
    return std::make_shared<Spline>();
}

void Spline::addPoint(const glm::vec3 &point)
{
    m_points.push_back(point);
    updateDistances();
}

void Spline::updateDistances()
{
    m_distances.clear();
    m_distances.push_back(0.0f);

    for (size_t i = 1; i < m_points.size(); ++i)
    {
        float segmentLength = glm::length(m_points[i] - m_points[i - 1]);
        m_distances.push_back(m_distances.back() + segmentLength);
    }
}

float Spline::distanceToT(float distance) const
{
    if (m_points.size() < 2)
        return 0.0f;

    // Find the segment containing this distance
    auto it = std::lower_bound(m_distances.begin(), m_distances.end(), distance);
    if (it == m_distances.begin())
        return 0.0f;
    if (it == m_distances.end())
        return static_cast<float>(m_points.size() - 1);

    // Get segment index and interpolation factor
    size_t index = std::distance(m_distances.begin(), it) - 1;
    float segmentStart = m_distances[index];
    float segmentLength = m_distances[index + 1] - segmentStart;
    float segmentT = (distance - segmentStart) / segmentLength;

    return static_cast<float>(index) + segmentT;
}

glm::vec3 Spline::positionByDistance(float distance) const
{
    return positionByT(distanceToT(distance));
}

glm::vec3 Spline::positionByT(float t) const
{
    if (m_points.size() < 2)
    {
        return m_points.empty() ? glm::vec3(0.0f) : m_points[0];
    }

    // Clamp t to valid range
    t = std::max(0.0f, std::min(t, static_cast<float>(m_points.size() - 1)));

    // Find the segment containing t
    size_t segment = static_cast<size_t>(t);
    if (segment >= m_points.size() - 1)
    {
        return m_points.back();
    }

    // Get surrounding points for interpolation
    glm::vec3 p0 = segment > 0 ? m_points[segment - 1] : m_points[0];
    glm::vec3 p1 = m_points[segment];
    glm::vec3 p2 = m_points[segment + 1];
    glm::vec3 p3 = segment < m_points.size() - 2 ? m_points[segment + 2] : p2;

    float localT = t - static_cast<float>(segment);
    return interpolate(p0, p1, p2, p3, localT);
}

glm::vec3 Spline::interpolate(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
                              float t) const
{
    // Catmull-Rom spline interpolation
    float t2 = t * t;
    float t3 = t2 * t;

    glm::vec3 a = p1;
    glm::vec3 b = 0.5f * (p2 - p0);
    glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
    glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

    return a + b * t + c * t2 + d * t3;
}

float Spline::totalLength() const
{
    return m_distances.empty() ? 0.0f : m_distances.back();
}

size_t Spline::size() const
{
    return m_points.size();
}

void Spline::clear()
{
    m_points.clear();
    m_distances.clear();
}
