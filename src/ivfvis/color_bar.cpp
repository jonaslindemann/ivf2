#include <ivfvis/color_bar.h>

#include <ivfvis/ticks.h>

#include <ivf/material.h>
#include <ivf/mesh.h>

#include <algorithm>
#include <cmath>

using namespace ivfvis;

namespace {

/** Ticks as positions along the bar (0..1) with labels */
struct Tick {
    float t;
    std::string label;
};

std::vector<Tick> makeTicks(const Colormap &cmap, const std::vector<double> &fixed, int count)
{
    const ValueRange range = cmap.range();
    const bool log = cmap.scale() == ColormapScale::Log && range.min > 0.0f && range.max > range.min;

    std::vector<double> values = fixed;
    double step = 0.0;
    if (values.empty())
    {
        if (log)
            values = logTicks(range.min, range.max);
        else
        {
            values = niceTicks(range.min, range.max, count);
            step = niceTickStep(range.min, range.max, count);
        }
    }
    if (step == 0.0 && !log)
        for (std::size_t i = 1; i < values.size(); ++i)
        {
            const double d = std::abs(values[i] - values[i - 1]);
            if (d > 0.0 && (step == 0.0 || d < step))
                step = d;
        }

    std::vector<Tick> ticks;
    for (double v : values)
    {
        const float t = cmap.normalize(static_cast<float>(v));
        if (!(t >= -1e-4f && t <= 1.0f + 1e-4f))
            continue;
        ticks.push_back({std::clamp(t, 0.0f, 1.0f), formatTick(v, log ? std::abs(v) : step)});
    }
    return ticks;
}

} // namespace

ColorBar::ColorBar(ColormapPtr colormap) : m_colormap(std::move(colormap))
{
    setName("ColorBar");

    m_bar = ivf::MeshNode::create();
    auto barMaterial = ivf::Material::create();
    barMaterial->setUseLighting(false);
    barMaterial->setUseVertexColor(true);
    m_bar->setMaterial(barMaterial);

    m_lines = ivf::MeshNode::create();
    auto lineMaterial = ivf::Material::create();
    lineMaterial->setUseLighting(false);
    m_lines->setMaterial(lineMaterial);
}

std::shared_ptr<ColorBar> ColorBar::create(ColormapPtr colormap)
{
    return std::make_shared<ColorBar>(std::move(colormap));
}

void ColorBar::setColormap(ColormapPtr colormap)
{
    m_colormap = std::move(colormap);
    ++m_settings;
}

ColormapPtr ColorBar::colormap() const
{
    return m_colormap;
}

void ColorBar::setTitle(const std::string &title)
{
    m_title = title;
    ++m_settings;
}

const std::string &ColorBar::title() const
{
    return m_title;
}

void ColorBar::setPlacement(ColorBarPlacement placement)
{
    m_placement = placement;
    ++m_settings;
}

ColorBarPlacement ColorBar::placement() const
{
    return m_placement;
}

void ColorBar::setLength(float fraction)
{
    m_length = std::clamp(fraction, 0.05f, 1.0f);
    ++m_settings;
}

float ColorBar::length() const
{
    return m_length;
}

void ColorBar::setThickness(int pixels)
{
    m_thickness = std::max(pixels, 1);
    ++m_settings;
}

int ColorBar::thickness() const
{
    return m_thickness;
}

void ColorBar::setMargin(int pixels)
{
    m_margin = std::max(pixels, 0);
    ++m_settings;
}

int ColorBar::margin() const
{
    return m_margin;
}

void ColorBar::setTickCount(int count)
{
    m_tickCount = std::max(count, 2);
    ++m_settings;
}

int ColorBar::tickCount() const
{
    return m_tickCount;
}

void ColorBar::setTicks(std::vector<double> ticks)
{
    m_ticks = std::move(ticks);
    ++m_settings;
}

void ColorBar::setTextSize(int pixels)
{
    m_textSize = std::max(pixels, 4);
    ++m_settings;
}

int ColorBar::textSize() const
{
    return m_textSize;
}

void ColorBar::setTextColor(const glm::vec3 &color)
{
    m_textColor = color;
    ++m_settings;
}

glm::vec3 ColorBar::textColor() const
{
    return m_textColor;
}

void ColorBar::setFrame(bool frame)
{
    m_frame = frame;
    ++m_settings;
}

bool ColorBar::frame() const
{
    return m_frame;
}

void ColorBar::onLayout(int width, int height)
{
    if (!m_colormap)
    {
        clear();
        return;
    }
    if (width == m_builtWidth && height == m_builtHeight && m_settings == m_builtSettings &&
        m_colormap.get() == m_builtFor && m_colormap->version() == m_builtColormap)
        return;
    rebuild(width, height);
    m_builtWidth = width;
    m_builtHeight = height;
    m_builtSettings = m_settings;
    m_builtFor = m_colormap.get();
    m_builtColormap = m_colormap->version();
}

ivf::TextNodePtr ColorBar::label(std::size_t index)
{
    while (m_labels.size() <= index)
        m_labels.push_back(ivf::TextNode::create());
    return m_labels[index];
}

void ColorBar::rebuild(int width, int height)
{
    const Colormap &cmap = *m_colormap;
    const bool vertical = m_placement == ColorBarPlacement::Right || m_placement == ColorBarPlacement::Left;
    const float w = static_cast<float>(width), h = static_cast<float>(height);
    const float length = std::round(m_length * (vertical ? h : w));
    const float thick = static_cast<float>(m_thickness);
    const float margin = static_cast<float>(m_margin);
    const float textSize = static_cast<float>(m_textSize);
    const float gap = 6.0f;      // between bar, ticks and labels
    const float tickLength = 4.0f;

    // Bar rectangle [x0, x1] x [y0, y1] in pixels
    float x0 = 0.0f, y0 = 0.0f;
    switch (m_placement)
    {
    case ColorBarPlacement::Right:
        x0 = w - margin - thick;
        y0 = std::round((h - length) * 0.5f);
        break;
    case ColorBarPlacement::Left:
        x0 = margin;
        y0 = std::round((h - length) * 0.5f);
        break;
    case ColorBarPlacement::Bottom:
        x0 = std::round((w - length) * 0.5f);
        y0 = margin + std::round(textSize) + gap + tickLength;
        break;
    case ColorBarPlacement::Top:
        x0 = std::round((w - length) * 0.5f);
        y0 = h - margin - thick;
        break;
    }
    const float x1 = vertical ? x0 + thick : x0 + length;
    const float y1 = vertical ? y0 + length : y0 + thick;

    // A point of the bar at position t along it and s across it (0..1)
    auto at = [&](float t, float s) {
        return vertical ? glm::vec3(x0 + s * thick, y0 + t * length, 0.0f)
                        : glm::vec3(x0 + t * length, y0 + s * thick, 0.0f);
    };

    // Colour bands: one per step, or a smooth gradient of many short segments
    const int segments = cmap.steps() > 0 ? cmap.steps() : 128;
    m_bar->clear();
    m_bar->newMesh(segments * 4, segments * 2, GL_TRIANGLES);
    auto bar = m_bar->mesh();
    bar->setGenerateNormals(false);
    bar->begin(GL_TRIANGLES);
    for (int i = 0; i < segments; ++i)
    {
        const float t0 = static_cast<float>(i) / static_cast<float>(segments);
        const float t1 = static_cast<float>(i + 1) / static_cast<float>(segments);
        glm::vec4 c0, c1;
        if (cmap.steps() > 0)
            c0 = c1 = cmap.colorAt((t0 + t1) * 0.5f);
        else
        {
            c0 = cmap.colorAt(t0);
            c1 = cmap.colorAt(t1);
        }
        const glm::vec3 corners[4] = {at(t0, 0.0f), at(t0, 1.0f), at(t1, 1.0f), at(t1, 0.0f)};
        const glm::vec4 colors[4] = {c0, c0, c1, c1};
        for (int k = 0; k < 4; ++k)
        {
            bar->color4f(colors[k].r, colors[k].g, colors[k].b, 1.0f);
            bar->normal3f(0.0f, 0.0f, 1.0f);
            bar->vertex3f(corners[k]);
        }
        const auto base = static_cast<GLuint>(i * 4);
        bar->index3i(base, base + 1, base + 2);
        bar->index3i(base, base + 2, base + 3);
    }
    bar->end();

    // Frame and tick marks (on pixel centres, so 1-pixel lines stay sharp)
    const std::vector<Tick> ticks = makeTicks(cmap, m_ticks, m_tickCount);
    std::vector<glm::vec3> lines;
    if (m_frame)
    {
        const glm::vec3 a(x0 - 0.5f, y0 - 0.5f, 0.0f), b(x1 + 0.5f, y0 - 0.5f, 0.0f);
        const glm::vec3 c(x1 + 0.5f, y1 + 0.5f, 0.0f), d(x0 - 0.5f, y1 + 0.5f, 0.0f);
        lines.insert(lines.end(), {a, b, b, c, c, d, d, a});
    }
    for (const Tick &tick : ticks)
    {
        if (vertical)
        {
            const float y = std::round(y0 + tick.t * length) + 0.5f;
            const float xs = m_placement == ColorBarPlacement::Right ? x0 - 0.5f : x1 + 0.5f;
            const float xe = m_placement == ColorBarPlacement::Right ? xs - tickLength : xs + tickLength;
            lines.insert(lines.end(), {glm::vec3(xs, y, 0.0f), glm::vec3(xe, y, 0.0f)});
        }
        else
        {
            const float x = std::round(x0 + tick.t * length) + 0.5f;
            lines.insert(lines.end(), {glm::vec3(x, y0 - 0.5f, 0.0f), glm::vec3(x, y0 - 0.5f - tickLength, 0.0f)});
        }
    }
    m_lines->clear();
    if (!lines.empty())
    {
        m_lines->newMesh(static_cast<int>(lines.size()), 0, GL_LINES);
        auto mesh = m_lines->mesh();
        mesh->setGenerateNormals(false);
        mesh->begin(GL_LINES);
        for (const glm::vec3 &p : lines)
        {
            mesh->normal3f(0.0f, 0.0f, 1.0f);
            mesh->vertex3f(p);
        }
        mesh->end();
    }
    m_lines->material()->setDiffuseColor(glm::vec4(m_textColor, 1.0f));

    // Children in drawing order
    clear();
    add(m_bar);
    add(m_lines);

    auto setupText = [&](const ivf::TextNodePtr &text, const std::string &string, int size) {
        text->setSize(static_cast<float>(size));
        text->setResolution(size);
        text->setColor(m_textColor);
        text->setText(string);
        text->setAlignX(ivf::TextAlignX::LEFT);
        text->setAlignY(ivf::TextAlignY::BOTTOM);
    };
    const float capHeight = std::round(textSize * 0.7f); // height of digits above the baseline

    std::size_t used = 0;
    for (const Tick &tick : ticks)
    {
        auto text = label(used++);
        setupText(text, tick.label, m_textSize);
        const float textWidth = text->textWidth();
        glm::vec3 pos(0.0f);
        if (vertical)
        {
            const float y = std::round(y0 + tick.t * length - capHeight * 0.5f);
            if (m_placement == ColorBarPlacement::Right)
                pos = glm::vec3(x0 - tickLength - gap - textWidth, y, 0.0f);
            else
                pos = glm::vec3(x1 + tickLength + gap, y, 0.0f);
        }
        else
        {
            const float x = std::round(x0 + tick.t * length - textWidth * 0.5f);
            pos = glm::vec3(x, y0 - tickLength - gap - capHeight, 0.0f);
        }
        text->setPos(pos);
        add(text);
    }

    if (!m_title.empty())
    {
        if (!m_titleNode)
            m_titleNode = ivf::TextNode::create();
        const int titleSize = m_textSize + 2;
        setupText(m_titleNode, m_title, titleSize);
        const float titleWidth = m_titleNode->textWidth();
        const float titleCap = std::round(static_cast<float>(titleSize) * 0.7f);
        glm::vec3 pos(0.0f);
        switch (m_placement)
        {
        case ColorBarPlacement::Right:
            pos = glm::vec3(std::min(x1 - titleWidth, std::round((x0 + x1 - titleWidth) * 0.5f)), y1 + gap + 4.0f,
                            0.0f);
            break;
        case ColorBarPlacement::Left:
            pos = glm::vec3(std::max(x0, std::round((x0 + x1 - titleWidth) * 0.5f)), y1 + gap + 4.0f, 0.0f);
            break;
        case ColorBarPlacement::Bottom:
        case ColorBarPlacement::Top:
            pos = glm::vec3(x0 - gap - 4.0f - titleWidth, std::round((y0 + y1 - titleCap) * 0.5f), 0.0f);
            break;
        }
        m_titleNode->setPos(pos);
        add(m_titleNode);
    }
}
