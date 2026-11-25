/**
 * @file vis_colormaps1.cpp
 * @brief ivfvis: colormaps, vertex colouring by scalar values and a colour bar.
 * @ingroup general_examples
 *
 * Demonstrates the foundation of the ivfvis module:
 *  - the built-in colormaps (viridis, magma, inferno, plasma, cividis, turbo, coolwarm, gray),
 *    each colouring the same height field through ivfvis::colorVertices()
 *  - an ivfvis::ColorBar for the selected surface (click a surface to select it), which
 *    follows the colormap's range, scale, reversal and steps
 *  - an ivfvis::UniformGrid holding the height values, sampled at the mesh vertices
 */

#include <imgui.h>
#include <ivf/extrusion_builder.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#include <array>
#include <cmath>
#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

namespace {

/** Height of the landscape at (x, z): a ring-shaped wave in a Gaussian bump, between 0 and 1 */
float landscape(float x, float z)
{
    const float r = std::sqrt(x * x + z * z);
    const float hill = std::exp(-0.35f * r * r);
    const float bump = std::exp(-6.0f * ((x - 0.9f) * (x - 0.9f) + (z + 0.6f) * (z + 0.6f)));
    return std::clamp(0.02f + 0.75f * hill * (0.55f + 0.45f * std::cos(3.5f * r)) + 0.3f * bump, 0.0f, 1.0f);
}

/** A surface over [-1.5, 1.5]^2 with smooth normals; y = height * scale */
MeshNodePtr makeSurface(int n, float scale)
{
    const float size = 3.0f;
    MeshData data;
    auto at = [&](int i, int j) {
        const float x = -size * 0.5f + size * static_cast<float>(i) / static_cast<float>(n - 1);
        const float z = -size * 0.5f + size * static_cast<float>(j) / static_cast<float>(n - 1);
        return glm::vec3(x, landscape(x, z) * scale, z);
    };
    const float d = size / static_cast<float>(n - 1);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
        {
            const glm::vec3 p = at(i, j);
            const float hx = (landscape(p.x + d, p.z) - landscape(p.x - d, p.z)) * scale / (2.0f * d);
            const float hz = (landscape(p.x, p.z + d) - landscape(p.x, p.z - d)) * scale / (2.0f * d);
            data.positions.push_back(p);
            data.normals.push_back(glm::normalize(glm::vec3(-hx, 1.0f, -hz)));
            data.texCoords.emplace_back(0.0f);
            data.colors.emplace_back(1.0f);
        }
    for (int j = 0; j + 1 < n; ++j)
        for (int i = 0; i + 1 < n; ++i)
        {
            const auto a = static_cast<unsigned>(j * n + i), b = a + 1, c = a + static_cast<unsigned>(n), e = c + 1;
            data.indices.emplace_back(a, c, b);
            data.indices.emplace_back(b, c, e);
        }
    auto node = MeshNode::create();
    node->createFromMeshData(data);
    return node;
}

} // namespace

class ColormapWindow : public GLFWSceneWindow {
private:
    static constexpr float HeightScale = 0.9f;

    ivfvis::UniformGridPtr m_grid;
    std::vector<ivfvis::ColormapPtr> m_colormaps;
    std::vector<MeshNodePtr> m_surfaces;
    ivfvis::ColorBarPtr m_colorBar;
    int m_selected{0};

    // Settings applied to all colormaps
    float m_rangeMin{0.0f};
    float m_rangeMax{1.0f};
    bool m_reversed{false};
    int m_steps{0};
    bool m_log{false};
    int m_placement{0};
    double m_pressX{0.0}, m_pressY{0.0};

public:
    ColormapWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<ColormapWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<ColormapWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.3f, -1.0f, -0.6f});
        sun->setDiffuseColor({0.75f, 0.75f, 0.75f});
        sun->setSpecularColor({0.3f, 0.3f, 0.3f});
        sun->setAmbientColor({0.3f, 0.3f, 0.3f});
        lights->apply();

        // The data: heights on a 2D grid (one point along y), sampled at the vertices
        m_grid = ivfvis::UniformGrid::create({97, 1, 97}, BoundingBox({-1.5f, 0.0f, -1.5f}, {1.5f, 0.0f, 1.5f}));
        m_grid->fillScalars("height", [](const glm::vec3 &p) { return landscape(p.x, p.z); });

        // One surface per built-in colormap, four by two
        for (int i = 0; i < ivfvis::BuiltinColormapCount; ++i)
        {
            auto cmap = ivfvis::Colormap::create(static_cast<ivfvis::BuiltinColormap>(i));
            auto surface = makeSurface(97, HeightScale);
            auto material = Material::create();
            material->setSpecularColor({0.2f, 0.2f, 0.2f, 1.0f});
            material->setShininess(24.0f);
            surface->setMaterial(material);
            ivfvis::useVertexColors(*surface);
            surface->setPos({-5.4f + 3.6f * static_cast<float>(i % 4), 0.0f, i < 4 ? -1.9f : 1.9f});
            surface->setName(cmap->name());
            add(surface);

            auto label = TextNode::create();
            label->setText(cmap->name());
            label->setSize(0.32f);
            label->setColor({0.9f, 0.9f, 0.9f});
            label->setAlignX(TextAlignX::CENTER);
            label->setPos(surface->pos() + glm::vec3(0.0f, 0.02f, 1.75f));
            label->setEulerAngles(-90.0f, 0.0f, 0.0f);
            add(label);

            m_colormaps.push_back(cmap);
            m_surfaces.push_back(surface);
        }

        m_colorBar = ivfvis::ColorBar::create(m_colormaps[0]);
        m_colorBar->setTitle("Height");
        add(m_colorBar);

        applySettings();

        setSelectionEnabled(true);
        setSelectionMethod(SelectionMethod::Ray);

        cameraManipulator()->setCameraPosition({0.9f, 8.6f, 9.4f});
        cameraManipulator()->setCameraTarget({0.9f, -0.5f, 0.5f});
        return 0;
    }

    /** Apply the UI settings to all colormaps and recolour the surfaces */
    void applySettings()
    {
        for (std::size_t i = 0; i < m_colormaps.size(); ++i)
        {
            auto &cmap = m_colormaps[i];
            cmap->setScale(m_log ? ivfvis::ColormapScale::Log : ivfvis::ColormapScale::Linear);
            cmap->setRange(m_log ? std::max(m_rangeMin, 0.01f) : m_rangeMin, m_rangeMax);
            cmap->setReversed(m_reversed);
            cmap->setSteps(m_steps);
            ivfvis::colorVertices(*m_surfaces[i], *m_grid, "height", *cmap);
        }
        m_colorBar->setColormap(m_colormaps[static_cast<std::size_t>(m_selected)]);
        m_colorBar->setPlacement(static_cast<ivfvis::ColorBarPlacement>(m_placement));
    }

    void onMouseButton(int button, int action, int /*mods*/) override
    {
        // Click (without dragging the camera) selects the surface whose colormap the bar shows
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
        if (auto hit = pick(mouseX(), mouseY()))
            for (std::size_t i = 0; i < m_surfaces.size(); ++i)
                if (hit.node == m_surfaces[i].get())
                {
                    m_selected = static_cast<int>(i);
                    m_colorBar->setColormap(m_colormaps[i]);
                }
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({12, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({330, 0}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Colormaps");
        bool changed = false;
        ImGui::Text("Colour bar: %s (click a surface)", m_colormaps[static_cast<std::size_t>(m_selected)]->name().c_str());
        changed |= ImGui::SliderFloat("Min", &m_rangeMin, 0.0f, 1.0f, "%.2f");
        changed |= ImGui::SliderFloat("Max", &m_rangeMax, 0.0f, 1.0f, "%.2f");
        changed |= ImGui::Checkbox("Reversed", &m_reversed);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("Log scale", &m_log);
        changed |= ImGui::SliderInt("Steps", &m_steps, 0, 16, m_steps == 0 ? "continuous" : "%d");
        const char *placements[] = {"Right", "Left", "Bottom", "Top"};
        changed |= ImGui::Combo("Bar", &m_placement, placements, 4);
        if (changed)
        {
            if (m_rangeMax <= m_rangeMin)
                m_rangeMax = std::min(m_rangeMin + 0.01f, 1.0f);
            applySettings();
        }
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = ColormapWindow::create(1280, 720, "ivfvis: colormaps");
    app->addWindow(win);
    return app->loop();
}
