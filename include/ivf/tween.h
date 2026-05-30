#pragma once

#include <ivf/math_utils.h>
#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <cmath>

namespace ivf {

enum class Easing {
    Linear,
    QuadIn,    QuadOut,    QuadInOut,
    CubicIn,   CubicOut,   CubicInOut,
    SineIn,    SineOut,    SineInOut,
    ElasticOut,
    BounceOut,
    BackOut
};

namespace detail {

inline float applyEasing(Easing e, float t) noexcept
{
    switch (e) {
        case Easing::QuadIn:     return easeInQuad(t);
        case Easing::QuadOut:    return easeOutQuad(t);
        case Easing::QuadInOut:  return easeInOutQuad(t);
        case Easing::CubicIn:    return easeInCubic(t);
        case Easing::CubicOut:   return easeOutCubic(t);
        case Easing::CubicInOut: return easeInOutCubic(t);
        case Easing::SineIn:     return easeInSine(t);
        case Easing::SineOut:    return easeOutSine(t);
        case Easing::SineInOut:  return easeInOutSine(t);
        case Easing::ElasticOut: return easeOutElastic(t);
        case Easing::BounceOut:  return easeOutBounce(t);
        case Easing::BackOut:    return easeOutBack(t);
        default:                 return t;
    }
}

} // namespace detail

// ---- Non-template base so TweenManager can store heterogeneous tweens ----

class TweenBase {
public:
    virtual ~TweenBase() = default;
    virtual bool update(float dt) = 0; ///< Returns true when done.
    [[nodiscard]] virtual bool isDone() const = 0;
};
using TweenBasePtr = std::shared_ptr<TweenBase>;

// ---- Tween<T> -----------------------------------------------------------

/**
 * @brief Animates a value of type T from its current value to `to` over `duration` seconds.
 *
 * T must support addition, subtraction, and scalar multiplication (e.g. float, glm::vec2/3/4).
 *
 * Usage:
 * @code
 * float x = 0.0f;
 * auto t = Tween<float>::create(x, 5.0f, 1.0f, Easing::QuadOut);
 * TweenManager::instance()->add(t);
 * @endcode
 */
template<typename T>
class Tween : public TweenBase {
private:
    T& m_target;
    T  m_from;
    T  m_to;
    float m_duration;
    float m_elapsed{0.0f};
    bool  m_done{false};
    Easing m_easing;
    std::function<void()> m_onComplete;

    Tween(T& target, T to, float duration, Easing easing)
        : m_target(target), m_from(target), m_to(to), m_duration(duration), m_easing(easing)
    {}

public:
    static std::shared_ptr<Tween<T>> create(T& target, T to, float duration,
                                             Easing easing = Easing::Linear)
    {
        return std::shared_ptr<Tween<T>>(new Tween<T>(target, to, duration, easing));
    }

    void setOnComplete(std::function<void()> fn) { m_onComplete = std::move(fn); }

    bool update(float dt) override
    {
        if (m_done) return true;
        m_elapsed += dt;
        float t = std::min(m_elapsed / m_duration, 1.0f);
        float te = detail::applyEasing(m_easing, t);
        m_target = m_from + (m_to - m_from) * te;
        if (t >= 1.0f) {
            m_target = m_to;
            m_done = true;
            if (m_onComplete) m_onComplete();
        }
        return m_done;
    }

    [[nodiscard]] bool isDone() const override { return m_done; }
};

// ---- TweenManager -------------------------------------------------------

/**
 * @brief Singleton that owns and updates all active tweens each frame.
 *
 * GLFWSceneWindow calls update() automatically. You can also call it manually
 * if using GLFWWindow directly.
 */
class TweenManager {
private:
    std::vector<TweenBasePtr> m_tweens;
    static TweenManager* m_instance;

    TweenManager() = default;

public:
    static TweenManager* instance()
    {
        if (!m_instance)
            m_instance = new TweenManager();
        return m_instance;
    }

    /** Add a tween created with Tween<T>::create(). Ownership is shared. */
    void add(TweenBasePtr tween) { m_tweens.push_back(std::move(tween)); }

    /** Convenience: create and immediately register a tween. Returns the tween for callback chaining. */
    template<typename T>
    std::shared_ptr<Tween<T>> to(T& target, T toValue, float duration,
                                  Easing easing = Easing::Linear)
    {
        auto t = Tween<T>::create(target, toValue, duration, easing);
        m_tweens.push_back(t);
        return t;
    }

    /** Update all active tweens and remove completed ones. Call once per frame. */
    void update(float dt)
    {
        m_tweens.erase(
            std::remove_if(m_tweens.begin(), m_tweens.end(),
                [dt](const TweenBasePtr& tw) { return tw->update(dt); }),
            m_tweens.end());
    }

    /** Cancel all active tweens. */
    void clear() { m_tweens.clear(); }

    [[nodiscard]] size_t count() const { return m_tweens.size(); }
};

} // namespace ivf
