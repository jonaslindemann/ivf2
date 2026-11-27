#include "engine.h"

#include <ivfphysics/debug_view.h>

#include <ivf/debug_draw.h>
#include <ivf/transform_node.h>

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace ivfphysics;

namespace {

/** Draws shape lines in body coordinates, mapped to the world. */
struct Painter {
    ivf::DebugDraw &draw;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec4 color;

    [[nodiscard]] glm::vec3 world(const glm::vec3 &p) const
    {
        return position + rotation * p;
    }

    void line(const glm::vec3 &a, const glm::vec3 &b) const
    {
        draw.line(world(a), world(b), color);
    }

    /** Arc around @p centre in the plane of the unit vectors @p u and @p v, from angle a0 to a1. */
    void arc(const glm::vec3 &centre, const glm::vec3 &u, const glm::vec3 &v, float radius, float a0, float a1,
             int segments) const
    {
        glm::vec3 previous = centre + radius * (std::cos(a0) * u + std::sin(a0) * v);
        for (int i = 1; i <= segments; ++i)
        {
            const float a = a0 + (a1 - a0) * static_cast<float>(i) / static_cast<float>(segments);
            const glm::vec3 p = centre + radius * (std::cos(a) * u + std::sin(a) * v);
            line(previous, p);
            previous = p;
        }
    }

    void box(const glm::vec3 &centre, const glm::vec3 &half) const
    {
        auto corner = [&](int i) {
            return centre + glm::vec3((i & 1) ? half.x : -half.x, (i & 2) ? half.y : -half.y, (i & 4) ? half.z : -half.z);
        };
        for (int i = 0; i < 8; ++i)
            for (int bit = 1; bit < 8; bit <<= 1)
                if (!(i & bit))
                    line(corner(i), corner(i | bit));
    }
};

void drawShape(const Shape &shape, const Painter &painter, const DebugViewOptions &options)
{
    constexpr glm::vec3 X(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 Y(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 Z(0.0f, 0.0f, 1.0f);
    const float pi = glm::pi<float>();
    const int n = std::max(options.circleSegments, 4);
    const auto &s = shape.impl();

    switch (s.type)
    {
    case ShapeType::Box:
        painter.box(glm::vec3(0.0f), s.halfExtents);
        break;
    case ShapeType::Sphere:
        painter.arc(glm::vec3(0.0f), X, Y, s.radius, 0.0f, 2.0f * pi, n);
        painter.arc(glm::vec3(0.0f), Y, Z, s.radius, 0.0f, 2.0f * pi, n);
        painter.arc(glm::vec3(0.0f), Z, X, s.radius, 0.0f, 2.0f * pi, n);
        break;
    case ShapeType::Capsule: {
        // Along the local Y axis; cap centres at +-height/2
        const glm::vec3 top(0.0f, 0.5f * s.height, 0.0f);
        const float r = s.radius;
        painter.arc(top, Z, X, r, 0.0f, 2.0f * pi, n);
        painter.arc(-top, Z, X, r, 0.0f, 2.0f * pi, n);
        painter.arc(top, X, Y, r, 0.0f, pi, n / 2);
        painter.arc(top, Z, Y, r, 0.0f, pi, n / 2);
        painter.arc(-top, X, -Y, r, 0.0f, pi, n / 2);
        painter.arc(-top, Z, -Y, r, 0.0f, pi, n / 2);
        for (const glm::vec3 &side : {X, -X, Z, -Z})
            painter.line(top + r * side, -top + r * side);
        break;
    }
    case ShapeType::ConvexHull:
        for (const auto &e : s.edges)
            painter.line(s.points[e.x], s.points[e.y]);
        break;
    case ShapeType::Compound:
        break; // drawn part by part
    case ShapeType::TriangleMesh:
        if (static_cast<int>(s.triangles.size()) > options.maxTriangles)
        {
            glm::vec3 lo(std::numeric_limits<float>::max());
            glm::vec3 hi(std::numeric_limits<float>::lowest());
            for (const auto &p : s.points)
            {
                lo = glm::min(lo, p);
                hi = glm::max(hi, p);
            }
            painter.box(0.5f * (lo + hi), 0.5f * (hi - lo));
            break;
        }
        for (const auto &t : s.triangles)
        {
            painter.line(s.points[t.x], s.points[t.y]);
            painter.line(s.points[t.y], s.points[t.z]);
            painter.line(s.points[t.z], s.points[t.x]);
        }
        break;
    }
}

} // namespace

void ivfphysics::drawDebug(const PhysicsWorld &world, ivf::DebugDraw &draw, const DebugViewOptions &options)
{
    for (const auto &body : world.bodies())
    {
        const auto &b = body->impl();
        if (!b.body || (b.desc.type == BodyType::Static && !options.staticBodies))
            continue;

        // Where the node is shown (interpolated); kinematic nodes may still carry Euler angles
        const glm::vec3 position = b.node->pos();
        const glm::quat rotation = detail::nodeRotation(*b.node);

        if (options.colliders)
        {
            glm::vec4 color = options.dynamicColor;
            if (b.desc.trigger)
                color = options.triggerColor;
            else if (b.desc.type == BodyType::Static)
                color = options.staticColor;
            else if (b.desc.type == BodyType::Kinematic)
                color = options.kinematicColor;
            else if (b.body->isSleeping())
                color = options.sleepingColor;

            for (const auto &placed : detail::placedShapes(*b.shape))
            {
                const Painter painter{draw, position + rotation * placed.position,
                                      glm::normalize(rotation * placed.rotation), color};
                drawShape(*placed.shape, painter, options);
            }
        }

        if (b.desc.type != BodyType::Dynamic)
            continue;
        const glm::vec3 com = body->centerOfMass();
        if (options.velocities)
        {
            const glm::vec3 v = body->linearVelocity() * options.velocityScale;
            if (glm::dot(v, v) > 1e-8f)
                draw.arrow(com, v, options.velocityColor);
        }
        if (options.centersOfMass)
            draw.axes(com, 0.2f);
    }

    if (options.joints)
    {
        for (const auto &joint : world.joints())
        {
            if (joint->isBroken())
                continue;
            const glm::vec3 anchor = joint->anchor();
            draw.sphere(anchor, 0.06f, options.jointColor, 0.0f, 8);
            draw.line(anchor, joint->body1()->centerOfMass(), options.jointColor);
            if (auto other = joint->body2())
                draw.line(anchor, other->centerOfMass(), options.jointColor);
            if (joint->type() == JointType::Hinge || joint->type() == JointType::Slider)
            {
                const glm::vec3 axis = 0.4f * joint->axis();
                draw.line(anchor - axis, anchor + axis, options.jointColor);
            }
        }
    }

    if (options.contacts)
    {
        const float c = options.contactSize;
        for (const auto &p : world.contactPoints())
        {
            draw.line(p.position - glm::vec3(c, 0.0f, 0.0f), p.position + glm::vec3(c, 0.0f, 0.0f), options.contactColor);
            draw.line(p.position - glm::vec3(0.0f, c, 0.0f), p.position + glm::vec3(0.0f, c, 0.0f), options.contactColor);
            draw.line(p.position - glm::vec3(0.0f, 0.0f, c), p.position + glm::vec3(0.0f, 0.0f, c), options.contactColor);
            draw.line(p.position, p.position + p.normal * options.normalLength, options.contactColor);
        }
    }
}

void ivfphysics::drawDebug(const DragController &drag, ivf::DebugDraw &draw, const glm::vec4 &color)
{
    if (!drag.isDragging())
        return;
    const glm::vec3 anchor = drag.anchor();
    const glm::vec3 target = drag.target();
    draw.line(anchor, target, color);
    draw.box(target, glm::vec3(0.04f), color);
    draw.sphere(anchor, 0.04f, color, 0.0f, 8);
}
