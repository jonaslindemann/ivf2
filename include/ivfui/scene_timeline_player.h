#pragma once

#include <ivf/scene_timeline.h>

#include <memory>

namespace ivfui {

class GLFWSceneWindow;

/**
 * @class SceneTimelinePlayer
 * @brief Applies an ivf SceneTimeline to a GLFWSceneWindow.
 */
class SceneTimelinePlayer {
private:
    GLFWSceneWindow* m_window{nullptr};
    ivf::SceneTimelinePtr m_timeline{nullptr};
    size_t m_activeSceneIndex{ivf::SceneTimeline::npos};
    bool m_renderToTextureWhenNoEffects{false};

    void activateScene(size_t index);
    void applyCamera(const ivf::TimelineCamera& camera);

public:
    SceneTimelinePlayer(GLFWSceneWindow* window, ivf::SceneTimelinePtr timeline);

    static std::shared_ptr<SceneTimelinePlayer> create(GLFWSceneWindow* window, ivf::SceneTimelinePtr timeline);

    void setTimeline(ivf::SceneTimelinePtr timeline);
    ivf::SceneTimelinePtr timeline() const;

    void setRenderToTextureWhenNoEffects(bool enabled);
    bool renderToTextureWhenNoEffects() const;

    void update(double deltaTime);
    void reset();
};

using SceneTimelinePlayerPtr = std::shared_ptr<SceneTimelinePlayer>;

} // namespace ivfui
