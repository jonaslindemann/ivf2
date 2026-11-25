/**
 * @file vis_axes1.cpp
 * @brief ivfvis: a surface plot with 3D axes, ticks, labels and grid walls.
 * @ingroup general_examples
 *
 * Demonstrates ivfvis::Axes3D:
 *  - axes fitted to the data's bounding box, with "nice" tick values in data units
 *  - grid lines on the walls facing away from the camera, ticks and labels on the outer
 *    edges; both follow the camera (rotate the view to see them move)
 *  - screen-space labels and axis titles, and a colour bar for the surface colours
 */

#include <imgui.h>
#include <ivf/extrusion_builder.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#include <cmath>
#include <string>

using namespace ivf;
using namespace ivfui;

namespace {

/** The function plotted: a damped ripple with a side peak */
float f(float x, float z)
{
    const float r = std::sqrt(x * x + z * z);
    return 1.4f * std::cos(2.2f * r) * std::exp(-0.35f * r) + 0.8f * std::exp(-((x - 1.6f) * (x - 1.6f) + (z + 1.2f) * (z + 1.2f)));
}

/** Surface over [-3, 3]^2 with smooth normals (y up) */
MeshNodePtr surface(int n)
{
    MeshData data;
    const float h = 6.0f / static_cast<float>(n - 1), d = 1e-3f;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
        {
            const float x = -3.0f + h * static_cast<float>(i), z = -3.0f + h * static_cast<float>(j);
            data.positions.emplace_back(x, f(x, z), z);
            const float fx = (f(x + d, z) - f(x - d, z)) / (2.0f * d), fz = (f(x, z + d) - f(x, z - d)) / (2.0f * d);
            data.normals.push_back(glm::normalize(glm::vec3(-fx, 1.0f, -fz)));
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

class AxesWindow : public GLFWSceneWindow {
private:
    ivfvis::Axes3DPtr m_axes;
    bool m_grid{true};
    int m_ticks{5};
    int m_textSize{13};
    char m_titles[3][32]{"x", "f(x, z)", "z"};

public:
    AxesWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<AxesWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<AxesWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.4f, -1.0f, -0.3f});
        sun->setDiffuseColor({0.75f, 0.75f, 0.75f});
        sun->setSpecularColor({0.3f, 0.3f, 0.3f});
        sun->setAmbientColor({0.35f, 0.35f, 0.35f});
        lights->apply();

        auto plot = surface(121);
        auto material = Material::create();
        material->setSpecularColor({0.25f, 0.25f, 0.25f, 1.0f});
        material->setShininess(30.0f);
        plot->setMaterial(material);

        auto cmap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Viridis);
        const BoundingBox box = plot->worldBoundingBox();
        cmap->setRange(box.min().y, box.max().y);
        ivfvis::colorVertices(*plot, [](const glm::vec3 &p) { return p.y; }, *cmap);
        ivfvis::useVertexColors(*plot);
        add(plot);

        // Axes around the data, in data units
        m_axes = ivfvis::Axes3D::create(box);
        applyTitles();
        add(m_axes);

        auto bar = ivfvis::ColorBar::create(cmap);
        bar->setTitle("f(x, z)");
        add(bar);

        cameraManipulator()->setCameraPosition({6.5f, 4.5f, 7.5f});
        cameraManipulator()->setCameraTarget({0.0f, -0.2f, 0.0f});
        return 0;
    }

    void applyTitles()
    {
        m_axes->setTitles(m_titles[0], m_titles[1], m_titles[2]);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({12, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({300, 0}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Axes");
        ImGui::TextDisabled("Rotate the view: walls and labels follow");
        if (ImGui::Checkbox("Grid", &m_grid))
            m_axes->setGrid(m_grid);
        if (ImGui::SliderInt("Ticks", &m_ticks, 2, 12))
            m_axes->setTickCount(m_ticks);
        if (ImGui::SliderInt("Text size", &m_textSize, 8, 24))
            m_axes->setTextSize(m_textSize);
        bool titles = ImGui::InputText("x title", m_titles[0], sizeof(m_titles[0]));
        titles |= ImGui::InputText("y title", m_titles[1], sizeof(m_titles[1]));
        titles |= ImGui::InputText("z title", m_titles[2], sizeof(m_titles[2]));
        if (titles)
            applyTitles();
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = AxesWindow::create(1280, 720, "ivfvis: 3D axes");
    app->addWindow(win);
    return app->loop();
}
