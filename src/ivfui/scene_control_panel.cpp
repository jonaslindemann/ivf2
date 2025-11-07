#include <ivfui/scene_control_panel.h>

#include <ivfui/glfw_scene_window.h>

#include <ivf/version.h>

using namespace ivfui;

ivfui::SceneControlPanel::SceneControlPanel(std::string caption, GLFWSceneWindow *sceneWindow)
    : UiWindow(caption), m_isDirty(false), m_sceneWindow(sceneWindow), m_showAxis(false), m_showGrid(false),
      m_axisLength(1.0f), m_tickX(11), m_tickY(11), m_tickZ(11), m_tickSpacingX(1.0e-0), m_tickSpacingY(1.0e-0),
      m_tickSpacingZ(1.0e-0)
{
    if (m_sceneWindow == nullptr)
    {
        throw std::runtime_error("SceneControlPanel requires a valid GLFWSceneWindow pointer.");
    }
}

std::shared_ptr<SceneControlPanel> ivfui::SceneControlPanel::create(std::string caption, GLFWSceneWindow *sceneWindow)
{
    return std::make_shared<SceneControlPanel>(caption, sceneWindow);
}

void SceneControlPanel::doDraw()
{
    if (m_sceneWindow == nullptr)
        return;

    m_isDirty = false;

    ImGui::Dummy(ImVec2(200.0f, 0.0f));

    if (ImGui::Button("Reset View"))
    {
        m_sceneWindow->resetView();
    }

    ImGui::SameLine();

    if (ImGui::Button("Save View"))
    {
        m_sceneWindow->saveView();
    }

    if (ImGui::Button("Edit View"))
    {
        m_sceneWindow->showCameraWindow();
    }

    ImGui::SameLine();

    if (ImGui::Button("Zoom Extent"))
    {
        m_sceneWindow->zoomToExtent();
    }

    if (ImGui::Button("Scene Inspector"))
    {
        m_sceneInspector->setVisible(!m_sceneInspector->visible());
    }

    ImGui::Separator();

    if (ImGui::Checkbox("Show Axis", &m_showAxis))
        m_isDirty = true;

    if (m_showAxis)
    {
        if (ImGui::SliderFloat("Axis Length", &m_axisLength, 0.1f, 10.0f))
            m_isDirty = true;
    }

    if (ImGui::Checkbox("Show Grid", &m_showGrid))
        m_isDirty = true;

    if (m_showGrid)
    {
        if (ImGui::SliderInt("Grid X", &m_tickX, 1, 100))
            m_isDirty = true;
        if (ImGui::SliderInt("Grid Y", &m_tickY, 1, 100))
            m_isDirty = true;
        if (ImGui::SliderInt("Grid Z", &m_tickZ, 1, 100))
            m_isDirty = true;
        if (ImGui::SliderFloat("Grid Spacing X", &m_tickSpacingX, 0.1f, 10.0f))
            m_isDirty = true;
        if (ImGui::SliderFloat("Grid Spacing Y", &m_tickSpacingY, 0.1f, 10.0f))
            m_isDirty = true;
        if (ImGui::SliderFloat("Grid Spacing Z", &m_tickSpacingZ, 0.1f, 10.0f))
            m_isDirty = true;
    }

    ImGui::Separator();

    ImGui::Text("Ivf2 Version %s", ivf::version_string);
    ImGui::Text("Build date - %s", ivf::version_date);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
}

bool ivfui::SceneControlPanel::isDirty()
{
    return m_isDirty;
}

void ivfui::SceneControlPanel::doUpdate()
{
    if (m_sceneWindow == nullptr)
        return;

    if (m_isDirty)
    {
        m_sceneWindow->setAxisVisible(m_showAxis);
        m_sceneWindow->setGridVisible(m_showGrid);

        if (m_showAxis)
            m_sceneWindow->setAxisLength(m_axisLength);

        if (m_showGrid)
        {
            m_sceneWindow->setGridTicks(m_tickX, m_tickY, m_tickZ);
            m_sceneWindow->setGridSpacing(m_tickSpacingX, m_tickSpacingY, m_tickSpacingZ);
        }

        m_isDirty = false;
    }
}
