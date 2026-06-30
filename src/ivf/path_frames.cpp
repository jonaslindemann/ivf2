#include <ivf/path_frames.h>

#include <ivfmath/spline.h>

#include <cmath>

using namespace ivf;

namespace {

constexpr float kEps = 1e-8f;

float length2(const glm::vec3 &v)
{
    return glm::dot(v, v);
}

// Pick an axis least aligned with t, used to seed an initial normal.
glm::vec3 pickReferenceAxis(const glm::vec3 &t)
{
    glm::vec3 a = glm::abs(t);
    if (a.x <= a.y && a.x <= a.z)
        return glm::vec3(1.0f, 0.0f, 0.0f);
    if (a.y <= a.z)
        return glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::vec3(0.0f, 0.0f, 1.0f);
}

// Rotate vector v around unit axis k by angle (Rodrigues' rotation formula).
glm::vec3 rotateAroundAxis(const glm::vec3 &v, const glm::vec3 &k, float angle)
{
    float c = std::cos(angle);
    float s = std::sin(angle);
    return v * c + glm::cross(k, v) * s + k * glm::dot(k, v) * (1.0f - c);
}

// Compute tangents from a list of station positions using central differences.
void computeTangents(std::vector<PathFrame> &frames, bool closed)
{
    int n = static_cast<int>(frames.size());
    if (n == 0)
        return;

    if (n == 1)
    {
        frames[0].tangent = glm::vec3(0.0f, 0.0f, 1.0f);
        return;
    }

    for (int i = 0; i < n; ++i)
    {
        glm::vec3 t;
        if (closed)
        {
            glm::vec3 prev = frames[(i - 1 + n) % n].position;
            glm::vec3 next = frames[(i + 1) % n].position;
            t = next - prev;
        }
        else if (i == 0)
            t = frames[1].position - frames[0].position;
        else if (i == n - 1)
            t = frames[n - 1].position - frames[n - 2].position;
        else
            t = frames[i + 1].position - frames[i - 1].position;

        if (length2(t) > kEps)
            frames[i].tangent = glm::normalize(t);
        else if (i > 0)
            frames[i].tangent = frames[i - 1].tangent;
    }
}

// Cumulative arc length along the stations -> frame.v.
void computeArcLength(std::vector<PathFrame> &frames)
{
    if (frames.empty())
        return;

    frames[0].v = 0.0f;
    for (std::size_t i = 1; i < frames.size(); ++i)
        frames[i].v = frames[i - 1].v + glm::length(frames[i].position - frames[i - 1].position);
}

// Build stations (position + tangent) for a piecewise-linear spine, honouring the join style.
std::vector<PathFrame> buildPolylineStations(const std::vector<glm::vec3> &pts, JoinStyle join, bool closed,
                                             float cornerRadius, int cornerSegments)
{
    std::vector<PathFrame> stations;
    int n = static_cast<int>(pts.size());
    if (n < 2)
    {
        for (const auto &p : pts)
        {
            PathFrame f;
            f.position = p;
            stations.push_back(f);
        }
        return stations;
    }

    auto segDir = [&](int a, int b) { return glm::normalize(pts[b] - pts[a]); };
    auto pushStation = [&](const glm::vec3 &pos, const glm::vec3 &tan) {
        PathFrame f;
        f.position = pos;
        f.tangent = tan;
        stations.push_back(f);
    };

    // Closed polylines only support mitered (Angle) joins for now.
    if (closed)
    {
        for (int i = 0; i < n; ++i)
        {
            glm::vec3 dIn = segDir((i - 1 + n) % n, i);
            glm::vec3 dOut = segDir(i, (i + 1) % n);
            glm::vec3 t = dIn + dOut;
            if (length2(t) < kEps)
                t = dOut;
            pushStation(pts[i], glm::normalize(t));
        }
        return stations;
    }

    if (join == JoinStyle::Angle)
    {
        for (int i = 0; i < n; ++i)
        {
            glm::vec3 t;
            if (i == 0)
                t = segDir(0, 1);
            else if (i == n - 1)
                t = segDir(n - 2, n - 1);
            else
            {
                glm::vec3 dIn = segDir(i - 1, i);
                glm::vec3 dOut = segDir(i, i + 1);
                t = dIn + dOut;
                if (length2(t) < kEps)
                    t = dOut;
                t = glm::normalize(t);
            }
            pushStation(pts[i], t);
        }
        return stations;
    }

    // Raw / Cut / Round all keep each segment perpendicular to its own direction; they differ in
    // how the corner between two segments is filled.
    pushStation(pts[0], segDir(0, 1));

    for (int i = 1; i < n - 1; ++i)
    {
        glm::vec3 dIn = segDir(i - 1, i);
        glm::vec3 dOut = segDir(i, i + 1);
        glm::vec3 corner = pts[i];

        if (join == JoinStyle::Raw)
        {
            // Two coincident sections at the corner, each perpendicular to its segment.
            pushStation(corner, dIn);
            pushStation(corner, dOut);
        }
        else
        {
            // Cut / Round: pull back along each segment by a corner offset.
            float inLen = glm::length(pts[i] - pts[i - 1]);
            float outLen = glm::length(pts[i + 1] - pts[i]);
            float r = cornerRadius;
            r = std::min(r, 0.49f * inLen);
            r = std::min(r, 0.49f * outLen);
            if (r <= kEps)
                r = 0.0f;

            glm::vec3 pA = corner - dIn * r;
            glm::vec3 pB = corner + dOut * r;

            pushStation(pA, dIn);

            if (join == JoinStyle::Round && r > 0.0f && cornerSegments > 1)
            {
                // Quadratic Bezier through pA, corner, pB; tangent interpolated linearly.
                for (int s = 1; s < cornerSegments; ++s)
                {
                    float u = static_cast<float>(s) / static_cast<float>(cornerSegments);
                    float omu = 1.0f - u;
                    glm::vec3 pos = omu * omu * pA + 2.0f * omu * u * corner + u * u * pB;
                    glm::vec3 tan = glm::normalize(glm::mix(dIn, dOut, u));
                    pushStation(pos, tan);
                }
            }

            pushStation(pB, dOut);
        }
    }

    pushStation(pts[n - 1], segDir(n - 2, n - 1));
    return stations;
}

} // namespace

std::vector<glm::vec3> ivf::sampleSpine(const std::vector<glm::vec3> &controlPoints, SpineInterp interp, int samples)
{
    if (interp == SpineInterp::Polyline || controlPoints.size() < 2)
        return controlPoints;

    if (samples < 2)
        samples = 2;

    auto spline = ivfmath::Spline::create();
    for (const auto &p : controlPoints)
        spline->addPoint(p);

    std::vector<glm::vec3> result;
    result.reserve(samples);

    float tMax = static_cast<float>(controlPoints.size() - 1);
    for (int i = 0; i < samples; ++i)
    {
        float t = tMax * static_cast<float>(i) / static_cast<float>(samples - 1);
        result.push_back(spline->positionByT(t));
    }

    return result;
}

void ivf::assignFrameOrientations(std::vector<PathFrame> &frames, FrameMethod frameMethod, const glm::vec3 &upHint,
                                  bool closed)
{
    int n = static_cast<int>(frames.size());
    if (n == 0)
        return;

    // Seed the initial normal orthogonal to the first tangent.
    glm::vec3 t0 = frames[0].tangent;
    glm::vec3 up = upHint;
    if (length2(up) < kEps)
        up = pickReferenceAxis(t0);

    glm::vec3 n0 = up - glm::dot(up, t0) * t0;
    if (length2(n0) < kEps)
    {
        up = pickReferenceAxis(t0);
        n0 = up - glm::dot(up, t0) * t0;
    }
    n0 = glm::normalize(n0);

    frames[0].normal = n0;
    frames[0].binormal = glm::normalize(glm::cross(t0, n0));

    for (int i = 1; i < n; ++i)
    {
        glm::vec3 ti = frames[i].tangent;

        if (frameMethod == FrameMethod::FixedUp)
        {
            glm::vec3 nn = up - glm::dot(up, ti) * ti;
            if (length2(nn) < kEps)
                nn = frames[i - 1].normal - glm::dot(frames[i - 1].normal, ti) * ti;
            frames[i].normal = glm::normalize(nn);
            frames[i].binormal = glm::normalize(glm::cross(ti, frames[i].normal));
            continue;
        }

        // Double-reflection rotation-minimizing frame (used for RMF and as a robust
        // fallback for Frenet on (near-)straight runs).
        const PathFrame &prev = frames[i - 1];
        glm::vec3 v1 = frames[i].position - prev.position;
        float c1 = length2(v1);

        glm::vec3 rL, tL;
        if (c1 < kEps)
        {
            rL = prev.normal;
            tL = prev.tangent;
        }
        else
        {
            rL = prev.normal - (2.0f / c1) * glm::dot(v1, prev.normal) * v1;
            tL = prev.tangent - (2.0f / c1) * glm::dot(v1, prev.tangent) * v1;
        }

        glm::vec3 v2 = ti - tL;
        float c2 = length2(v2);
        glm::vec3 nNew = (c2 < kEps) ? rL : rL - (2.0f / c2) * glm::dot(v2, rL) * v2;

        // Re-orthogonalize against the tangent to avoid drift.
        nNew = nNew - glm::dot(nNew, ti) * ti;
        if (length2(nNew) < kEps)
            nNew = prev.normal - glm::dot(prev.normal, ti) * ti;
        nNew = glm::normalize(nNew);

        frames[i].normal = nNew;
        frames[i].binormal = glm::normalize(glm::cross(ti, nNew));
    }

    // Distribute residual twist for closed loops so the seam matches.
    if (closed && n > 2 && frameMethod != FrameMethod::FixedUp)
    {
        // Propagate one more reflection from the last frame back to the first.
        const PathFrame &last = frames[n - 1];
        glm::vec3 v1 = frames[0].position - last.position;
        float c1 = length2(v1);

        glm::vec3 rL = last.normal;
        glm::vec3 tL = last.tangent;
        if (c1 >= kEps)
        {
            rL = last.normal - (2.0f / c1) * glm::dot(v1, last.normal) * v1;
            tL = last.tangent - (2.0f / c1) * glm::dot(v1, last.tangent) * v1;
        }

        glm::vec3 v2 = frames[0].tangent - tL;
        float c2 = length2(v2);
        glm::vec3 nClose = (c2 < kEps) ? rL : rL - (2.0f / c2) * glm::dot(v2, rL) * v2;
        nClose = nClose - glm::dot(nClose, frames[0].tangent) * frames[0].tangent;
        if (length2(nClose) > kEps)
        {
            nClose = glm::normalize(nClose);

            // Signed drift angle around the first tangent.
            glm::vec3 a = frames[0].tangent;
            float s = glm::dot(glm::cross(nClose, frames[0].normal), a);
            float c = glm::dot(nClose, frames[0].normal);
            float drift = std::atan2(s, c);

            for (int i = 0; i < n; ++i)
            {
                float theta = -drift * static_cast<float>(i) / static_cast<float>(n);
                glm::vec3 axis = frames[i].tangent;
                frames[i].normal = glm::normalize(rotateAroundAxis(frames[i].normal, axis, theta));
                frames[i].binormal = glm::normalize(glm::cross(axis, frames[i].normal));
            }
        }
    }
}

std::vector<PathFrame> ivf::buildPathFrames(const std::vector<glm::vec3> &controlPoints, SpineInterp interp,
                                            JoinStyle join, int samples, FrameMethod frameMethod,
                                            const glm::vec3 &upHint, bool closed, float cornerRadius,
                                            int cornerSegments)
{
    std::vector<PathFrame> frames;

    if (controlPoints.size() < 2)
        return frames;

    if (interp == SpineInterp::CatmullRom)
    {
        std::vector<glm::vec3> positions = sampleSpine(controlPoints, interp, samples);
        frames.reserve(positions.size());
        for (const auto &p : positions)
        {
            PathFrame f;
            f.position = p;
            frames.push_back(f);
        }
        computeTangents(frames, closed);
    }
    else
    {
        frames = buildPolylineStations(controlPoints, join, closed, cornerRadius, cornerSegments);
        // Tangents are already set by the station builder; ensure they are unit length.
        for (auto &f : frames)
            if (length2(f.tangent) > kEps)
                f.tangent = glm::normalize(f.tangent);
    }

    computeArcLength(frames);
    assignFrameOrientations(frames, frameMethod, upHint, closed);

    return frames;
}
