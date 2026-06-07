#include <ivfui/scene_timeline_player.h>

#include <ivfui/glfw_scene_window.h>

namespace ivfui {

SceneTimelinePlayer::SceneTimelinePlayer(GLFWSceneWindow* window, ivf::SceneTimelinePtr timeline)
    : m_window(window), m_timeline(timeline)
{}

std::shared_ptr<SceneTimelinePlayer> SceneTimelinePlayer::create(GLFWSceneWindow* window, ivf::SceneTimelinePtr timeline)
{
    return std::make_shared<SceneTimelinePlayer>(window, timeline);
}

void SceneTimelinePlayer::setTimeline(ivf::SceneTimelinePtr timeline)
{
    if (m_timeline && m_activeSceneIndex != ivf::SceneTimeline::npos) {
        if (auto* scene = m_timeline->scene(m_activeSceneIndex))
            scene->exit();
    }

    m_timeline = timeline;
    m_activeSceneIndex = ivf::SceneTimeline::npos;
}

ivf::SceneTimelinePtr SceneTimelinePlayer::timeline() const
{
    return m_timeline;
}

void SceneTimelinePlayer::setRenderToTextureWhenNoEffects(bool enabled)
{
    m_renderToTextureWhenNoEffects = enabled;
}

bool SceneTimelinePlayer::renderToTextureWhenNoEffects() const
{
    return m_renderToTextureWhenNoEffects;
}

void SceneTimelinePlayer::update(double deltaTime)
{
    if (!m_window || !m_timeline)
        return;

    m_timeline->update(deltaTime);

    auto index = m_timeline->activeSceneIndex();
    if (index != m_activeSceneIndex)
        activateScene(index);

    if (auto* scene = m_timeline->activeScene())
        scene->update(m_timeline->localTime(), deltaTime);
}

void SceneTimelinePlayer::reset()
{
    if (!m_timeline)
        return;

    m_activeSceneIndex = ivf::SceneTimeline::npos;
    m_timeline->stop();
}

void SceneTimelinePlayer::activateScene(size_t index)
{
    if (!m_window || !m_timeline)
        return;

    if (m_activeSceneIndex != ivf::SceneTimeline::npos) {
        if (auto* previousScene = m_timeline->scene(m_activeSceneIndex))
            previousScene->exit();
    }

    m_activeSceneIndex = index;

    m_window->clear();
    m_window->clearEffects();

    auto* scene = m_timeline->scene(index);
    if (!scene)
        return;

    if (scene->root())
        m_window->add(scene->root());

    applyCamera(scene->camera());

    for (auto& effect : scene->effects())
        m_window->addEffect(effect);

    m_window->setRenderToTexture(!scene->effects().empty() || m_renderToTextureWhenNoEffects);

    scene->enter();
}

void SceneTimelinePlayer::applyCamera(const ivf::TimelineCamera& camera)
{
    auto manipulator = m_window->cameraManipulator();
    if (!manipulator)
        return;

    manipulator->setCameraPosition(camera.position);
    manipulator->setCameraTarget(camera.target);
    manipulator->setFov(camera.fov);
    manipulator->setNearZ(camera.nearZ);
    manipulator->setFarZ(camera.farZ);
}

} // namespace ivfui
