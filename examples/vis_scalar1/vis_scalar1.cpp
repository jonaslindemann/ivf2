/**
 * @file vis_scalar1.cpp
 * @brief ivfvis: isosurfaces and slice planes through a 3D scalar field.
 * @ingroup general_examples
 *
 * Demonstrates scalar field visualisation with ivfvis:
 *  - an ivfvis::UniformGrid (96^3 points) with a "density" field of Gaussian blobs and a
 *    ring, and a "height" array for colouring
 *  - two ivfvis::Isosurface nodes (marching cubes): a translucent outer shell and an inner
 *    surface coloured by height, rebuilt live from the UI sliders
 *  - ivfvis::SlicePlane nodes perpendicular to the axes, coloured by density with contour
 *    lines, and colour bars for both colormaps
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#include <array>
#include <cmath>
#include <string>

using namespace ivf;
using namespace ivfui;

namespace {

/** Density: four blobs and a ring around the y axis, about 0 to 1 */
float density(const glm::vec3 &p)
{
    const std::array<glm::vec4, 4> blobs{glm::vec4(0.0f, 0.55f, 0.0f, 0.38f), glm::vec4(0.75f, -0.35f, 0.3f, 0.3f),
                                         glm::vec4(-0.7f, -0.3f, -0.35f, 0.3f), glm::vec4(0.0f, -0.4f, 0.8f, 0.26f)};
    float d = 0.0f;
    for (const glm::vec4 &b : blobs)
    {
        const glm::vec3 c(b);
        d += std::exp(-glm::dot(p - c, p - c) / (2.0f * b.w * b.w));
    }
    // A ring of radius 1.3 in the plane y = -0.1
    const float ring = std::sqrt(p.x * p.x + p.z * p.z) - 1.3f;
    d += 0.75f * std::exp(-(ring * ring + (p.y + 0.1f) * (p.y + 0.1f)) / (2.0f * 0.16f * 0.16f));
    return d;
}

} // namespace

class ScalarWindow : public GLFWSceneWindow {
private:
    ivfvis::UniformGridPtr m_grid;
    ivfvis::IsosurfacePtr m_shell;
    ivfvis::IsosurfacePtr m_core;
    std::array<ivfvis::SlicePlanePtr, 3> m_slices;
    ivfvis::ColormapPtr m_densityMap;
    ivfvis::ColormapPtr m_heightMap;

    // UI state
    float m_shellIso{0.25f};
    float m_coreIso{0.7f};
    bool m_showShell{true};
    bool m_showCore{true};
    std::array<bool, 3> m_showSlice{false, true, false};
    std::array<float, 3> m_slicePos{0.0f, -0.4f, 0.0f};
    int m_contours{8};

public:
    ScalarWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<ScalarWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<ScalarWindow>(w, h, title);
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.4f, -1.0f, -0.6f});
        sun->setDiffuseColor({0.8f, 0.8f, 0.8f});
        sun->setSpecularColor({0.4f, 0.4f, 0.4f});
        sun->setAmbientColor({0.3f, 0.3f, 0.3f});
        lights->apply();

        // The data
        m_grid = ivfvis::UniformGrid::create({96, 96, 96}, BoundingBox({-2, -2, -2}, {2, 2, 2}));
        m_grid->fillScalars("density", density);
        m_grid->fillScalars("height", [](const glm::vec3 &p) { return p.y; });

        m_densityMap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Viridis);
        m_densityMap->setRange(0.0f, 1.2f);
        m_heightMap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Plasma);
        m_heightMap->setRange(-0.8f, 0.9f);

        // Slices (behind the surfaces in the drawing order does not matter: they are opaque)
        for (int a = 0; a < 3; ++a)
        {
            auto slice = ivfvis::SlicePlane::create(m_grid, "density", m_densityMap);
            slice->setAxis(static_cast<ivfvis::SliceAxis>(a), m_slicePos[static_cast<std::size_t>(a)]);
            slice->setContours(m_contours);
            slice->setContourColor({0.05f, 0.05f, 0.05f});
            slice->setVisible(m_showSlice[static_cast<std::size_t>(a)]);
            add(slice);
            m_slices[static_cast<std::size_t>(a)] = slice;
        }

        // Inner surface coloured by height
        m_core = ivfvis::Isosurface::create(m_grid, "density", m_coreIso);
        auto coreMaterial = Material::create();
        coreMaterial->setSpecularColor({0.5f, 0.5f, 0.5f, 1.0f});
        coreMaterial->setShininess(40.0f);
        m_core->setMaterial(coreMaterial);
        m_core->setColorBy("height", m_heightMap);
        add(m_core);

        // Translucent outer shell
        m_shell = ivfvis::Isosurface::create(m_grid, "density", m_shellIso);
        auto shellMaterial = Material::create();
        shellMaterial->setDiffuseColor({0.55f, 0.75f, 1.0f, 1.0f});
        shellMaterial->setSpecularColor({0.6f, 0.6f, 0.6f, 1.0f});
        shellMaterial->setShininess(60.0f);
        shellMaterial->setAlpha(0.28f);
        m_shell->setMaterial(shellMaterial);
        add(m_shell);

        auto densityBar = ivfvis::ColorBar::create(m_densityMap);
        densityBar->setTitle("density");
        add(densityBar);
        auto heightBar = ivfvis::ColorBar::create(m_heightMap);
        heightBar->setTitle("height");
        heightBar->setPlacement(ivfvis::ColorBarPlacement::Left);
        heightBar->setMargin(40);
        add(heightBar);

        cameraManipulator()->setCameraPosition({3.4f, 2.6f, 4.4f});
        cameraManipulator()->setCameraTarget({0.0f, -0.15f, 0.0f});
        return 0;
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({120, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({330, 0}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Scalar field");
        ImGui::TextDisabled("96 x 96 x 96 grid");

        ImGui::SeparatorText("Isosurfaces");
        if (ImGui::Checkbox("Shell", &m_showShell))
            m_shell->setVisible(m_showShell);
        ImGui::SameLine();
        if (ImGui::SliderFloat("##shell", &m_shellIso, 0.05f, 1.2f, "%.2f"))
            m_shell->setIsoValue(m_shellIso);
        if (ImGui::Checkbox("Core ", &m_showCore))
            m_core->setVisible(m_showCore);
        ImGui::SameLine();
        if (ImGui::SliderFloat("##core", &m_coreIso, 0.05f, 1.2f, "%.2f"))
            m_core->setIsoValue(m_coreIso);
        ImGui::TextDisabled("%zu + %zu triangles", m_shell->triangleCount(), m_core->triangleCount());

        ImGui::SeparatorText("Slices");
        const char *labels[] = {"X", "Y", "Z"};
        for (std::size_t a = 0; a < 3; ++a)
        {
            ImGui::PushID(static_cast<int>(a));
            if (ImGui::Checkbox(labels[a], &m_showSlice[a]))
                m_slices[a]->setVisible(m_showSlice[a]);
            ImGui::SameLine();
            if (ImGui::SliderFloat("##pos", &m_slicePos[a], -2.0f, 2.0f, "%.2f"))
                m_slices[a]->setAxis(static_cast<ivfvis::SliceAxis>(a), m_slicePos[a]);
            ImGui::PopID();
        }
        if (ImGui::SliderInt("Contours", &m_contours, 0, 20))
            for (auto &slice : m_slices)
                slice->setContours(m_contours);
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = ScalarWindow::create(1280, 720, "ivfvis: isosurfaces and slices");
    app->addWindow(win);
    return app->loop();
}
