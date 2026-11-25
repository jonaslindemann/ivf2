#include <ivf/shadow_math.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

using namespace ivf;

const char *ivf::shadowFilterName(ShadowFilter filter)
{
    switch (filter)
    {
    case ShadowFilter::Hard:
        return "Hard";
    case ShadowFilter::Pcf:
        return "PCF";
    case ShadowFilter::Pcss:
        return "PCSS";
    }
    return "Unknown";
}

glm::vec2 ivf::projectionNearFar(const glm::mat4 &projection)
{
    const float a = projection[2][2];
    const float b = projection[3][2];
    if (projection[2][3] != 0.0f) // perspective: w = -z
        return {b / (a - 1.0f), b / (a + 1.0f)};
    return {(b + 1.0f) / a, (b - 1.0f) / a}; // orthographic
}

std::vector<float> ivf::cascadeSplits(float nearDist, float farDist, int count, float lambda)
{
    count = std::max(count, 1);
    nearDist = std::max(nearDist, 1e-4f);
    farDist = std::max(farDist, nearDist * 1.001f);
    lambda = std::clamp(lambda, 0.0f, 1.0f);

    std::vector<float> splits(static_cast<std::size_t>(count));
    for (int i = 1; i <= count; ++i)
    {
        const float f = float(i) / float(count);
        const float logSplit = nearDist * std::pow(farDist / nearDist, f);
        const float uniformSplit = nearDist + (farDist - nearDist) * f;
        splits[static_cast<std::size_t>(i - 1)] = lambda * logSplit + (1.0f - lambda) * uniformSplit;
    }
    splits.back() = farDist;
    return splits;
}

std::array<glm::vec3, 8> ivf::frustumSliceCorners(const glm::mat4 &view, const glm::mat4 &projection, float fromDist,
                                                 float toDist)
{
    const glm::mat4 inverse = glm::inverse(projection * view);
    const glm::vec2 nearFar = projectionNearFar(projection);
    const float range = nearFar.y - nearFar.x;
    const float t0 = (fromDist - nearFar.x) / range;
    const float t1 = (toDist - nearFar.x) / range;

    std::array<glm::vec3, 8> corners;
    int i = 0;
    for (float y : {-1.0f, 1.0f})
        for (float x : {-1.0f, 1.0f})
        {
            glm::vec4 n = inverse * glm::vec4(x, y, -1.0f, 1.0f);
            glm::vec4 f = inverse * glm::vec4(x, y, 1.0f, 1.0f);
            const glm::vec3 nearCorner = glm::vec3(n) / n.w;
            const glm::vec3 farCorner = glm::vec3(f) / f.w;
            // Points on a corner ray move linearly with the view distance
            corners[static_cast<std::size_t>(i)] = glm::mix(nearCorner, farCorner, t0);
            corners[static_cast<std::size_t>(i + 4)] = glm::mix(nearCorner, farCorner, t1);
            ++i;
        }
    return corners;
}

namespace {

/// An up vector that is not parallel to @p dir.
glm::vec3 upFor(const glm::vec3 &dir)
{
    return std::abs(dir.y) > 0.99f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
}

} // namespace

glm::mat4 ivf::fitCascade(const std::array<glm::vec3, 8> &corners, const glm::vec3 &lightDir,
                          const BoundingBox &casters, int resolution)
{
    glm::vec3 center(0.0f);
    for (const auto &c : corners)
        center += c;
    center /= 8.0f;

    float radius = 0.0f;
    for (const auto &c : corners)
        radius = std::max(radius, glm::length(c - center));
    // Round up, so the size stays the same while the camera turns or moves
    radius = std::max(std::ceil(radius * 16.0f) / 16.0f, 1e-3f);

    const glm::vec3 dir = glm::normalize(lightDir);
    const glm::mat4 view = glm::lookAt(center - dir * radius, center, upFor(dir));

    // The slice lies between distance 0 and 2 radius from the eye; casters between the
    // light and the slice must be in the map too
    float nearDist = 0.0f;
    if (casters.isValid())
        for (const auto &c : casters.corners())
            nearDist = std::min(nearDist, -(view * glm::vec4(c, 1.0f)).z);
    nearDist -= 0.01f * radius;
    const float farDist = 2.0f * radius;

    glm::mat4 projection = glm::ortho(-radius, radius, -radius, radius, nearDist, farDist);

    // Snap the world origin to a texel, so the map moves in whole texels
    const glm::vec4 origin = projection * view * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const float half = float(std::max(resolution, 1)) * 0.5f;
    const glm::vec2 texelPos = glm::vec2(origin) * half;
    const glm::vec2 offset = (glm::round(texelPos) - texelPos) / half;
    projection[3][0] += offset.x;
    projection[3][1] += offset.y;

    return projection * view;
}

std::array<glm::mat4, 6> ivf::pointShadowMatrices(const glm::vec3 &position, float nearDist, float farDist,
                                                  float tanHalfFov)
{
    static constexpr std::array<glm::vec3, 6> directions{glm::vec3(1, 0, 0),  glm::vec3(-1, 0, 0),
                                                          glm::vec3(0, 1, 0),  glm::vec3(0, -1, 0),
                                                          glm::vec3(0, 0, 1),  glm::vec3(0, 0, -1)};
    const glm::mat4 projection = glm::perspective(2.0f * std::atan(tanHalfFov), 1.0f, nearDist, farDist);
    std::array<glm::mat4, 6> matrices;
    for (std::size_t i = 0; i < 6; ++i)
        matrices[i] = projection * glm::lookAt(position, position + directions[i], upFor(directions[i]));
    return matrices;
}

glm::mat4 ivf::spotShadowMatrix(const glm::vec3 &position, const glm::vec3 &direction, float outerAngleDeg,
                                float nearDist, float farDist, float *tanHalfFov)
{
    const float angle = glm::radians(std::clamp(outerAngleDeg, 1.0f, 80.0f));
    const float tanHalf = std::tan(angle) * 1.05f;
    if (tanHalfFov != nullptr)
        *tanHalfFov = tanHalf;
    const glm::vec3 dir = glm::length(direction) > 0.0f ? glm::normalize(direction) : glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::mat4 projection = glm::perspective(2.0f * std::atan(tanHalf), 1.0f, nearDist, farDist);
    return projection * glm::lookAt(position, position + dir, upFor(dir));
}

int ivf::cubeFace(const glm::vec3 &v)
{
    const glm::vec3 a = glm::abs(v);
    if (a.x >= a.y && a.x >= a.z)
        return v.x > 0.0f ? 0 : 1;
    if (a.y >= a.z)
        return v.y > 0.0f ? 2 : 3;
    return v.z > 0.0f ? 4 : 5;
}
