#pragma once

/**
 * @file automation_recorder.h
 * @brief Records live parameter values and triggers as automation keys.
 */

#include <ivfmusic/automation.h>

#include <glm/glm.hpp>

#include <map>
#include <string>
#include <vector>

namespace ivfmusic {

/**
 * @class AutomationRecorder
 * @brief Turns a live performance into automation, so it can be played back and exported frame-exact.
 *
 * While recording, the scene reports every frame which parameters a live control
 * is changing and the value the scene actually used (sample()), and every trigger
 * that fired (trigger()). commit() then writes them into a take, punch-in style:
 *
 * - Each stretch of a parameter that was under live control becomes linear keys,
 *   thinned out so the curve stays within a small tolerance of what was played,
 *   and a region covering the stretch (Automation::addRegion()).
 * - Each trigger becomes a "step" key holding the velocity, at the bar it fired;
 *   its region covers the whole recorded stretch, so a new take of a trigger
 *   replaces its earlier hits there.
 *
 * Merged into the hand-written automation (Automation::merge()), a take is used
 * only inside its regions; outside them the automation is exactly as written.
 * Keys and regions of earlier takes that a new stretch covers are replaced.
 *
 * Recording only advances while the song plays. Seeking starts a new stretch.
 */
class AutomationRecorder {
public:
    /** @brief Start a take; earlier samples are discarded. */
    void start();

    /** @brief Stop recording. The samples are kept until commit() or the next start(). */
    void stop();

    [[nodiscard]] bool recording() const { return m_recording; }

    /** @brief true if the take holds anything to commit. */
    [[nodiscard]] bool hasData() const { return !m_samples.empty() || !m_triggers.empty(); }

    /**
     * @brief Advance to the song position of this frame. Call once per frame before sample().
     * @param bar Current position in bars.
     * @param playing Whether the song is playing; nothing is recorded while paused.
     */
    void frame(double bar, bool playing);

    /**
     * @brief Record the value a live-controlled parameter has in this frame.
     * @param name Parameter name.
     * @param value Value the scene used.
     * @param components Number of components the scene reads (1-4).
     */
    void sample(const std::string &name, glm::vec4 value, int components);

    /** @brief Record a trigger fired in this frame. */
    void trigger(const std::string &name, float velocity);

    /**
     * @brief Write the recording into @p take (keys and regions) and clear it.
     * @return Names of the lanes that were changed.
     */
    std::vector<std::string> commit(Automation &take);

    /** @brief Discard the take. */
    void clear();

    /** @brief Number of recorded samples (for display). */
    [[nodiscard]] size_t sampleCount() const;

private:
    struct Sample {
        int stretch;
        double bar;
        glm::vec4 value;
    };
    struct Lane {
        std::vector<Sample> samples;
        int components{1};
    };
    struct TriggerHit {
        int stretch;
        double bar;
        float velocity;
    };

    bool m_recording{false};
    bool m_playing{false};
    bool m_hasBar{false};
    double m_bar{0.0};
    int m_stretch{0};
    std::vector<std::pair<double, double>> m_stretches; ///< Bar range of each stretch.
    std::map<std::string, Lane, std::less<>> m_samples;
    std::map<std::string, std::vector<TriggerHit>, std::less<>> m_triggers;
};

} // namespace ivfmusic
