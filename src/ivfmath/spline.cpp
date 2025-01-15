#include <ivfmath/spline.h>

using namespace ivfmath;

glm::vec3 Spline::interpolate(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
                              float t) const
{
    // Catmull-Rom basis matrix coefficients
    float t2 = t * t;
    float t3 = t2 * t;

    // Catmull-Rom spline blending functions
    float b0 = -0.5f * t3 + t2 - 0.5f * t;
    float b1 = 1.5f * t3 - 2.5f * t2 + 1.0f;
    float b2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
    float b3 = 0.5f * t3 - 0.5f * t2;

    return b0 * p0 + b1 * p1 + b2 * p2 + b3 * p3;
}

void Spline::calculateLengths()
{
    m_segmentLengths.clear();
    m_totalLength = 0.0f;

    if (m_controlPoints.size() < 2)
        return;

    // Handle cases with less than 4 points
    if (m_controlPoints.size() == 2)
    {
        float length = glm::length(m_controlPoints[1] - m_controlPoints[0]);
        m_segmentLengths.push_back(length);
        m_totalLength = length;
        return;
    }

    // Calculate length for each segment
    for (size_t i = 0; i < m_controlPoints.size() - 1; ++i)
    {
        float segmentLength = 0.0f;
        glm::vec3 prevPoint = getPointByT(i);

        // Approximate length using small steps
        for (int j = 1; j <= LENGTH_SAMPLES; ++j)
        {
            float t = static_cast<float>(j) / LENGTH_SAMPLES;
            glm::vec3 currentPoint = getPointByT(i + t);
            segmentLength += glm::length(currentPoint - prevPoint);
            prevPoint = currentPoint;
        }

        m_segmentLengths.push_back(segmentLength);
        m_totalLength += segmentLength;
    }
}

ivfmath::Spline::Spline() : m_totalLength(0.0f), m_controlPoints(), m_segmentLengths()
{}

std::shared_ptr<Spline> Spline::create()
{
    return std::make_shared<Spline>();
}

void Spline::addPoint(const glm::vec3 &point)
{
    m_controlPoints.push_back(point);

    // If we have enough points for a complete segment, recalculate lengths
    if (m_controlPoints.size() >= 4)
    {
        calculateLengths();
    }
}

// Get point by parameter t (0 <= t <= n-1)
glm::vec3 Spline::getPointByT(float t) const
{
    if (m_controlPoints.size() < 2)
    {
        throw std::runtime_error("Not enough control points for interpolation");
    }

    // Handle cases with less than 4 points
    if (m_controlPoints.size() == 2)
    {
        float normalizedT = t / (m_controlPoints.size() - 1);
        normalizedT = std::max(0.0f, std::min(1.0f, normalizedT));
        return glm::mix(m_controlPoints[0], m_controlPoints[1], normalizedT);
    }

    // For spline interpolation, we need at least 4 points
    // Create virtual points for the first and last segments if needed
    std::vector<glm::vec3> extendedPoints = m_controlPoints;
    if (m_controlPoints.size() == 3)
    {
        // Add virtual end point
        glm::vec3 virtualEnd = m_controlPoints[2] + (m_controlPoints[2] - m_controlPoints[1]);
        extendedPoints.push_back(virtualEnd);
    }

    // Normalize t to the range [0, n-1] where n is the number of segments
    float maxT = static_cast<float>(m_controlPoints.size() - 1);
    t = std::max(0.0f, std::min(t, maxT));

    // Find the segment and local parameter
    int segment = std::min(static_cast<int>(t), static_cast<int>(extendedPoints.size()) - 4);
    float localT = t - static_cast<float>(segment);

    return interpolate(m_controlPoints[segment], m_controlPoints[segment + 1], m_controlPoints[segment + 2],
                       m_controlPoints[segment + 3], localT);
}

// Get point by distance along the spline (0 <= distance <= totalLength)
glm::vec3 Spline::getPointByDistance(float distance) const
{
    if (m_controlPoints.size() < 4)
    {
        throw std::runtime_error("Not enough control points for interpolation");
    }

    if (distance <= 0.0f)
        return m_controlPoints[1];
    if (distance >= m_totalLength)
        return m_controlPoints[m_controlPoints.size() - 2];

    // Find the segment containing the distance
    float accumulatedLength = 0.0f;
    size_t segment = 0;

    for (size_t i = 0; i < m_segmentLengths.size(); ++i)
    {
        if (accumulatedLength + m_segmentLengths[i] >= distance)
        {
            segment = i;
            break;
        }
        accumulatedLength += m_segmentLengths[i];
    }

    // Calculate local parameter t based on distance within segment
    float segmentDistance = distance - accumulatedLength;
    float localT = segmentDistance / m_segmentLengths[segment];

    return getPointByT(segment + localT);
}

// Get total spline length
float Spline::getLength() const
{
    return m_totalLength;
}

// Get number of control points
size_t Spline::size() const
{
    return m_controlPoints.size();
}
