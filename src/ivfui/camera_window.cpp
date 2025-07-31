#include <ivfui/camera_window.h>
#include <ivf/extent_visitor.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace ivfui;
using namespace ivf;

CameraWindow::CameraWindow(CameraManipulatorPtr camManip, std::string caption)
    : UiWindow(caption), m_cameraManipulator(camManip), m_scene(nullptr), m_isDirty(false), m_fov(45.0f), m_nearZ(0.1f),
      m_farZ(100.0f), m_cameraPosition{0.0f, 0.0f, 0.0f}, m_cameraTarget{0.0f, 0.0f, 0.0f}, m_includeInvisible(false)
{
    updateControls();
}

std::shared_ptr<CameraWindow> CameraWindow::create(CameraManipulatorPtr camManip, std::string caption)
{
    return std::make_shared<CameraWindow>(camManip, caption);
}

void ivfui::CameraWindow::updateControls()
{
    m_cameraPosition[0] = m_cameraManipulator->cameraPosition().x;
    m_cameraPosition[1] = m_cameraManipulator->cameraPosition().y;
    m_cameraPosition[2] = m_cameraManipulator->cameraPosition().z;
    m_cameraTarget[0] = m_cameraManipulator->cameraTarget().x;
    m_cameraTarget[1] = m_cameraManipulator->cameraTarget().y;
    m_cameraTarget[2] = m_cameraManipulator->cameraTarget().z;

    m_fov = m_cameraManipulator->fov();
    m_nearZ = m_cameraManipulator->nearZ();
    m_farZ = m_cameraManipulator->farZ();
}

void ivfui::CameraWindow::updateCamera()
{
    m_cameraManipulator->setCameraPosition(glm::vec3(m_cameraPosition[0], m_cameraPosition[1], m_cameraPosition[2]));
    m_cameraManipulator->setCameraTarget(glm::vec3(m_cameraTarget[0], m_cameraTarget[1], m_cameraTarget[2]));
    m_cameraManipulator->setFov(m_fov);
    m_cameraManipulator->setNearZ(m_nearZ);
    m_cameraManipulator->setFarZ(m_farZ);
}

void CameraWindow::doDraw()
{
    if (m_cameraManipulator == nullptr)
        return;

    updateControls();

    // === Camera Parameters Section ===
    if (ImGui::CollapsingHeader("Camera Parameters", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::InputFloat3("Camera Position", m_cameraPosition))
        {
            updateCamera();
        }
        if (ImGui::InputFloat3("Camera Target", m_cameraTarget))
        {
            updateCamera();
        }
        if (ImGui::SliderFloat("FOV", &m_fov, 1.0f, 179.0f))
        {
            updateCamera();
        }
        if (ImGui::SliderFloat("Near Z", &m_nearZ, 0.001f, 100.0f))
        {
            updateCamera();
        }
        if (ImGui::SliderFloat("Far Z", &m_farZ, 0.1f, 10000.0f))
        {
            updateCamera();
        }
    }

    ImGui::Spacing();

    // === View Control Buttons ===
    if (ImGui::CollapsingHeader("View Controls", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // First row: Reset and Zoom to Extent
        if (ImGui::Button("Reset View"))
        {
            m_cameraManipulator->reset();
        }
        ImGui::SameLine();

        if (ImGui::Button("Zoom to Extent"))
        {
            if (m_scene)
            {
                m_cameraManipulator->zoomToExtent(m_scene, m_includeInvisible);
            }
        }

        // Include invisible checkbox for extent calculation
        ImGui::Checkbox("Include Invisible", &m_includeInvisible);
    }

    ImGui::Spacing();

    // === View Slots Section ===
    if (ImGui::CollapsingHeader("View Slots", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Save current view button
        if (ImGui::Button("Save Current View"))
        {
            m_cameraManipulator->saveState();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restore Saved View"))
        {
            m_cameraManipulator->restoreState();
        }

        ImGui::Spacing();

        // View slots 0-9
        ImGui::Text("Quick Save Slots (0-9):");

        // Create a 5x2 grid of slots
        for (int row = 0; row < 2; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                int slot = row * 5 + col;

                if (col > 0)
                    ImGui::SameLine();

                // Color the button differently if it has data
                bool hasData = m_cameraManipulator->hasSlotData(slot);
                if (hasData)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f)); // Green for saved

                std::string buttonLabel = std::to_string(slot);
                if (ImGui::Button(buttonLabel.c_str(), ImVec2(30, 30)))
                {
                    // Shift+click to save, normal click to restore
                    if (ImGui::GetIO().KeyShift)
                    {
                        m_cameraManipulator->saveStateToSlot(slot);
                    }
                    else if (hasData)
                    {
                        m_cameraManipulator->restoreStateFromSlot(slot);
                    }
                }

                if (hasData)
                    ImGui::PopStyleColor();

                // Tooltip
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    if (hasData)
                    {
                        ImGui::Text("Slot %d: Saved view", slot);
                        ImGui::Text("Click to restore");
                        ImGui::Text("Shift+click to overwrite");
                    }
                    else
                    {
                        ImGui::Text("Slot %d: Empty", slot);
                        ImGui::Text("Shift+click to save current view");
                    }
                    ImGui::EndTooltip();
                }
            }
        }

        ImGui::Spacing();
        ImGui::Text("Tip: Shift+click to save, click to restore");
    }

    // === Camera Information Section ===
    if (ImGui::CollapsingHeader("Camera Information", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Calculate and display distance from target
        auto pos = m_cameraManipulator->cameraPosition();
        auto target = m_cameraManipulator->cameraTarget();
        float distance = glm::length(pos - target);
        ImGui::Text("Distance to target: %.3f", distance);

        // Direction vector
        auto direction = glm::normalize(target - pos);
        ImGui::Text("View direction: (%.3f, %.3f, %.3f)", direction.x, direction.y, direction.z);

        // Camera coordinate system
        glm::vec3 forward = glm::normalize(target - pos);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(forward, up));
        glm::vec3 realUp = glm::normalize(glm::cross(right, forward));

        if (ImGui::TreeNode("Camera Coordinate System"))
        {
            ImGui::Text("Forward: (%.3f, %.3f, %.3f)", forward.x, forward.y, forward.z);
            ImGui::Text("Right:   (%.3f, %.3f, %.3f)", right.x, right.y, right.z);
            ImGui::Text("Up:      (%.3f, %.3f, %.3f)", realUp.x, realUp.y, realUp.z);
            ImGui::TreePop();
        }

        // Scene extent information if available
        if (m_scene && ImGui::TreeNode("Scene Information"))
        {
            ivf::ExtentVisitor extentVisitor(m_includeInvisible);
            m_scene->accept(&extentVisitor);
            auto bbox = extentVisitor.bbox();

            if (bbox.isValid())
            {
                auto center = bbox.center();
                auto size = bbox.size();
                ImGui::Text("Scene center: (%.3f, %.3f, %.3f)", center.x, center.y, center.z);
                ImGui::Text("Scene size: (%.3f, %.3f, %.3f)", size.x, size.y, size.z);
                ImGui::Text("Scene extent: ±(%.3f, %.3f, %.3f)", size.x / 2, size.y / 2, size.z / 2);
            }
            else
            {
                ImGui::Text("No valid scene bounds");
            }
            ImGui::TreePop();
        }
    }
}

void ivfui::CameraWindow::doUpdate()
{
    if (m_cameraManipulator == nullptr)
        return;
}

bool ivfui::CameraWindow::isDirty()
{
    return m_isDirty;
}

void ivfui::CameraWindow::setScene(ivf::CompositeNodePtr scene)
{
    m_scene = scene;
}

ivf::CompositeNodePtr ivfui::CameraWindow::scene() const
{
    return m_scene;
}
