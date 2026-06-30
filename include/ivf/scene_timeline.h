#pragma once

#include <ivf/composite_node.h>
#include <ivf/property_inspectable.h>

#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

class Effect;
using EffectPtr = std::shared_ptr<Effect>;

/**
 * @struct TimelineCamera
 * @brief Camera shot used by a timeline scene.
 */
struct TimelineCamera {
    glm::vec3 position{0.0f, 4.0f, 8.0f};
    glm::vec3 target{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    double fov{45.0};
    double nearZ{1.0};
    double farZ{100.0};

    static TimelineCamera lookAt(glm::vec3 position, glm::vec3 target, double fov = 45.0,
                                 double nearZ = 1.0, double farZ = 100.0,
                                 glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));
};

/**
 * @class TimelineScene
 * @brief A named timeline clip with its own scene root, camera, effects, and callbacks.
 */
class TimelineScene : public PropertyInspectable {
public:
    using Callback = std::function<void()>;
    using UpdateCallback = std::function<void(double localTime, double deltaTime)>;

private:
    std::string m_name;
    double m_duration{1.0};
    CompositeNodePtr m_root{nullptr};
    std::vector<EffectPtr> m_effects;
    TimelineCamera m_camera;
    Callback m_onEnter;
    UpdateCallback m_onUpdate;
    Callback m_onExit;

public:
    TimelineScene(std::string_view name = {}, double duration = 1.0);

    TimelineScene& setName(std::string_view name);
    TimelineScene& setDuration(double duration);
    TimelineScene& root(CompositeNodePtr root);
    TimelineScene& camera(const TimelineCamera& camera);
    TimelineScene& effect(EffectPtr effect);
    TimelineScene& effects(const std::vector<EffectPtr>& effects);
    TimelineScene& clearEffects();
    TimelineScene& onEnter(Callback callback);
    TimelineScene& onUpdate(UpdateCallback callback);
    TimelineScene& onExit(Callback callback);

    const std::string& name() const;
    double duration() const;
    CompositeNodePtr root() const;
    const TimelineCamera& camera() const;
    const std::vector<EffectPtr>& effects() const;

    void enter();
    void update(double localTime, double deltaTime);
    void exit();
};

/**
 * @class SceneTimeline
 * @brief Ordered collection of timeline scenes with play, pause, seek, and looping.
 */
class SceneTimeline {
public:
    static constexpr size_t npos = std::numeric_limits<size_t>::max();

private:
    std::vector<TimelineScene> m_scenes;
    double m_time{0.0};
    bool m_playing{false};
    bool m_loop{false};

public:
    static std::shared_ptr<SceneTimeline> create();

    TimelineScene& addScene(std::string_view name, double duration);
    void clear();

    void play();
    void pause();
    void togglePlay();
    void stop();
    void restart();
    void seek(double time);
    void seekToScene(size_t index);
    void nextScene();
    void previousScene();
    void update(double deltaTime);

    SceneTimeline& setLoop(bool loop);
    bool loop() const;
    bool isPlaying() const;

    double time() const;
    double duration() const;
    double localTime() const;

    size_t sceneCount() const;
    size_t activeSceneIndex() const;
    TimelineScene* activeScene();
    const TimelineScene* activeScene() const;
    TimelineScene* scene(size_t index);
    const TimelineScene* scene(size_t index) const;
};

using SceneTimelinePtr = std::shared_ptr<SceneTimeline>;

} // namespace ivf
