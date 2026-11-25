/**
 * @file vis_vectors1.cpp
 * @brief ivfvis: glyphs and streamlines of a vector field (a magnetic dipole).
 * @ingroup general_examples
 *
 * Demonstrates vector field visualisation with ivfvis:
 *  - an ivfvis::VectorField given by a function (the field of a magnetic dipole)
 *  - ivfvis::Glyphs: arrows on a plane, of constant length and coloured by field strength on
 *    a logarithmic colormap (the strength falls off with the cube of the distance)
 *  - ivfvis::Streamlines: field lines traced both ways from seeds on rings around the
 *    dipole, drawn as tubes (or lines) coloured by the field strength
 *  - picking: click an arrow to see its position and vector
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#include <cmath>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

namespace {

/** Field of a magnetic dipole along y at the origin; zero inside a small core */
glm::vec3 dipole(const glm::vec3 &p)
{
    const float r = glm::length(p);
    if (r < 0.2f)
        return glm::vec3(0.0f);
    const glm::vec3 m(0.0f, 1.0f, 0.0f);
    const glm::vec3 n = p / r;
    return (3.0f * glm::dot(m, n) * n - m) / (r * r * r);
}

} // namespace

class VectorsWindow : public GLFWSceneWindow {
private:
    ivfvis::VectorFieldPtr m_field;
    ivfvis::GlyphsPtr m_glyphs;
    ivfvis::StreamlinesPtr m_lines;
    ivfvis::ColormapPtr m_colormap;
    double m_pressX{0.0}, m_pressY{0.0};
    int m_picked{-1};

    // UI state
    bool m_showGlyphs{true};
    bool m_showLines{true};
    float m_glyphScale{0.2f};
    int m_glyphScaling{1}; // constant
    int m_glyphShape{0};
    bool m_tubes{true};
    float m_tubeRadius{0.014f};
    int m_seedsPerRing{12};

public:
    VectorsWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<VectorsWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<VectorsWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.4f, -1.0f, -0.5f});
        sun->setDiffuseColor({0.8f, 0.8f, 0.8f});
        sun->setSpecularColor({0.4f, 0.4f, 0.4f});
        sun->setAmbientColor({0.35f, 0.35f, 0.35f});
        lights->apply();

        m_field = ivfvis::VectorField::fromFunction(dipole, BoundingBox({-2, -2, -2}, {2, 2, 2}));

        // Strength from about 0.1 at the edge to 100 near the core: a log scale
        m_colormap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Plasma);
        m_colormap->setScale(ivfvis::ColormapScale::Log);
        m_colormap->setRange(0.05f, 50.0f);

        // Arrows on the plane z = 0
        std::vector<glm::vec3> plane;
        for (int j = 0; j < 17; ++j)
            for (int i = 0; i < 17; ++i)
                plane.emplace_back(-2.0f + 0.25f * static_cast<float>(i), -2.0f + 0.25f * static_cast<float>(j), 0.0f);
        m_glyphs = ivfvis::Glyphs::create(m_field);
        m_glyphs->setPositions(std::move(plane));
        m_glyphs->setAnchor(ivfvis::GlyphAnchor::Center);
        m_glyphs->setColormap(m_colormap);
        add(m_glyphs);

        // Field lines through rings around the equator
        m_lines = ivfvis::Streamlines::create(m_field, {});
        m_lines->setColormap(m_colormap);
        add(m_lines);

        auto bar = ivfvis::ColorBar::create(m_colormap);
        bar->setTitle("|B|");
        add(bar);

        applyGlyphs();
        applyLines();

        cameraManipulator()->setCameraPosition({3.6f, 2.2f, 4.8f});
        cameraManipulator()->setCameraTarget({0.0f, 0.0f, 0.0f});
        return 0;
    }

    void applyGlyphs()
    {
        m_glyphs->setVisible(m_showGlyphs);
        m_glyphs->setShape(static_cast<ivfvis::GlyphShape>(m_glyphShape));
        m_glyphs->setScaling(m_glyphScaling == 0 ? ivfvis::GlyphScaling::Magnitude : ivfvis::GlyphScaling::Constant);
        m_glyphs->setScale(m_glyphScaling == 0 ? m_glyphScale * 0.3f : m_glyphScale);
    }

    void applyLines()
    {
        std::vector<glm::vec3> seeds;
        for (float radius : {0.45f, 0.7f, 1.0f, 1.4f})
        {
            auto ring = ivfvis::seedCircle({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, radius, m_seedsPerRing);
            seeds.insert(seeds.end(), ring.begin(), ring.end());
        }
        m_lines->setVisible(m_showLines);
        m_lines->setTubes(m_tubes);
        m_lines->setTubeRadius(m_tubeRadius);
        m_lines->setSeeds(std::move(seeds));
    }

    void onMouseButton(int button, int action, int /*mods*/) override
    {
        if (button != GLFW_MOUSE_BUTTON_LEFT)
            return;
        if (action == GLFW_PRESS)
        {
            m_pressX = mouseX();
            m_pressY = mouseY();
            return;
        }
        if (std::abs(mouseX() - m_pressX) + std::abs(mouseY() - m_pressY) > 4.0)
            return;
        const RayHit hit = pick(mouseX(), mouseY());
        m_picked = hit.node == m_glyphs.get() ? m_glyphs->glyphIndex(hit.triangle) : -1;
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({12, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({330, 0}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Vector field");

        ImGui::SeparatorText("Glyphs");
        bool glyphs = ImGui::Checkbox("Show##glyphs", &m_showGlyphs);
        const char *shapes[] = {"Arrow", "Cone", "Line"};
        glyphs |= ImGui::Combo("Shape", &m_glyphShape, shapes, 3);
        const char *scalings[] = {"Magnitude", "Constant"};
        glyphs |= ImGui::Combo("Length", &m_glyphScaling, scalings, 2);
        glyphs |= ImGui::SliderFloat("Scale", &m_glyphScale, 0.05f, 0.5f, "%.2f");
        if (glyphs)
            applyGlyphs();
        ImGui::TextDisabled("%zu arrows", m_glyphs->glyphCount());

        ImGui::SeparatorText("Field lines");
        bool lines = ImGui::Checkbox("Show##lines", &m_showLines);
        lines |= ImGui::Checkbox("Tubes", &m_tubes);
        lines |= ImGui::SliderFloat("Radius", &m_tubeRadius, 0.004f, 0.04f, "%.3f");
        lines |= ImGui::SliderInt("Per ring", &m_seedsPerRing, 2, 32);
        if (lines)
            applyLines();
        ImGui::TextDisabled("%zu lines", m_lines->lines().size());

        ImGui::SeparatorText("Picked arrow (click)");
        if (m_picked >= 0)
        {
            const auto i = static_cast<std::size_t>(m_picked);
            const glm::vec3 p = m_glyphs->glyphPosition(i), v = m_glyphs->glyphVector(i);
            ImGui::Text("Position  %.2f, %.2f, %.2f", p.x, p.y, p.z);
            ImGui::Text("Field     %.3f, %.3f, %.3f", v.x, v.y, v.z);
            ImGui::Text("|B|       %.3f", glm::length(v));
        }
        else
            ImGui::TextDisabled("none");
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = VectorsWindow::create(1280, 720, "ivfvis: glyphs and field lines");
    app->addWindow(win);
    return app->loop();
}
