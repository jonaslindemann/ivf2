#pragma once

/**
 * @file automation.h
 * @brief Named parameters keyed by bar position, for continuous scene changes over a song.
 */

#include <ivf/tween.h>

#include <glm/glm.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace ivfmusic {

/**
 * @struct AutomationKey
 * @brief One key of an automation lane.
 */
struct AutomationKey {
    double bar{1.0};
    glm::vec4 value{0.0f};
    ivf::Easing easing{ivf::Easing::Linear}; ///< Curve used to arrive at this key from the previous one.
    bool spline{false}; ///< Arrive on a smooth curve through the neighbouring keys (Catmull-Rom); overrides easing.
    int components{1};  ///< Number of components given for the value (1-4).
};

/**
 * @struct SongMarker
 * @brief A named position in the song, e.g. the start of a verse.
 */
struct SongMarker {
    double bar{1.0};
    std::string name;
};

/**
 * @class Automation
 * @brief Keyframed parameters (automation lanes) and named markers over the song, positioned in bars.
 *
 * Positions are musical bar numbers as counted by musicians: 1.0 is the start of
 * the song, 121.0 the start of bar 121, 121.5 halfway through it. Each key holds
 * a value (1 to 4 components) and the easing used to arrive at it from the
 * previous key. Before the first key a parameter holds the first value, after
 * the last key the last value.
 *
 * Runs of spline keys form one smooth curve, so motion such as a camera path
 * flows through its keys without stopping. The curve starts and ends at rest,
 * and also comes to rest at a key next to a segment of another easing.
 *
 * Usually loaded from a JSON file that can be reloaded while the scene runs:
 * @code{.json}
 * {
 *   "markers": [ { "bar": 1, "name": "Intro" }, { "bar": 17, "name": "Verse 1" } ],
 *   "params": {
 *     "fog":   [ [1, 0.12], [17, 0.06, "sine"] ],
 *     "sky":   [ [1, [0.01, 0.01, 0.03]], [121, [0.3, 0.1, 0.4], "smooth"] ]
 *   }
 * }
 * @endcode
 *
 * Easing names: linear, step, smooth, sine, in, out, cubic, spline, and every
 * ivf::Easing name (e.g. "quadInOut", "elasticOut"; case-insensitive).
 *
 * @section automation_choosing Automation and the other animation tools
 *
 * ivf2 has several ways to change things over time. They differ in what drives
 * time and whether a value can be looked up for any moment:
 *
 * | Tool | Time base | Seekable | Use for |
 * |------|-----------|----------|---------|
 * | Automation | Musical bars of the song | Yes, a pure function of the position | Continuous parameter changes synchronised to music: colours, fog, camera paths, effect amounts |
 * | ivf::SceneTimeline | Seconds, advanced per frame | Seek between scenes | Sequencing whole scenes, each with its own root, camera and effects (presentations, storyboards) |
 * | ivf::Tween / ivf::TweenManager | Seconds from when the tween starts | No | Transitions triggered by events or input (move this there, fade that out) |
 * | ivf::KeyframeAnimation, ivf::SplineAnimation | Seconds, advanced per frame | Play/stop | Moving one TransformNode through keyframes or along a spline |
 *
 * Use Automation when the result must be reproducible at any song position:
 * scrubbing, seeking to a marker and frame export all give the same picture,
 * because nothing depends on the previous frame. A rate can still be
 * accumulated without state with integral().
 */
class Automation {
public:
    using Lanes = std::map<std::string, std::vector<AutomationKey>, std::less<>>;

    /**
     * @brief Load from a JSON file, replacing the current contents.
     *
     * On error the current contents are kept, so a typo while editing does not
     * reset a running scene.
     */
    bool loadFile(const std::string &path, std::string *error = nullptr);

    /**
     * @brief Reload the file loaded with loadFile() if it changed on disk.
     * @return true if it was reloaded successfully.
     */
    bool reloadIfChanged(std::string *error = nullptr);

    [[nodiscard]] const std::string &path() const { return m_path; }

    /**
     * @brief Write markers and parameters to a JSON file in the format loadFile() reads.
     *
     * The file is written to a temporary file first and then renamed, so a reader
     * never sees a half-written file. Comments of a hand-written file are not kept;
     * GLFWMusicSceneWindow therefore saves recordings to a separate take file.
     * The automation then refers to @p path, so reloadIfChanged() watches it.
     */
    bool saveFile(const std::string &path, std::string *error = nullptr);

    void clear();

    /**
     * @brief Lay the lanes of @p other over this automation.
     *
     * A lane of @p other with regions (a recorded take) is used only inside its
     * regions; everywhere else this automation's lane of the same name is used
     * exactly as before. A lane without regions replaces the lane of the same name.
     * Markers are kept.
     */
    void merge(const Automation &other);

    /**
     * @brief Mark [fromBar, toBar] of lane @p name as a region, merging it with regions it overlaps.
     *
     * Regions mark where a recorded take is in effect when this automation is
     * merged into another; they are saved as "regions" in the JSON file.
     */
    void addRegion(const std::string &name, double fromBar, double toBar);

    /** @brief Regions of lane @p name, sorted; empty if it has none. */
    [[nodiscard]] std::vector<std::pair<double, double>> regions(std::string_view name) const;

    /** @brief Names of all parameters, including those only defined by merged takes. */
    [[nodiscard]] std::vector<std::string> names() const;

    /** @brief Remove the keys of @p name with fromBar <= bar <= toBar. */
    void eraseKeys(const std::string &name, double fromBar, double toBar);

    /** @brief Remove a parameter. */
    void removeLane(const std::string &name);

    /**
     * @brief The last key of @p name at or before @p bar, nullptr if there is none.
     *
     * With a merged take, keys of the lane inside the take's regions are replaced by the take's keys.
     */
    [[nodiscard]] const AutomationKey *keyAtOrBefore(std::string_view name, double bar) const;

    /** @brief Add or replace the key of @p name at key.bar. */
    void setKey(const std::string &name, const AutomationKey &key);
    void setKey(const std::string &name, double bar, glm::vec4 value, ivf::Easing easing = ivf::Easing::Linear,
                int components = 4);
    void setKey(const std::string &name, double bar, float value, ivf::Easing easing = ivf::Easing::Linear);
    /** @brief Add or replace a key that continues a smooth spline curve. */
    void setSplineKey(const std::string &name, double bar, glm::vec4 value, int components = 4);

    /** @brief Largest number of components used by the keys of @p name, 0 if it does not exist. */
    [[nodiscard]] int components(std::string_view name) const;

    void addMarker(double bar, std::string name);

    [[nodiscard]] bool has(std::string_view name) const;

    /**
     * @brief Interpolated value at a bar position.
     *
     * @p fallback if the parameter is missing, or only defined by a merged take
     * whose regions do not cover @p bar.
     */
    [[nodiscard]] glm::vec4 vec4(std::string_view name, double bar, glm::vec4 fallback = glm::vec4(0.0f)) const;
    [[nodiscard]] glm::vec3 vec3(std::string_view name, double bar, glm::vec3 fallback = glm::vec3(0.0f)) const;
    [[nodiscard]] float value(std::string_view name, double bar, float fallback = 0.0f) const;

    /**
     * @brief Integral of a parameter's first component from @p fromBar to @p toBar, in value x bars.
     *
     * Turns a rate into an amount that is still a pure function of the position,
     * e.g. how far something has drifted while a "weightless" value was up.
     * Uses the trapezoid rule with steps of at most @p step bars.
     */
    [[nodiscard]] double integral(std::string_view name, double fromBar, double toBar, float fallback = 0.0f,
                                  double step = 1.0 / 32.0) const;

    [[nodiscard]] const std::vector<SongMarker> &markers() const { return m_markers; }

    /** @brief Index of the marker in effect at @p bar, -1 before the first. */
    [[nodiscard]] int markerIndexAt(double bar) const;

    [[nodiscard]] const Lanes &lanes() const { return m_lanes; }

    /**
     * @brief Parse an easing name as used in automation files.
     * @return false if the name is unknown.
     */
    static bool parseEasing(std::string_view name, ivf::Easing &easing, bool &spline);

private:
    using Regions = std::vector<std::pair<double, double>>;
    struct Overlay {
        Regions regions;
        std::vector<AutomationKey> keys;
    };

    /** @brief The overlay of @p name covering @p bar, nullptr if none. */
    [[nodiscard]] const Overlay *overlayAt(std::string_view name, double bar) const;

    Lanes m_lanes;
    std::vector<SongMarker> m_markers;
    std::map<std::string, Regions, std::less<>> m_regions;   ///< Regions of this automation's lanes (a take).
    std::map<std::string, Overlay, std::less<>> m_overlays; ///< Takes merged into this automation.

    std::string m_path;
    std::filesystem::file_time_type m_fileTime{};
};

} // namespace ivfmusic
