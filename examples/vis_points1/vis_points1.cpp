/**
 * @file vis_points1.cpp
 * @brief ivfvis: a point cloud of half a million points, coloured and sized by data.
 * @ingroup general_examples
 *
 * Demonstrates ivfvis::PointCloud:
 *  - 500 000 points along a trajectory of the Lorenz attractor (an ivfvis::PointSet with
 *    "speed" and "time" arrays), drawn with one draw call
 *  - colouring through a colormap on the GPU: range, scale, steps and colours change
 *    instantly; a ColorBar shows the scale
 *  - point size in world units (shrinking with distance) or pixels, disc, sphere or square
 *    shapes, size by an array, and transparency
 *  - picking single points: click a point to see its index and values
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#include <string>
#include <vector>

using namespace ivf;
using namespace ivfui;

namespace {

/** Points along a trajectory of the Lorenz system (classic parameters), RK4 */
ivfvis::PointSetPtr lorenz(int count)
{
    const float sigma = 10.0f, rho = 28.0f, beta = 8.0f / 3.0f;
    auto f = [&](const glm::vec3 &p) {
        return glm::vec3(sigma * (p.y - p.x), p.x * (rho - p.z) - p.y, p.x * p.y - beta * p.z);
    };
    const float dt = 0.0025f;
    glm::vec3 p(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 2000; ++i) // onto the attractor
    {
        const glm::vec3 k1 = f(p), k2 = f(p + 0.5f * dt * k1), k3 = f(p + 0.5f * dt * k2), k4 = f(p + dt * k3);
        p += dt / 6.0f * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
    }

    std::vector<glm::vec3> positions;
    std::vector<float> speed, time;
    positions.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
    {
        const glm::vec3 k1 = f(p), k2 = f(p + 0.5f * dt * k1), k3 = f(p + 0.5f * dt * k2), k4 = f(p + dt * k3);
        p += dt / 6.0f * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
        // Lorenz z is up: map to ivf's y, centred around the origin
        positions.emplace_back(p.x * 0.1f, (p.z - 25.0f) * 0.1f, p.y * 0.1f);
        speed.push_back(glm::length(k1));
        time.push_back(static_cast<float>(i) * dt);
    }

    auto points = ivfvis::PointSet::create(std::move(positions));
    std::copy(speed.begin(), speed.end(), points->addScalars("speed").begin());
    std::copy(time.begin(), time.end(), points->addScalars("time").begin());
    points->fillScalars("height", [](const glm::vec3 &q) { return q.y; });
    return points;
}

} // namespace

class PointsWindow : public GLFWSceneWindow {
private:
    ivfvis::PointSetPtr m_points;
    ivfvis::PointCloudPtr m_cloud;
    ivfvis::ColormapPtr m_colormap;
    ivfvis::ColorBarPtr m_colorBar;
    SpherePtr m_marker;
    RayHit m_selected;
    double m_pressX{0.0}, m_pressY{0.0};

    // UI state
    int m_map{static_cast<int>(ivfvis::BuiltinColormap::Inferno)};
    int m_array{0}; // speed, time, height
    float m_size{0.03f};
    int m_sizeMode{0};
    int m_shape{1};
    bool m_sizeBy{true};
    float m_opacity{1.0f};
    int m_steps{0};

public:
    PointsWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<PointsWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<PointsWindow>(w, h, title);
    }

    int onSetup() override
    {
        m_points = lorenz(500000);

        m_colormap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Inferno);
        m_cloud = ivfvis::PointCloud::create(m_points);
        add(m_cloud);

        m_colorBar = ivfvis::ColorBar::create(m_colormap);
        add(m_colorBar);

        // Marks the selected point
        m_marker = Sphere::create(0.08, 16, 8);
        auto markerMaterial = Material::create();
        markerMaterial->setDiffuseColor({0.2f, 0.9f, 1.0f, 1.0f});
        markerMaterial->setUseLighting(false);
        m_marker->setMaterial(markerMaterial);
        m_marker->setVisible(false);
        add(m_marker);

        apply();

        cameraManipulator()->setCameraPosition({1.0f, 0.6f, 8.6f});
        cameraManipulator()->setCameraTarget({0.0f, -0.1f, 0.0f});
        return 0;
    }

    [[nodiscard]] static const char *arrayName(int i)
    {
        static const char *names[] = {"speed", "time", "height"};
        return names[i];
    }

    /** Apply the UI settings */
    void apply()
    {
        const char *name = arrayName(m_array);
        const auto steps = m_steps;
        m_colormap = ivfvis::Colormap::create(static_cast<ivfvis::BuiltinColormap>(m_map));
        m_colormap->setRange(m_points->range(name));
        m_colormap->setSteps(steps);
        m_cloud->setColorBy(name, m_colormap);
        m_colorBar->setColormap(m_colormap);
        m_colorBar->setTitle(name);

        m_cloud->setSizeMode(m_sizeMode == 0 ? ivfvis::PointSizeMode::World : ivfvis::PointSizeMode::Pixels);
        m_cloud->setPointSize(m_sizeMode == 0 ? m_size : m_size * 60.0f);
        m_cloud->setShape(static_cast<ivfvis::PointShape>(m_shape));
        if (m_sizeBy)
            m_cloud->setSizeBy("height", 0.3f, 1.0f);
        else
            m_cloud->clearSizeBy();
        m_cloud->setOpacity(m_opacity);
    }

    void onMouseButton(int button, int action, int /*mods*/) override
    {
        // Click (without dragging the camera) picks the point under the mouse
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
        PickOptions options;
        options.skip = [this](const DrawItem &item) { return item.node == m_marker.get(); };
        m_selected = pick(mouseX(), mouseY(), options);
        m_marker->setVisible(m_selected.point >= 0);
        if (m_selected.point >= 0)
            m_marker->setPos(m_points->position(static_cast<std::size_t>(m_selected.point)));
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({12, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({340, 0}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Point cloud");
        ImGui::Text("%zu points", m_cloud->pointCount());
        bool changed = false;
        const char *arrays[] = {"speed", "time", "height"};
        changed |= ImGui::Combo("Colour by", &m_array, arrays, 3);
        std::vector<std::string> names = ivfvis::Colormap::builtinNames();
        std::vector<const char *> items;
        for (const auto &n : names)
            items.push_back(n.c_str());
        changed |= ImGui::Combo("Colormap", &m_map, items.data(), static_cast<int>(items.size()));
        changed |= ImGui::SliderInt("Steps", &m_steps, 0, 12, m_steps == 0 ? "continuous" : "%d");
        const char *modes[] = {"World units", "Pixels"};
        changed |= ImGui::Combo("Size in", &m_sizeMode, modes, 2);
        changed |= ImGui::SliderFloat("Size", &m_size, 0.005f, 0.2f, "%.3f");
        changed |= ImGui::Checkbox("Size by height", &m_sizeBy);
        const char *shapes[] = {"Disc", "Sphere", "Square"};
        changed |= ImGui::Combo("Shape", &m_shape, shapes, 3);
        changed |= ImGui::SliderFloat("Opacity", &m_opacity, 0.05f, 1.0f, "%.2f");
        if (changed)
            apply();

        ImGui::SeparatorText("Selected point (click)");
        if (m_selected.point >= 0)
        {
            const auto i = static_cast<std::size_t>(m_selected.point);
            ImGui::Text("Index   %d", m_selected.point);
            ImGui::Text("Speed   %.2f", m_points->scalars("speed")[i]);
            ImGui::Text("Time    %.3f", m_points->scalars("time")[i]);
            ImGui::Text("Height  %.3f", m_points->scalars("height")[i]);
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

    auto win = PointsWindow::create(1280, 720, "ivfvis: point cloud");
    app->addWindow(win);
    return app->loop();
}
