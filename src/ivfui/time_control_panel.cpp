#include <ivfui/time_control_panel.h>
#include <ivf/time_controller.h>
#include <imgui.h>

using namespace ivfui;
using namespace ivf;

TimeControlPanel::TimeControlPanel()
    : UiWindow("Time Control")
{
}

std::shared_ptr<TimeControlPanel> TimeControlPanel::create()
{
    return std::make_shared<TimeControlPanel>();
}

void TimeControlPanel::doDraw()
{
    auto *tc = TimeController::instance();

    // Pause / Resume
    if (tc->isPaused()) {
        if (ImGui::Button("Resume"))
            tc->resume();
    } else {
        if (ImGui::Button("Pause"))
            tc->pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
        tc->reset();

    // Time scale slider
    float scale = tc->scale();
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::SliderFloat("Scale", &scale, 0.0f, 4.0f, "%.2f×"))
        tc->setScale(scale);

    // Live readout
    ImGui::Separator();
    ImGui::Text("Elapsed : %.3f s", tc->elapsed());
    ImGui::Text("Delta   : %.4f s", tc->delta());
    ImGui::Text("Status  : %s", tc->isPaused() ? "PAUSED" : "playing");
}
