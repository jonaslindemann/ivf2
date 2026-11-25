/**
 * @file vis_volume1.cpp
 * @brief ivfvis: volume rendering of a CT-like scalar field, mixed with opaque geometry.
 * @ingroup general_examples
 *
 * Demonstrates ivfvis::Volume:
 *  - a synthetic "scan" on a 128^3 ivfvis::UniformGrid: a shell, soft tissue with structure
 *    and two denser organs
 *  - a transfer function: colours from a custom colormap, opacity from a curve over the
 *    value range (UI sliders for the tissue, organ and shell bands)
 *  - opaque objects inside and around the volume: an ivfvis::Isosurface of a marker array
 *    and a floor plane; the rays stop where they are
 *  - density, sample count and gradient shading
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#include <cmath>
#include <string>

using namespace ivf;
using namespace ivfui;

namespace {

float smoothstep(float a, float b, float x)
{
    const float t = std::clamp((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

/** CT-like values: 0 outside, ~0.3 tissue, ~0.6 organs, ~1 shell */
float scan(const glm::vec3 &p)
{
    // Ellipsoidal radius
    const float r = glm::length(p / glm::vec3(0.85f, 0.95f, 0.8f));
    const float inside = 1.0f - smoothstep(0.9f, 0.93f, r);
    const float shell = smoothstep(0.82f, 0.86f, r) * inside;
    // Soft tissue with some structure
    const float structure = 0.05f * std::sin(9.0f * p.x) * std::sin(7.0f * p.y + 1.0f) * std::sin(8.0f * p.z);
    float value = inside * (0.3f + structure);
    // Two organs
    const float organ1 = std::exp(-glm::dot(p - glm::vec3(-0.3f, 0.15f, 0.0f), p - glm::vec3(-0.3f, 0.15f, 0.0f)) / 0.04f);
    const float organ2 = std::exp(-glm::dot(p - glm::vec3(0.3f, -0.25f, 0.1f), p - glm::vec3(0.3f, -0.25f, 0.1f)) / 0.025f);
    value = std::max(value, inside * 0.62f * std::max(organ1, organ2) + (1.0f - std::max(organ1, organ2)) * value);
    return std::max(value, shell);
}

} // namespace

class VolumeWindow : public GLFWSceneWindow {
private:
    ivfvis::UniformGridPtr m_grid;
    ivfvis::VolumePtr m_volume;
    ivfvis::ColormapPtr m_colormap;

    // UI state: opacity of the bands
    float m_tissue{0.008f};
    float m_organ{0.9f};
    float m_shell{0.18f};
    float m_density{2.5f};
    int m_samples{256};
    bool m_shading{true};

public:
    VolumeWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<VolumeWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<VolumeWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.4f, -1.0f, -0.5f});
        sun->setDiffuseColor({0.8f, 0.8f, 0.8f});
        sun->setSpecularColor({0.4f, 0.4f, 0.4f});
        sun->setAmbientColor({0.3f, 0.3f, 0.3f});
        lights->apply();

        // The data: the scan, and a marker for an opaque object inside
        m_grid = ivfvis::UniformGrid::create({128, 128, 128}, BoundingBox({-1, -1, -1}, {1, 1, 1}));
        m_grid->fillScalars("scan", scan);
        m_grid->fillScalars("marker", [](const glm::vec3 &p) {
            const glm::vec3 c(0.05f, 0.45f, -0.25f);
            return std::exp(-glm::dot(p - c, p - c) / 0.012f);
        });

        // Dark red to white, like a rendered CT scan
        m_colormap = ivfvis::Colormap::create("scan", {{0.0f, {0.15f, 0.02f, 0.02f, 1.0f}},
                                                       {0.3f, {0.75f, 0.25f, 0.12f, 1.0f}},
                                                       {0.62f, {1.0f, 0.65f, 0.3f, 1.0f}},
                                                       {1.0f, {1.0f, 0.97f, 0.9f, 1.0f}}});
        m_colormap->setRange(0.0f, 1.0f);

        m_volume = ivfvis::Volume::create(m_grid, "scan", m_colormap);
        add(m_volume);

        // Opaque objects: rays stop at them
        auto marker = ivfvis::Isosurface::create(m_grid, "marker", 0.5f);
        auto markerMaterial = Material::create();
        markerMaterial->setDiffuseColor({0.2f, 0.75f, 1.0f, 1.0f});
        markerMaterial->setSpecularColor({0.6f, 0.6f, 0.6f, 1.0f});
        markerMaterial->setShininess(50.0f);
        marker->setMaterial(markerMaterial);
        add(marker);

        auto floor = Box::create({2.4f, 0.04f, 2.4f});
        floor->setPos({0.0f, -0.8f, 0.0f});
        auto floorMaterial = Material::create();
        floorMaterial->setDiffuseColor({0.16f, 0.17f, 0.19f, 1.0f});
        floor->setMaterial(floorMaterial);
        add(floor);

        auto bar = ivfvis::ColorBar::create(m_colormap);
        bar->setTitle("value");
        add(bar);

        apply();

        cameraManipulator()->setCameraPosition({1.7f, 0.9f, 2.1f});
        cameraManipulator()->setCameraTarget({0.0f, -0.05f, 0.0f});
        return 0;
    }

    void apply()
    {
        // Opacity bands: tissue around 0.3, organs around 0.62, shell at 1
        m_volume->setOpacity({{0.0f, 0.0f},
                              {0.15f, 0.0f},
                              {0.3f, m_tissue},
                              {0.45f, m_tissue},
                              {0.6f, m_organ},
                              {0.7f, m_organ},
                              {0.85f, m_shell},
                              {1.0f, m_shell}});
        m_volume->setDensity(m_density);
        m_volume->setSamples(m_samples);
        m_volume->setShading(m_shading);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({12, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({320, 0}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Volume");
        ImGui::TextDisabled("128 x 128 x 128 grid");
        bool changed = false;
        ImGui::SeparatorText("Opacity");
        changed |= ImGui::SliderFloat("Tissue", &m_tissue, 0.0f, 0.3f, "%.3f");
        changed |= ImGui::SliderFloat("Organs", &m_organ, 0.0f, 1.0f, "%.2f");
        changed |= ImGui::SliderFloat("Shell", &m_shell, 0.0f, 1.0f, "%.2f");
        ImGui::SeparatorText("Rendering");
        changed |= ImGui::SliderFloat("Density", &m_density, 0.1f, 5.0f, "%.2f");
        changed |= ImGui::SliderInt("Samples", &m_samples, 32, 1024);
        changed |= ImGui::Checkbox("Shading", &m_shading);
        if (changed)
            apply();
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = VolumeWindow::create(1280, 720, "ivfvis: volume rendering");
    app->addWindow(win);
    return app->loop();
}
