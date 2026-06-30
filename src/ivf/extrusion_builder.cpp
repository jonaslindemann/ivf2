#include <ivf/extrusion_builder.h>

#include <cmath>

using namespace ivf;

namespace {

constexpr float kEps = 1e-8f;

// Apply per-station scale + twist to a profile point.
glm::vec2 transformPoint(const glm::vec2 &p, const SectionTransform &st)
{
    glm::vec2 q(p.x * st.scale.x, p.y * st.scale.y);
    float c = std::cos(st.twist);
    float s = std::sin(st.twist);
    return glm::vec2(q.x * c - q.y * s, q.x * s + q.y * c);
}

// Apply the inverse-scale + twist to a profile normal (keeps it perpendicular under non-uniform
// scale), returning a unit 2D normal.
glm::vec2 transformNormal(const glm::vec2 &n, const SectionTransform &st)
{
    float sx = (std::abs(st.scale.x) > kEps) ? st.scale.x : 1.0f;
    float sy = (std::abs(st.scale.y) > kEps) ? st.scale.y : 1.0f;
    glm::vec2 q(n.x / sx, n.y / sy);
    float len = glm::length(q);
    if (len > kEps)
        q /= len;
    float c = std::cos(st.twist);
    float s = std::sin(st.twist);
    return glm::vec2(q.x * c - q.y * s, q.x * s + q.y * c);
}

glm::vec3 placePoint(const PathFrame &f, const glm::vec2 &q)
{
    return f.position + q.x * f.normal + q.y * f.binormal;
}

glm::vec3 worldNormal(const PathFrame &f, const glm::vec2 &n2)
{
    glm::vec3 n = n2.x * f.normal + n2.y * f.binormal;
    float len = glm::length(n);
    return (len > kEps) ? n / len : f.normal;
}

} // namespace

MeshData ExtrusionBuilder::build(const ExtrusionProfile &profile, const std::vector<PathFrame> &frames,
                                 const ExtrusionOptions &options)
{
    MeshData data;

    int S = static_cast<int>(frames.size());
    int M = static_cast<int>(profile.pointCount());
    if (S < 2 || M < 2)
        return data;

    const std::vector<glm::vec2> &P = profile.points();
    std::vector<glm::vec2> pn = profile.hasNormals() ? profile.normals() : profile.computeSmoothNormals();
    bool closedProfile = profile.closed();
    bool closedPath = options.closedPath;

    float totalV = frames.back().v;
    if (totalV < kEps)
        totalV = 1.0f;

    auto sectionAt = [&](int s) -> SectionTransform {
        if (!options.sectionFn)
            return SectionTransform{};
        float t = (S > 1) ? static_cast<float>(s) / static_cast<float>(S - 1) : 0.0f;
        return options.sectionFn(t);
    };

    auto vCoord = [&](const PathFrame &f) -> float {
        return options.texMode == TexMode::Normalized ? f.v / totalV : f.v;
    };

    int sEnd = closedPath ? S : S - 1;

    if (options.normalStyle == NormalStyle::Smooth)
    {
        int ringVerts = closedProfile ? M + 1 : M;

        // Cumulative profile arc length for the U coordinate.
        std::vector<float> ucum(ringVerts, 0.0f);
        for (int j = 1; j < ringVerts; ++j)
        {
            int a = (j - 1) % M;
            int b = j % M;
            ucum[j] = ucum[j - 1] + glm::length(P[b] - P[a]);
        }
        float perim = ucum[ringVerts - 1];
        if (perim < kEps)
            perim = 1.0f;

        for (int s = 0; s < S; ++s)
        {
            SectionTransform st = sectionAt(s);
            float v = vCoord(frames[s]);
            for (int j = 0; j < ringVerts; ++j)
            {
                int src = j % M;
                glm::vec2 q = transformPoint(P[src], st);
                glm::vec2 n2 = transformNormal(pn[src], st);
                data.positions.push_back(placePoint(frames[s], q));
                data.normals.push_back(worldNormal(frames[s], n2));
                data.texCoords.push_back(glm::vec2(ucum[j] / perim, v));
                data.colors.push_back(st.color);
            }
        }

        auto idx = [&](int s, int j) { return static_cast<unsigned>(s * ringVerts + j); };
        for (int s = 0; s < sEnd; ++s)
        {
            int s1 = (s + 1) % S;
            for (int j = 0; j < ringVerts - 1; ++j)
            {
                unsigned a = idx(s, j);
                unsigned b = idx(s, j + 1);
                unsigned c = idx(s1, j + 1);
                unsigned d = idx(s1, j);
                data.indices.push_back(glm::uvec3(a, b, c));
                data.indices.push_back(glm::uvec3(a, c, d));
            }
        }
    }
    else // Facet
    {
        int edgeCount = closedProfile ? M : M - 1;
        int ringVerts = edgeCount * 2;

        std::vector<float> edgeUStart(edgeCount, 0.0f);
        float acc = 0.0f;
        for (int e = 0; e < edgeCount; ++e)
        {
            edgeUStart[e] = acc;
            int a = e;
            int b = (e + 1) % M;
            acc += glm::length(P[b] - P[a]);
        }
        float perim = acc;
        if (perim < kEps)
            perim = 1.0f;

        for (int s = 0; s < S; ++s)
        {
            SectionTransform st = sectionAt(s);
            float v = vCoord(frames[s]);
            for (int e = 0; e < edgeCount; ++e)
            {
                int a = e;
                int b = (e + 1) % M;

                glm::vec2 edRaw = P[b] - P[a];
                glm::vec2 enRaw(edRaw.y, -edRaw.x);
                if (glm::dot(enRaw, pn[a] + pn[b]) < 0.0f)
                    enRaw = -enRaw;

                glm::vec2 en2 = transformNormal(enRaw, st);
                glm::vec3 enW = worldNormal(frames[s], en2);

                glm::vec2 qa = transformPoint(P[a], st);
                glm::vec2 qb = transformPoint(P[b], st);

                data.positions.push_back(placePoint(frames[s], qa));
                data.normals.push_back(enW);
                data.texCoords.push_back(glm::vec2(edgeUStart[e] / perim, v));
                data.colors.push_back(st.color);

                data.positions.push_back(placePoint(frames[s], qb));
                data.normals.push_back(enW);
                data.texCoords.push_back(glm::vec2((edgeUStart[e] + glm::length(edRaw)) / perim, v));
                data.colors.push_back(st.color);
            }
        }

        auto idx = [&](int s, int k) { return static_cast<unsigned>(s * ringVerts + k); };
        for (int s = 0; s < sEnd; ++s)
        {
            int s1 = (s + 1) % S;
            for (int e = 0; e < edgeCount; ++e)
            {
                unsigned a = idx(s, 2 * e);
                unsigned b = idx(s, 2 * e + 1);
                unsigned c = idx(s1, 2 * e + 1);
                unsigned d = idx(s1, 2 * e);
                data.indices.push_back(glm::uvec3(a, b, c));
                data.indices.push_back(glm::uvec3(a, c, d));
            }
        }
    }

    // Caps (only for closed profiles that are not closed loops).
    if (closedProfile && M >= 3 && !closedPath)
    {
        auto addCap = [&](const PathFrame &f, bool start) {
            SectionTransform st = sectionAt(start ? 0 : S - 1);
            glm::vec3 nrm = start ? -f.tangent : f.tangent;

            std::vector<glm::vec2> q(M);
            glm::vec2 c2(0.0f);
            for (int i = 0; i < M; ++i)
            {
                q[i] = transformPoint(P[i], st);
                c2 += q[i];
            }
            c2 /= static_cast<float>(M);

            float r = 0.0f;
            for (int i = 0; i < M; ++i)
                r = std::max(r, glm::length(q[i] - c2));
            if (r < kEps)
                r = 1.0f;

            unsigned base = static_cast<unsigned>(data.positions.size());

            data.positions.push_back(placePoint(f, c2));
            data.normals.push_back(nrm);
            data.texCoords.push_back(glm::vec2(0.5f, 0.5f));
            data.colors.push_back(st.color);

            for (int i = 0; i < M; ++i)
            {
                data.positions.push_back(placePoint(f, q[i]));
                data.normals.push_back(nrm);
                glm::vec2 uv = 0.5f + 0.5f * (q[i] - c2) / r;
                data.texCoords.push_back(uv);
                data.colors.push_back(st.color);
            }

            for (int i = 0; i < M; ++i)
            {
                unsigned center = base;
                unsigned a = base + 1 + i;
                unsigned b = base + 1 + ((i + 1) % M);
                if (start)
                    data.indices.push_back(glm::uvec3(center, b, a));
                else
                    data.indices.push_back(glm::uvec3(center, a, b));
            }
        };

        if (options.capStart)
            addCap(frames.front(), true);
        if (options.capEnd)
            addCap(frames.back(), false);
    }

    return data;
}
