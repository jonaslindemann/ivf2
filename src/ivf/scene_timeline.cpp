#include <ivf/scene_timeline.h>

#include <algorithm>
#include <cmath>

namespace ivf {

TimelineCamera TimelineCamera::lookAt(glm::vec3 position, glm::vec3 target, double fov, double nearZ, double farZ,
                                      glm::vec3 up)
{
    TimelineCamera camera;
    camera.position = position;
    camera.target = target;
    camera.up = up;
    camera.fov = fov;
    camera.nearZ = nearZ;
    camera.farZ = farZ;
    return camera;
}

TimelineScene::TimelineScene(std::string_view name, double duration) : m_name(name), m_duration(std::max(0.0, duration))
{}

TimelineScene& TimelineScene::setName(std::string_view name)
{
    m_name = name;
    return *this;
}

TimelineScene& TimelineScene::setDuration(double duration)
{
    m_duration = std::max(0.0, duration);
    return *this;
}

TimelineScene& TimelineScene::root(CompositeNodePtr root)
{
    m_root = root;
    return *this;
}

TimelineScene& TimelineScene::camera(const TimelineCamera& camera)
{
    m_camera = camera;
    return *this;
}

TimelineScene& TimelineScene::effect(EffectPtr effect)
{
    if (effect)
        m_effects.push_back(effect);
    return *this;
}

TimelineScene& TimelineScene::effects(const std::vector<EffectPtr>& effects)
{
    m_effects = effects;
    return *this;
}

TimelineScene& TimelineScene::clearEffects()
{
    m_effects.clear();
    return *this;
}

TimelineScene& TimelineScene::onEnter(Callback callback)
{
    m_onEnter = std::move(callback);
    return *this;
}

TimelineScene& TimelineScene::onUpdate(UpdateCallback callback)
{
    m_onUpdate = std::move(callback);
    return *this;
}

TimelineScene& TimelineScene::onExit(Callback callback)
{
    m_onExit = std::move(callback);
    return *this;
}

const std::string& TimelineScene::name() const
{
    return m_name;
}

double TimelineScene::duration() const
{
    return m_duration;
}

CompositeNodePtr TimelineScene::root() const
{
    return m_root;
}

const TimelineCamera& TimelineScene::camera() const
{
    return m_camera;
}

const std::vector<EffectPtr>& TimelineScene::effects() const
{
    return m_effects;
}

void TimelineScene::enter()
{
    if (m_onEnter)
        m_onEnter();
}

void TimelineScene::update(double localTime, double deltaTime)
{
    if (m_onUpdate)
        m_onUpdate(localTime, deltaTime);
}

void TimelineScene::exit()
{
    if (m_onExit)
        m_onExit();
}

std::shared_ptr<SceneTimeline> SceneTimeline::create()
{
    return std::make_shared<SceneTimeline>();
}

TimelineScene& SceneTimeline::addScene(std::string_view name, double duration)
{
    m_scenes.emplace_back(name, duration);
    return m_scenes.back();
}

void SceneTimeline::clear()
{
    m_scenes.clear();
    m_time = 0.0;
    m_playing = false;
}

void SceneTimeline::play()
{
    m_playing = true;
}

void SceneTimeline::pause()
{
    m_playing = false;
}

void SceneTimeline::togglePlay()
{
    m_playing = !m_playing;
}

void SceneTimeline::stop()
{
    m_playing = false;
    m_time = 0.0;
}

void SceneTimeline::restart()
{
    m_time = 0.0;
    m_playing = true;
}

void SceneTimeline::seek(double time)
{
    auto total = duration();
    if (total <= 0.0) {
        m_time = 0.0;
        return;
    }

    if (m_loop) {
        m_time = std::fmod(std::max(0.0, time), total);
    } else {
        m_time = std::clamp(time, 0.0, total);
    }
}

void SceneTimeline::seekToScene(size_t index)
{
    if (index >= m_scenes.size())
        return;

    double sceneStart = 0.0;
    for (size_t i = 0; i < index; ++i)
        sceneStart += m_scenes[i].duration();

    seek(sceneStart);
}

void SceneTimeline::nextScene()
{
    if (m_scenes.empty())
        return;

    auto index = activeSceneIndex();
    if (index == npos)
        index = 0;
    else
        index = (index + 1) % m_scenes.size();

    seekToScene(index);
}

void SceneTimeline::previousScene()
{
    if (m_scenes.empty())
        return;

    auto index = activeSceneIndex();
    if (index == npos || index == 0)
        index = m_scenes.size() - 1;
    else
        --index;

    seekToScene(index);
}

void SceneTimeline::update(double deltaTime)
{
    if (!m_playing)
        return;

    auto total = duration();
    if (total <= 0.0)
        return;

    m_time += std::max(0.0, deltaTime);

    if (m_loop) {
        while (m_time >= total)
            m_time -= total;
    } else if (m_time >= total) {
        m_time = total;
        m_playing = false;
    }
}

SceneTimeline& SceneTimeline::setLoop(bool loop)
{
    m_loop = loop;
    return *this;
}

bool SceneTimeline::loop() const
{
    return m_loop;
}

bool SceneTimeline::isPlaying() const
{
    return m_playing;
}

double SceneTimeline::time() const
{
    return m_time;
}

double SceneTimeline::duration() const
{
    double total = 0.0;
    for (const auto& scene : m_scenes)
        total += scene.duration();
    return total;
}

double SceneTimeline::localTime() const
{
    auto index = activeSceneIndex();
    if (index == npos)
        return 0.0;

    double sceneStart = 0.0;
    for (size_t i = 0; i < index; ++i)
        sceneStart += m_scenes[i].duration();

    return std::max(0.0, m_time - sceneStart);
}

size_t SceneTimeline::sceneCount() const
{
    return m_scenes.size();
}

size_t SceneTimeline::activeSceneIndex() const
{
    if (m_scenes.empty())
        return npos;

    double sceneStart = 0.0;
    for (size_t i = 0; i < m_scenes.size(); ++i) {
        double sceneEnd = sceneStart + m_scenes[i].duration();
        if (m_time < sceneEnd)
            return i;
        sceneStart = sceneEnd;
    }

    return m_scenes.size() - 1;
}

TimelineScene* SceneTimeline::activeScene()
{
    auto index = activeSceneIndex();
    if (index == npos)
        return nullptr;
    return &m_scenes[index];
}

const TimelineScene* SceneTimeline::activeScene() const
{
    auto index = activeSceneIndex();
    if (index == npos)
        return nullptr;
    return &m_scenes[index];
}

TimelineScene* SceneTimeline::scene(size_t index)
{
    if (index >= m_scenes.size())
        return nullptr;
    return &m_scenes[index];
}

const TimelineScene* SceneTimeline::scene(size_t index) const
{
    if (index >= m_scenes.size())
        return nullptr;
    return &m_scenes[index];
}

} // namespace ivf
