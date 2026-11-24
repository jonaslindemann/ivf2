#pragma once

/**
 * @file render_stats_window.h
 * @brief Declares the RenderStatsWindow class, a readout of the render passes' statistics.
 */

#include <ivfui/ui_window.h>

#include <ivf/renderer.h>

#include <memory>
#include <string>

namespace ivfui {

/**
 * @class RenderStatsWindow
 * @brief UI window showing what each render pass drew in the last frame.
 *
 * One row per pass (shadow, reflection, main, selection) with the items in the queue,
 * drawn, culled and skipped items, OpenGL draw calls, shader program changes and material
 * applications. GLFWSceneWindow creates one (menu Windows > Render Statistics) that shows
 * its renderStats().
 */
class RenderStatsWindow : public UiWindow {
private:
    const ivf::RenderStats *m_stats; ///< Statistics to show, owned by the caller.

public:
    /**
     * @brief Constructor.
     * @param stats Statistics to show; must outlive the window (updated by its owner).
     * @param name Window title.
     */
    explicit RenderStatsWindow(const ivf::RenderStats *stats, const std::string &name = "Render Statistics");

    /**
     * @brief Factory method.
     * @param stats Statistics to show; must outlive the window.
     * @param name Window title.
     * @return Shared pointer to the new window.
     */
    [[nodiscard]] static std::shared_ptr<RenderStatsWindow> create(const ivf::RenderStats *stats,
                                                                   const std::string &name = "Render Statistics");

    /** @brief Draw the table (called per frame while visible). */
    void doDraw() override;
};

/** @brief Shared pointer type for RenderStatsWindow. */
using RenderStatsWindowPtr = std::shared_ptr<RenderStatsWindow>;

} // namespace ivfui
