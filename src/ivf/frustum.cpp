#include <ivf/frustum.h>

using namespace ivf;

Frustum::Frustum(const glm::mat4 &m) : m_valid(true)
{
    // Rows of the matrix (glm is column-major: m[column][row])
    auto row = [&m](int r) { return glm::vec4(m[0][r], m[1][r], m[2][r], m[3][r]); };
    const glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);

    // Clip volume -w <= x, y, z <= w (Gribb/Hartmann)
    m_planes = {r3 + r0, r3 - r0,  // left, right
                r3 + r1, r3 - r1,  // bottom, top
                r3 + r2, r3 - r2}; // near, far

    for (auto &plane : m_planes)
    {
        const float len = glm::length(glm::vec3(plane));
        if (len > 0.0f)
            plane /= len;
    }
}

bool Frustum::intersects(const BoundingBox &box) const
{
    if (!m_valid || !box.isValid())
        return true;

    const glm::vec3 lo = box.min();
    const glm::vec3 hi = box.max();
    for (const auto &plane : m_planes)
    {
        // Corner furthest along the plane normal: if even it is outside, the box is
        const glm::vec3 p(plane.x >= 0.0f ? hi.x : lo.x, plane.y >= 0.0f ? hi.y : lo.y,
                          plane.z >= 0.0f ? hi.z : lo.z);
        if (glm::dot(glm::vec3(plane), p) + plane.w < 0.0f)
            return false;
    }
    return true;
}
