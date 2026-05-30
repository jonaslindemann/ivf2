#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace ivf {

/**
 * @brief Singleton for scheduling one-shot and repeating callbacks by elapsed time or frame count.
 *
 * GLFWSceneWindow calls update() automatically each frame.
 *
 * Usage:
 * @code
 * // Fire once after 2 seconds
 * TimerManager::instance()->after(2.0f, []{ spawnEnemy(); });
 *
 * // Fire every 0.5 seconds
 * int id = TimerManager::instance()->every(0.5f, []{ pulse(); });
 *
 * // Cancel a repeating timer
 * TimerManager::instance()->cancel(id);
 * @endcode
 */
class TimerManager {
private:
    struct Entry {
        int   id{0};
        float interval{0.0f};
        float remaining{0.0f};
        int   framesRemaining{-1};  // -1 = time-based
        bool  repeat{false};
        bool  cancelled{false};
        std::function<void()> callback;
    };

    std::vector<Entry> m_entries;
    int m_nextId{1};

    static TimerManager* m_instance;
    TimerManager() = default;

public:
    static TimerManager* instance()
    {
        if (!m_instance)
            m_instance = new TimerManager();
        return m_instance;
    }

    /**
     * Schedule a one-shot callback after `seconds` elapsed game time.
     * Returns a cancellable ID.
     */
    int after(float seconds, std::function<void()> callback)
    {
        m_entries.push_back({m_nextId, seconds, seconds, -1, false, false, std::move(callback)});
        return m_nextId++;
    }

    /**
     * Schedule a repeating callback every `seconds` game time.
     * Returns a cancellable ID.
     */
    int every(float seconds, std::function<void()> callback)
    {
        m_entries.push_back({m_nextId, seconds, seconds, -1, true, false, std::move(callback)});
        return m_nextId++;
    }

    /**
     * Schedule a one-shot callback after `frames` frames.
     * Returns a cancellable ID.
     */
    int afterFrames(int frames, std::function<void()> callback)
    {
        Entry e;
        e.id = m_nextId;
        e.framesRemaining = frames;
        e.repeat = false;
        e.cancelled = false;
        e.callback = std::move(callback);
        m_entries.push_back(std::move(e));
        return m_nextId++;
    }

    /** Cancel a timer by ID. Safe to call from inside a callback. */
    void cancel(int id)
    {
        for (auto& e : m_entries)
            if (e.id == id) { e.cancelled = true; break; }
    }

    /** Cancel all pending timers. */
    void clear() { m_entries.clear(); }

    /** Update all timers. Called automatically by GLFWSceneWindow. */
    void update(float dt)
    {
        for (auto& e : m_entries) {
            if (e.cancelled) continue;
            if (e.framesRemaining >= 0) {
                if (--e.framesRemaining <= 0) {
                    e.callback();
                    e.cancelled = true;
                }
            } else {
                e.remaining -= dt;
                if (e.remaining <= 0.0f) {
                    e.callback();
                    if (e.repeat)
                        e.remaining = e.interval;
                    else
                        e.cancelled = true;
                }
            }
        }
        // Remove dead entries
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                [](const Entry& e){ return e.cancelled && !e.repeat; }),
            m_entries.end());
    }
};

} // namespace ivf
