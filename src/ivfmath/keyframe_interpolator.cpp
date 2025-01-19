#include <ivfmath/keyframe_interpolator.h>
#include <algorithm>
#include <cmath>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

using namespace ivfmath;

KeyframeInterpolator::KeyframeInterpolator(const std::string &name)
    : m_name(name), m_duration(0.0f), m_interpolationType(InterpolationType::Linear), m_defaultTension(0.5f)
{}

std::shared_ptr<KeyframeInterpolator> ivfmath::KeyframeInterpolator::create(const std::string &name)
{
    return std::make_shared<KeyframeInterpolator>(name);
}

void KeyframeInterpolator::addKeyframe(const KeyframeData &keyframe)
{
    m_keyframes.push_back(keyframe);
    // Sort keyframes by timestamp
    std::sort(m_keyframes.begin(), m_keyframes.end(),
              [](const KeyframeData &a, const KeyframeData &b) { return a.timestamp < b.timestamp; });
    // Update duration
    if (!m_keyframes.empty())
    {
        m_duration = m_keyframes.back().timestamp;
    }
}

/*
KeyframeData KeyframeInterpolator::interpolate(float time) const
{
    if (m_keyframes.empty())
        return KeyframeData();
    if (m_keyframes.size() == 1)
        return m_keyframes[0];

    // Handle loop or clamp
    time = std::fmod(time, m_duration);
    if (time < 0)
        time += m_duration;

    // Find the surrounding keyframes
    auto next = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), time,
                                 [](const KeyframeData &kf, float t) { return kf.timestamp < t; });

    if (next == m_keyframes.begin())
        return *next;
    if (next == m_keyframes.end())
        return m_keyframes.back();

    auto prev = next - 1;
    float t = (time - prev->timestamp) / (next->timestamp - prev->timestamp);

    // Apply easing function
    t = applyEasing(t);

    // Interpolate values
    KeyframeData result;
    result.position = glm::mix(prev->position, next->position, t);
    result.rotation = glm::mix(prev->rotation, next->rotation, t);
    result.scale = glm::mix(prev->scale, next->scale, t);

    return result;
}
*/

float KeyframeInterpolator::applyEasing(float t) const
{
    switch (m_interpolationType)
    {
    case InterpolationType::Linear:
        return t;
    case InterpolationType::EaseIn:
        return t * t;
    case InterpolationType::EaseOut:
        return 1.0f - (1.0f - t) * (1.0f - t);
    case InterpolationType::EaseInOut:
        return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    case InterpolationType::Step:
        return t < 1.0f ? 0.0f : 1.0f;
    default:
        return t;
    }
}

void KeyframeInterpolator::setInterpolationType(InterpolationType type)
{
    m_interpolationType = type;
}

float KeyframeInterpolator::getDuration() const
{
    return m_duration;
}

const std::string &KeyframeInterpolator::getName() const
{
    return m_name;
}

void KeyframeInterpolator::setSplineTension(float tension)
{
    m_defaultTension = std::clamp(tension, 0.0f, 1.0f);
}

void KeyframeInterpolator::setKeyframeTension(size_t index, float tension)
{
    if (index < m_keyframes.size())
    {
        m_keyframes[index].tension = std::clamp(tension, 0.0f, 1.0f);
    }
}

float KeyframeInterpolator::getKeyframeTension(size_t index) const
{
    if (index < m_keyframes.size())
    {
        return m_keyframes[index].tension;
    }
    return m_defaultTension;
}

KeyframeData KeyframeInterpolator::interpolate(float time) const
{
    if (m_keyframes.empty())
        return KeyframeData();
    if (m_keyframes.size() == 1)
        return m_keyframes[0];

    // Handle loop or clamp
    time = std::fmod(time, m_duration);
    if (time < 0)
        time += m_duration;

    // Find the correct keyframe segment
    size_t prevIndex = 0;
    for (size_t i = 0; i < m_keyframes.size() - 1; ++i)
    {
        if (time >= m_keyframes[i].timestamp && time <= m_keyframes[i + 1].timestamp)
        {
            prevIndex = i;
            break;
        }
    }

    // Calculate local interpolation parameter
    float segmentDuration = m_keyframes[prevIndex + 1].timestamp - m_keyframes[prevIndex].timestamp;
    float localTime = (time - m_keyframes[prevIndex].timestamp) / segmentDuration;

    // Apply easing if not using spline interpolation
    if (m_interpolationType != InterpolationType::CatmullRom)
    {
        localTime = applyEasing(localTime);
    }

    KeyframeData result;

    if (m_interpolationType == InterpolationType::CatmullRom)
    {
        auto [p0, p1, p2, p3] = getSplinePoints(prevIndex, time);

        // Get tensions from the two keyframes being interpolated between
        float tension1 = m_keyframes[prevIndex].tension;
        float tension2 = m_keyframes[prevIndex + 1].tension;

        // Apply spline interpolation
        result.position = interpolateSpline(p0, p1, p2, p3, localTime, tension1, tension2);

        // For rotation and scale, use regular interpolation
        result.rotation = glm::mix(m_keyframes[prevIndex].rotation, m_keyframes[prevIndex + 1].rotation, localTime);
        result.scale = glm::mix(m_keyframes[prevIndex].scale, m_keyframes[prevIndex + 1].scale, localTime);
    }
    else
    {
        // Regular linear interpolation
        result.position = glm::mix(m_keyframes[prevIndex].position, m_keyframes[prevIndex + 1].position, localTime);
        result.rotation = glm::mix(m_keyframes[prevIndex].rotation, m_keyframes[prevIndex + 1].rotation, localTime);
        result.scale = glm::mix(m_keyframes[prevIndex].scale, m_keyframes[prevIndex + 1].scale, localTime);
    }

    result.timestamp = time;
    result.tension = glm::mix(m_keyframes[prevIndex].tension, m_keyframes[prevIndex + 1].tension, localTime);

    return result;
}

// Also modify getSplinePoints to use the correct points for the Catmull-Rom spline:
std::tuple<glm::vec3, glm::vec3, glm::vec3, glm::vec3> KeyframeInterpolator::getSplinePoints(size_t currentIndex,
                                                                                             float time) const
{
    const size_t numKeyframes = m_keyframes.size();

    // Get indices for the four control points
    size_t p0Index = (currentIndex == 0) ? currentIndex : currentIndex - 1;
    size_t p1Index = currentIndex;
    size_t p2Index = currentIndex + 1;
    size_t p3Index = (p2Index + 1 < numKeyframes) ? p2Index + 1 : p2Index;

    // Get the positions for each control point
    glm::vec3 p0 = m_keyframes[p0Index].position;
    glm::vec3 p1 = m_keyframes[p1Index].position;
    glm::vec3 p2 = m_keyframes[p2Index].position;
    glm::vec3 p3 = m_keyframes[p3Index].position;

    // Handle endpoint cases
    if (currentIndex == 0)
    {
        // Generate a phantom point before the first point
        p0 = p1 - (p2 - p1);
    }
    else if (currentIndex >= numKeyframes - 2)
    {
        // Generate a phantom point after the last point
        p3 = p2 + (p2 - p1);
    }

    return {p0, p1, p2, p3};
}

// Also update interpolateSpline to ensure smooth blending:
glm::vec3 KeyframeInterpolator::interpolateSpline(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
                                                  const glm::vec3 &p3, float t, float tension1, float tension2) const
{
    float t2 = t * t;
    float t3 = t2 * t;

    // Blend between the two tension values based on t
    float blendedTension = glm::mix(tension1, tension2, t);
    float alpha = 0.5f; // Standard Catmull-Rom alpha

    // Apply tension influence
    alpha = alpha * (1.0f - blendedTension);

    // Compute basis functions
    float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h2 = -2.0f * t3 + 3.0f * t2;
    float h3 = t3 - 2.0f * t2 + t;
    float h4 = t3 - t2;

    // Compute tangents
    glm::vec3 m1 = alpha * (p2 - p0);
    glm::vec3 m2 = alpha * (p3 - p1);

    // Compute final position
    return h1 * p1 + h2 * p2 + h3 * m1 + h4 * m2;
}
