#include <ivfui/render_stats_window.h>

#include <imgui.h>

#include <cstdint>
#include <initializer_list>

using namespace ivfui;

namespace {

void statsRow(const char *pass, const ivf::Renderer::Stats &stats)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(pass);
    for (const auto value : {std::uint64_t(stats.items), std::uint64_t(stats.drawn), std::uint64_t(stats.culled),
                             std::uint64_t(stats.skipped), stats.drawCalls, stats.programChanges,
                             stats.materialApplies})
    {
        ImGui::TableNextColumn();
        ImGui::Text("%llu", static_cast<unsigned long long>(value));
    }
}

} // namespace

RenderStatsWindow::RenderStatsWindow(const ivf::RenderStats *stats, const std::string &name)
    : UiWindow(name), m_stats(stats)
{}

std::shared_ptr<RenderStatsWindow> RenderStatsWindow::create(const ivf::RenderStats *stats, const std::string &name)
{
    return std::make_shared<RenderStatsWindow>(stats, name);
}

void RenderStatsWindow::doDraw()
{
    if (m_stats == nullptr)
        return;

    constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("render_stats", 8, flags))
    {
        for (const char *heading : {"Pass", "Items", "Drawn", "Culled", "Skipped", "Draw calls", "Programs", "Materials"})
            ImGui::TableSetupColumn(heading);
        ImGui::TableHeadersRow();

        statsRow("Shadow", m_stats->shadow);
        statsRow("Reflection", m_stats->reflection);
        statsRow("Main", m_stats->main);
        statsRow("Frame", m_stats->frame());
        statsRow("Selection", m_stats->selection);
        ImGui::EndTable();
    }

    ImGui::TextDisabled("Frame = shadow + reflection + main. Selection is rendered after input only.");
    ImGui::TextDisabled("Programs: shader program switches. Materials: materials applied.");
}
