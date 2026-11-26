/**
 * @file vis_dashboard1.cpp
 * @brief ivfvis: a 3D view linked to 2D plots (ImPlot) - a probe profile and a histogram.
 * @ingroup visualisation_examples
 *
 * Demonstrates 2D plots next to the 3D scene:
 *  - a temperature field on an ivfvis::UniformGrid, shown by a slice plane with contours,
 *    3D axes and a colour bar
 *  - a probe line on the slice: ivfvis::sampleLine() gives the profile plotted with
 *    ImPlot::PlotLine(); move and turn the probe with the sliders
 *  - a histogram of the whole field (ivfvis::histogram(), ImPlot::PlotBars()); drag its two
 *    lines to set the colormap range - slice, contours and colour bar follow
 *
 * ImPlot is available when ivf2 is built with IVF2_WITH_IMPLOT (the default).
 */

#include <imgui.h>
#include <ivf/config.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#if IVF2_HAS_IMPLOT
#include <implot.h>
#endif

#include <array>
#include <cmath>
#include <string>

using namespace ivf;
using namespace ivfui;

namespace {

/** Temperature: warm blobs over a cool gradient */
float temperature(const glm::vec3 &p)
{
    const std::array<glm::vec4, 4> sources{glm::vec4(0.45f, 0.1f, 0.3f, 80.0f), glm::vec4(-0.5f, -0.1f, -0.2f, 60.0f),
                                           glm::vec4(0.0f, 0.3f, -0.55f, 45.0f),
                                           glm::vec4(-0.2f, 0.0f, 0.55f, 35.0f)};
    float t = 15.0f + 8.0f * p.x;
    for (const glm::vec4 &s : sources)
    {
        const glm::vec3 d = p - glm::vec3(s);
        t += s.w * std::exp(-glm::dot(d, d) / 0.06f);
    }
    return t;
}

} // namespace

class DashboardWindow : public GLFWSceneWindow {
private:
    ivfvis::UniformGridPtr m_grid;
    ivfvis::ColormapPtr m_colormap;
    ivfvis::SlicePlanePtr m_slice;
    MeshNodePtr m_probe;
    SpherePtr m_probeStart, m_probeEnd;
    ivfvis::LineSamples m_profile;
    ivfvis::Histogram m_histogram;

    float m_height{0.1f};
    float m_angle{20.0f};
    double m_rangeMin{15.0}, m_rangeMax{85.0};

public:
    DashboardWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<DashboardWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<DashboardWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.3f, -1.0f, -0.5f});
        sun->setDiffuseColor({0.8f, 0.8f, 0.8f});
        sun->setAmbientColor({0.35f, 0.35f, 0.35f});
        lights->apply();

        const BoundingBox box({-1, -0.5f, -1}, {1, 0.5f, 1});
        m_grid = ivfvis::UniformGrid::create({81, 41, 81}, box);
        m_grid->fillScalars("temperature", temperature);
        m_histogram = ivfvis::histogram(m_grid->scalars("temperature"), 60);

        m_colormap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Inferno);
        m_colormap->setRange(static_cast<float>(m_rangeMin), static_cast<float>(m_rangeMax));

        m_slice = ivfvis::SlicePlane::create(m_grid, "temperature", m_colormap);
        m_slice->setContours(8);
        m_slice->setContourColor({0.1f, 0.1f, 0.12f});
        add(m_slice);

        // The probe: a line on the slice with its ends marked
        m_probe = MeshNode::create();
        auto probeMaterial = Material::create();
        probeMaterial->setUseLighting(false);
        probeMaterial->setDiffuseColor({0.3f, 0.9f, 1.0f, 1.0f});
        m_probe->setMaterial(probeMaterial);
        add(m_probe);
        m_probeStart = Sphere::create(0.03, 16, 8);
        m_probeEnd = Sphere::create(0.03, 16, 8);
        for (auto &end : {m_probeStart, m_probeEnd})
        {
            end->setMaterial(probeMaterial);
            add(end);
        }

        auto axes = ivfvis::Axes3D::create(box);
        axes->setTitles("x", "y", "z");
        add(axes);

        auto bar = ivfvis::ColorBar::create(m_colormap);
        bar->setTitle("T [C]");
        add(bar);

        updateProbe();

        cameraManipulator()->setCameraPosition({-0.25f, 2.4f, 2.9f});
        cameraManipulator()->setCameraTarget({-0.55f, -0.25f, 0.0f});
        return 0;
    }

    /** Slice and probe at the probe's height; the profile along it */
    void updateProbe()
    {
        m_slice->setAxis(ivfvis::SliceAxis::Y, m_height);
        const float a = glm::radians(m_angle);
        const glm::vec3 dir(std::cos(a), 0.0f, std::sin(a));
        const glm::vec3 centre(0.0f, m_height, 0.0f);
        const glm::vec3 start = centre - 0.95f * dir, end = centre + 0.95f * dir;
        m_profile = ivfvis::sampleLine(*m_grid, "temperature", start, end, 200);

        const glm::vec3 lift(0.0f, 0.01f, 0.0f); // above the slice
        m_probe->clear();
        m_probe->newMesh(2, 0, GL_LINES);
        auto mesh = m_probe->currentMesh();
        mesh->begin(GL_LINES);
        for (const glm::vec3 &p : {start + lift, end + lift})
        {
            mesh->normal3f(0.0f, 1.0f, 0.0f);
            mesh->vertex3f(p);
        }
        mesh->end();
        m_probe->updateBoundingBox();
        m_probeStart->setPos(start + lift);
        m_probeEnd->setPos(end + lift);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({12, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({460, 640}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Dashboard");

        ImGui::SeparatorText("Probe");
        bool probe = ImGui::SliderFloat("Height", &m_height, -0.45f, 0.45f, "%.2f");
        probe |= ImGui::SliderFloat("Angle", &m_angle, 0.0f, 180.0f, "%.0f deg");
        if (probe)
            updateProbe();

#if IVF2_HAS_IMPLOT
        if (ImPlot::BeginPlot("Along the probe", ImVec2(-1, 230), ImPlotFlags_NoLegend))
        {
            ImPlot::SetupAxes("distance", "T [C]");
            ImPlot::SetupAxisLimits(ImAxis_Y1, m_histogram.range.min, m_histogram.range.max, ImPlotCond_Always);
            const int n = static_cast<int>(m_profile.values.size());
            ImPlot::PlotShaded("T##shade", m_profile.distance.data(), m_profile.values.data(), n,
                               m_histogram.range.min);
            ImPlot::PlotLine("T", m_profile.distance.data(), m_profile.values.data(), n);
            ImPlot::EndPlot();
        }

        ImGui::SeparatorText("Colour range (drag the lines)");
        if (ImPlot::BeginPlot("Histogram", ImVec2(-1, 230), ImPlotFlags_NoLegend))
        {
            ImPlot::SetupAxes("T [C]", "points");
            ImPlot::PlotBars("count", m_histogram.centers.data(), m_histogram.counts.data(),
                             static_cast<int>(m_histogram.counts.size()), m_histogram.binWidth() * 0.9);
            bool changed = ImPlot::DragLineX(0, &m_rangeMin, ImVec4(0.4f, 0.8f, 1.0f, 1.0f), 2.0f);
            changed |= ImPlot::DragLineX(1, &m_rangeMax, ImVec4(1.0f, 0.7f, 0.3f, 1.0f), 2.0f);
            if (changed)
            {
                if (m_rangeMax < m_rangeMin + 0.5)
                    m_rangeMax = m_rangeMin + 0.5;
                m_colormap->setRange(static_cast<float>(m_rangeMin), static_cast<float>(m_rangeMax));
            }
            ImPlot::EndPlot();
        }
#else
        ImGui::TextWrapped("ivf2 was built without ImPlot (IVF2_WITH_IMPLOT=OFF): no plots.");
        double range[2] = {m_rangeMin, m_rangeMax};
        if (ImGui::InputScalarN("Range", ImGuiDataType_Double, range, 2))
        {
            m_rangeMin = range[0];
            m_rangeMax = std::max(range[1], range[0] + 0.5);
            m_colormap->setRange(static_cast<float>(m_rangeMin), static_cast<float>(m_rangeMax));
        }
#endif
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = DashboardWindow::create(1280, 720, "ivfvis: dashboard");
    app->addWindow(win);
    return app->loop();
}
