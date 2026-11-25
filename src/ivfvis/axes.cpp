#include <ivfvis/axes.h>

#include <ivfvis/ticks.h>

#include <ivf/draw_context.h>
#include <ivf/material.h>
#include <ivf/mesh.h>
#include <ivf/mesh_node.h>
#include <ivf/overlay_node.h>
#include <ivf/render_queue.h>
#include <ivf/text_node.h>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace ivfvis;

namespace {

/** True if the face of @p box on side @p hi of @p axis faces the camera at @p eye */
bool faceFront(const ivf::BoundingBox &box, int axis, bool hi, const glm::vec3 &eye)
{
    return hi ? eye[axis] > box.max()[axis] : eye[axis] < box.min()[axis];
}

/** Start and end of edge @p edge parallel to @p axis */
std::pair<glm::vec3, glm::vec3> edgeLine(const ivf::BoundingBox &box, int axis, int edge)
{
    const int b = (axis + 1) % 3, c = (axis + 2) % 3;
    glm::vec3 p = box.min();
    p[b] = (edge & 1) ? box.max()[b] : box.min()[b];
    p[c] = (edge & 2) ? box.max()[c] : box.min()[c];
    glm::vec3 q = p;
    p[axis] = box.min()[axis];
    q[axis] = box.max()[axis];
    return {p, q};
}

/** Unit direction pointing away from the box at edge @p edge of @p axis */
glm::vec3 edgeOutward(int axis, int edge)
{
    const int b = (axis + 1) % 3, c = (axis + 2) % 3;
    glm::vec3 d(0.0f);
    d[b] = (edge & 1) ? 1.0f : -1.0f;
    d[c] = (edge & 2) ? 1.0f : -1.0f;
    return glm::normalize(d);
}

/** Camera position in the coordinates of a node drawn with @p world */
glm::vec3 eyeIn(const glm::mat4 &world, const glm::mat4 &view)
{
    const glm::vec3 eye = glm::vec3(glm::inverse(view)[3]);
    return glm::vec3(glm::inverse(world) * glm::vec4(eye, 1.0f));
}

std::shared_ptr<ivf::Material> lineMaterial(const glm::vec3 &color)
{
    auto material = ivf::Material::create();
    material->setUseLighting(false);
    material->setDiffuseColor(glm::vec4(color, 1.0f));
    return material;
}

void setLines(ivf::MeshNode &node, const std::vector<glm::vec3> &lines)
{
    node.clear();
    if (lines.empty())
        return;
    node.newMesh(static_cast<int>(lines.size()), 0, GL_LINES);
    auto mesh = node.currentMesh();
    mesh->setGenerateNormals(false);
    mesh->begin(GL_LINES);
    for (const glm::vec3 &p : lines)
    {
        mesh->normal3f(0.0f, 0.0f, 1.0f);
        mesh->vertex3f(p);
    }
    mesh->end();
    node.updateBoundingBox();
}

/** A wall of the box, drawn only while it faces away from the camera */
class Wall : public ivf::MeshNode {
public:
    Wall(int axis, bool hi) : m_axis(axis), m_hi(hi)
    {
        setCastShadows(false);
        setName("Axes3D wall");
    }
    ivf::BoundingBox box;

protected:
    void doDrawPass(const ivf::DrawContext &context) override
    {
        if (context.pass != ivf::RenderPass::Main)
            return;
        if (faceFront(box, m_axis, m_hi, eyeIn(context.item.parentWorld, context.view)))
            return;
        MeshNode::doDrawPass(context);
    }

private:
    int m_axis;
    bool m_hi;
};

/** An edge with tick marks, drawn only while it is the axis's labelled edge */
class TickEdge : public ivf::MeshNode {
public:
    TickEdge(int axis, int edge) : m_axis(axis), m_edge(edge)
    {
        setCastShadows(false);
        setName("Axes3D edge");
    }
    ivf::BoundingBox box;

protected:
    void doDrawPass(const ivf::DrawContext &context) override
    {
        if (context.pass != ivf::RenderPass::Main)
            return;
        const glm::mat4 &world = context.item.parentWorld;
        const int chosen = axisEdge(box, m_axis, eyeIn(world, context.view), context.projection * context.view * world);
        if (chosen != m_edge)
            return;
        MeshNode::doDrawPass(context);
    }

private:
    int m_axis;
    int m_edge;
};

/** Tick labels and titles, placed in screen space next to the labelled edges */
class Labels : public ivf::OverlayNode {
public:
    explicit Labels(const Axes3D &axes) : m_axes(axes)
    {
        setName("Axes3D labels");
    }

protected:
    void onLayout(int /*width*/, int /*height*/) override
    {
        clear();
        std::size_t used = 0;
        const ivf::BoundingBox box = m_axes.bounds();
        if (!box.isValid())
            return;
        const glm::vec3 eye = eyeIn(parentWorld(), sceneView());
        const glm::mat4 viewProjection = sceneProjection() * sceneView() * parentWorld();
        const float tick = m_axes.tickLength() * std::max({box.size().x, box.size().y, box.size().z});
        const int size = m_axes.textSize();
        const float capHeight = std::round(static_cast<float>(size) * 0.7f);

        for (int axis = 0; axis < 3; ++axis)
        {
            const int edge = axisEdge(box, axis, eye, viewProjection);
            if (edge < 0)
                continue;
            const auto [start, end] = edgeLine(box, axis, edge);
            const glm::vec3 outward = edgeOutward(axis, edge);

            // Screen direction away from the box at the middle of the edge
            const glm::vec3 middle = 0.5f * (start + end);
            const auto m0 = project(middle), m1 = project(middle + outward * tick * 4.0f);
            if (!m0 || !m1)
                continue;
            glm::vec2 dir = glm::vec2(*m1) - glm::vec2(*m0);
            dir = glm::length(dir) > 1e-3f ? glm::normalize(dir) : glm::vec2(0.0f, -1.0f);

            // Tick labels
            const std::vector<double> values = m_axes.ticks(axis);
            const double step = values.size() > 1 ? values[1] - values[0] : 0.0;
            float extent = 0.0f;
            for (double value : values)
            {
                glm::vec3 p = start;
                p[axis] = static_cast<float>(value);
                const auto s = project(p + outward * tick);
                if (!s)
                    continue;
                auto text = label(used++, formatTick(value, step), size);
                const float w = text->textWidth();
                const float along = 0.5f * (std::abs(dir.x) * w + std::abs(dir.y) * capHeight);
                extent = std::max(extent, 2.0f * along);
                const glm::vec2 centre = glm::vec2(*s) + dir * (6.0f + along);
                text->setPos({std::round(centre.x - 0.5f * w), std::round(centre.y - 0.5f * capHeight), 0.0f});
                add(text);
            }

            // Title beyond the labels
            const std::string &title = m_axes.title(axis);
            if (!title.empty())
            {
                auto text = label(used++, title, size + 2);
                const float w = text->textWidth();
                const float cap = std::round(static_cast<float>(size + 2) * 0.7f);
                const float along = 0.5f * (std::abs(dir.x) * w + std::abs(dir.y) * cap);
                const auto s = project(middle + outward * tick);
                if (!s)
                    continue;
                const glm::vec2 centre = glm::vec2(*s) + dir * (6.0f + extent + 10.0f + along);
                text->setPos({std::round(centre.x - 0.5f * w), std::round(centre.y - 0.5f * cap), 0.0f});
                add(text);
            }
        }
    }

private:
    ivf::TextNodePtr label(std::size_t index, const std::string &string, int size)
    {
        while (m_pool.size() <= index)
            m_pool.push_back(ivf::TextNode::create());
        auto &text = m_pool[index];
        text->setSize(static_cast<float>(size));
        text->setResolution(size);
        text->setColor(m_axes.textColor());
        text->setAlignX(ivf::TextAlignX::LEFT);
        text->setAlignY(ivf::TextAlignY::BOTTOM);
        if (text->text() != string)
            text->setText(string);
        return text;
    }

    const Axes3D &m_axes;
    std::vector<ivf::TextNodePtr> m_pool;
};

} // namespace

int ivfvis::axisEdge(const ivf::BoundingBox &box, int axis, const glm::vec3 &eye, const glm::mat4 &viewProjection)
{
    const int b = (axis + 1) % 3, c = (axis + 2) % 3;
    int best = -1;
    float bestScore = std::numeric_limits<float>::max();
    for (int edge = 0; edge < 4; ++edge)
    {
        // On the outline: exactly one of the edge's two faces faces the camera
        const int front = (faceFront(box, b, edge & 1, eye) ? 1 : 0) + (faceFront(box, c, (edge & 2) != 0, eye) ? 1 : 0);
        if (front != 1)
            continue;
        const auto [p, q] = edgeLine(box, axis, edge);
        const glm::vec4 clip = viewProjection * glm::vec4(0.5f * (p + q), 1.0f);
        if (clip.w <= 1e-6f)
            continue;
        const glm::vec2 ndc = glm::vec2(clip) / clip.w;
        const float score = axis == 1 ? ndc.x : ndc.y; // leftmost for y, lowest otherwise
        if (score < bestScore)
        {
            bestScore = score;
            best = edge;
        }
    }
    return best;
}

Axes3D::Axes3D(const ivf::BoundingBox &bounds) : m_bounds(bounds)
{
    setName("Axes3D");
    setCastShadows(false);
    // The children are built once a shared_ptr owns the axes (add() needs shared_from_this())
}

std::shared_ptr<Axes3D> Axes3D::create(const ivf::BoundingBox &bounds)
{
    auto axes = std::make_shared<Axes3D>(bounds);
    axes->rebuild();
    return axes;
}

void Axes3D::collect(ivf::RenderQueue &queue, const ivf::CollectState &state)
{
    if (!m_built)
        rebuild();
    CompositeNode::collect(queue, state);
}

void Axes3D::setBounds(const ivf::BoundingBox &bounds)
{
    m_bounds = bounds;
    rebuild();
}

ivf::BoundingBox Axes3D::bounds() const
{
    return m_bounds;
}

void Axes3D::setTitles(const std::string &x, const std::string &y, const std::string &z)
{
    m_titles = {x, y, z};
}

const std::string &Axes3D::title(int axis) const
{
    return m_titles[static_cast<std::size_t>(std::clamp(axis, 0, 2))];
}

void Axes3D::setTickCount(int count)
{
    m_tickCount = std::max(count, 2);
    rebuild();
}

int Axes3D::tickCount() const
{
    return m_tickCount;
}

std::vector<double> Axes3D::ticks(int axis) const
{
    if (!m_bounds.isValid())
        return {};
    const int a = std::clamp(axis, 0, 2);
    return niceTicks(m_bounds.min()[a], m_bounds.max()[a], m_tickCount);
}

void Axes3D::setGrid(bool grid)
{
    m_grid = grid;
    rebuild();
}

bool Axes3D::grid() const
{
    return m_grid;
}

void Axes3D::setTickLength(float fraction)
{
    m_tickLength = std::max(fraction, 0.0f);
    rebuild();
}

float Axes3D::tickLength() const
{
    return m_tickLength;
}

void Axes3D::setColor(const glm::vec3 &color)
{
    m_color = color;
    rebuild();
}

void Axes3D::setGridColor(const glm::vec3 &color)
{
    m_gridColor = color;
    rebuild();
}

void Axes3D::setTextColor(const glm::vec3 &color)
{
    m_textColor = color;
}

glm::vec3 Axes3D::textColor() const
{
    return m_textColor;
}

void Axes3D::setTextSize(int pixels)
{
    m_textSize = std::max(pixels, 6);
}

int Axes3D::textSize() const
{
    return m_textSize;
}

void Axes3D::rebuild()
{
    m_built = true;
    clear();
    if (!m_bounds.isValid())
        return;
    const ivf::BoundingBox &box = m_bounds;
    const glm::vec3 lo = box.min(), hi = box.max();
    const float tick = m_tickLength * std::max({box.size().x, box.size().y, box.size().z});
    std::array<std::vector<double>, 3> axisTicks{ticks(0), ticks(1), ticks(2)};

    // Walls: outline and grid lines
    auto gridMaterial = lineMaterial(m_gridColor);
    auto edgeMaterial = lineMaterial(m_color);
    for (int axis = 0; axis < 3; ++axis)
        for (bool side : {false, true})
        {
            const int u = (axis + 1) % 3, v = (axis + 2) % 3;
            const float w = side ? hi[axis] : lo[axis];
            auto point = [&](float pu, float pv) {
                glm::vec3 p(0.0f);
                p[axis] = w;
                p[u] = pu;
                p[v] = pv;
                return p;
            };
            std::vector<glm::vec3> lines;
            if (m_grid)
            {
                for (double t : axisTicks[static_cast<std::size_t>(u)])
                    if (t > lo[u] && t < hi[u])
                        lines.insert(lines.end(),
                                     {point(static_cast<float>(t), lo[v]), point(static_cast<float>(t), hi[v])});
                for (double t : axisTicks[static_cast<std::size_t>(v)])
                    if (t > lo[v] && t < hi[v])
                        lines.insert(lines.end(),
                                     {point(lo[u], static_cast<float>(t)), point(hi[u], static_cast<float>(t))});
            }
            auto wall = std::make_shared<Wall>(axis, side);
            wall->box = box;
            setLines(*wall, lines);
            wall->setMaterial(gridMaterial);
            add(wall);

            // Outline of the wall, in the edge colour
            auto outline = std::make_shared<Wall>(axis, side);
            outline->box = box;
            setLines(*outline, {point(lo[u], lo[v]), point(hi[u], lo[v]), point(hi[u], lo[v]), point(hi[u], hi[v]),
                                point(hi[u], hi[v]), point(lo[u], hi[v]), point(lo[u], hi[v]), point(lo[u], lo[v])});
            outline->setMaterial(edgeMaterial);
            add(outline);
        }

    // Edges with tick marks, one per axis is drawn
    for (int axis = 0; axis < 3; ++axis)
        for (int edge = 0; edge < 4; ++edge)
        {
            const auto [start, end] = edgeLine(box, axis, edge);
            const glm::vec3 outward = edgeOutward(axis, edge);
            std::vector<glm::vec3> lines{start, end};
            for (double t : axisTicks[static_cast<std::size_t>(axis)])
            {
                glm::vec3 p = start;
                p[axis] = static_cast<float>(t);
                lines.insert(lines.end(), {p, p + outward * tick});
            }
            auto node = std::make_shared<TickEdge>(axis, edge);
            node->box = box;
            setLines(*node, lines);
            node->setMaterial(edgeMaterial);
            add(node);
        }

    add(std::make_shared<Labels>(*this));
}
