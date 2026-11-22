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
    BackOut,
    Step,      ///< Hold the start value, jump to the end value at t = 1.
    SmoothStep ///< Hermite smoothstep, 3t² - 2t³.
};

/// Number of values in Easing, for iterating over all curves (e.g. in a UI).
inline constexpr int EasingCount = static_cast<int>(Easing::SmoothStep) + 1;

/**
 * @brief Name of an easing curve, matching the enumerator ("QuadInOut", "SmoothStep", ...).
 */
inline const char *easingName(Easing e) noexcept
{
    static constexpr const char *names[EasingCount] = {
        "Linear",  "QuadIn",  "QuadOut",    "QuadInOut", "CubicIn", "CubicOut", "CubicInOut",
        "SineIn",  "SineOut", "SineInOut",  "ElasticOut", "BounceOut", "BackOut", "Step",
        "SmoothStep"};
    const int index = static_cast<int>(e);
    return index >= 0 && index < EasingCount ? names[index] : "Linear";
}

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
        case Easing::Step:       return t < 1.0f ? 0.0f : 1.0f;
        case Easing::SmoothStep: return t * t * (3.0f - 2.0f * t);
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
    [[nodiscard]] static std::shared_ptr<Tween<T>> create(T& target, T to, float duration,
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
 *
 * Tweens run from the moment they are started and cannot be seeked, which
 * suits transitions triggered by events or input. For values that must follow
 * a song position (scrubbing, frame export), use ivfmusic::Automation, which
 * shares the Easing curves; see it for a comparison of the animation tools.
 */
class TweenManager {
private:
    std::vector<TweenBasePtr> m_tweens;
    std::vector<TweenBasePtr> m_pending; ///< Tweens added during update(), e.g. from an onComplete callback.
    bool m_updating{false};
    bool m_clearRequested{false};
    static TweenManager* m_instance;

    TweenManager() = default;

    void addTween(TweenBasePtr tween)
    {
        // Adding to m_tweens while update() iterates it would invalidate the iteration
        (m_updating ? m_pending : m_tweens).push_back(std::move(tween));
    }

public:
    static TweenManager* instance()
    {
        if (!m_instance)
            m_instance = new TweenManager();
        return m_instance;
    }

    /** Add a tween created with Tween<T>::create(). Ownership is shared. */
    void add(TweenBasePtr tween) { addTween(std::move(tween)); }

    /** Convenience: create and immediately register a tween. Returns the tween for callback chaining. */
    template<typename T>
    std::shared_ptr<Tween<T>> to(T& target, T toValue, float duration,
                                  Easing easing = Easing::Linear)
    {
        auto t = Tween<T>::create(target, toValue, duration, easing);
        addTween(t);
        return t;
    }

    /**
     * Update all active tweens and remove completed ones. Call once per frame.
     * Tweens added from callbacks during the update start with the next update.
     */
    void update(float dt)
    {
        m_updating = true;
        m_tweens.erase(
            std::remove_if(m_tweens.begin(), m_tweens.end(),
                [dt](const TweenBasePtr& tw) { return tw->update(dt); }),
            m_tweens.end());
        m_updating = false;

        if (m_clearRequested)
        {
            m_tweens.clear();
            m_clearRequested = false;
        }
        for (auto& tween : m_pending)
            m_tweens.push_back(std::move(tween));
        m_pending.clear();
    }

    /** Cancel all active tweens. Safe to call from a tween callback. */
    void clear()
    {
        m_pending.clear();
        if (m_updating)
            m_clearRequested = true;
        else
            m_tweens.clear();
    }

    [[nodiscard]] size_t count() const { return m_tweens.size() + m_pending.size(); }
};

} // namespace ivf
