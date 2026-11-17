#pragma once

/**
 * @file score.h
 * @brief Note events of a composition, and envelopes derived from them.
 */

#include <string>
#include <string_view>
#include <vector>

namespace ivfmusic {

/**
 * @struct ScoreNote
 * @brief One note of a track.
 */
struct ScoreNote {
    double time{0.0};     ///< Start in song seconds.
    double duration{0.0}; ///< Until note-off, the next note in the same column, or the song end.
    int pitch{60};        ///< MIDI note number (C-4 = 48 in Renoise notation, 60 = C-5).
    float velocity{1.0f}; ///< [0, 1].
    int column{0};        ///< Note column within the track.

    [[nodiscard]] double end() const { return time + duration; }
};

/**
 * @struct AdsrShape
 * @brief How a note becomes a smooth control value over time.
 *
 * A note rises over @c attack seconds, falls over @c decay seconds towards
 * @c sustain while held, and fades over @c release seconds after it ends.
 * Long attack and release give a "breathing" response; short ones give sharp hits.
 */
struct AdsrShape {
    float attack{0.02f};
    float decay{0.3f};
    float sustain{0.0f};
    float release{0.3f};
    bool useVelocity{true}; ///< Scale by note velocity.

    /** @brief Envelope value of one note at @p time, in [0, 1]. */
    [[nodiscard]] float evaluate(const ScoreNote &note, double time) const;
};

/**
 * @struct ScoreTrack
 * @brief A named track with its notes sorted by start time.
 */
struct ScoreTrack {
    std::string name;
    std::vector<ScoreNote> notes;
    double maxDuration{0.0}; ///< Longest note, bounds envelope lookups.

    /** @brief Index of the first note starting at or after @p time. */
    [[nodiscard]] size_t firstNoteAtOrAfter(double time) const;

    /**
     * @brief Envelope of the track at @p time: the maximum over its sounding notes.
     *
     * A pure function of time, so it is exact under scrubbing and frame export.
     */
    [[nodiscard]] float envelope(double time, const AdsrShape &shape) const;
};

/**
 * @struct ScoreSection
 * @brief A span of the song, e.g. one entry of a tracker's pattern sequence.
 */
struct ScoreSection {
    std::string name;
    double start{0.0};
    double end{0.0};
};

/**
 * @class Score
 * @brief Tempo, structure and note events of a composition.
 *
 * Loaded from the composition rather than analysed from audio, so note timing
 * is exact and every instrument is a separate track.
 */
class Score {
public:
    /**
     * @brief Load a score file. Supported: Renoise (.xrns), when built with IVF_WITH_RENOISE.
     * @param path File path.
     * @param error Receives a description when loading fails.
     * @return true on success.
     */
    bool load(const std::string &path, std::string *error = nullptr);

    void clear();

    [[nodiscard]] bool empty() const { return m_tracks.empty(); }

    [[nodiscard]] double bpm() const { return m_bpm; }
    [[nodiscard]] int beatsPerBar() const { return m_beatsPerBar; }
    [[nodiscard]] double length() const { return m_length; }

    [[nodiscard]] const std::vector<ScoreTrack> &tracks() const { return m_tracks; }
    [[nodiscard]] const std::vector<ScoreSection> &sections() const { return m_sections; }

    /** @brief Track by exact name, nullptr if there is none. */
    [[nodiscard]] const ScoreTrack *track(std::string_view name) const;

    /** @name Building a score (used by readers) */
    ///@{
    void setTempo(double bpm, int beatsPerBar);
    void setLength(double seconds);
    void addTrack(ScoreTrack track);
    void addSection(ScoreSection section);
    ///@}

private:
    double m_bpm{120.0};
    int m_beatsPerBar{4};
    double m_length{0.0};
    std::vector<ScoreTrack> m_tracks;
    std::vector<ScoreSection> m_sections;
};

/**
 * @class TrackEnvelope
 * @brief Control value from one or more tracks, e.g. "bass" from the Kick and Bass tracks.
 */
class TrackEnvelope {
public:
    TrackEnvelope() = default;

    /**
     * @brief Combine the named tracks of a score. Unknown names are ignored.
     * @param score Score that outlives the envelope.
     * @param trackNames Tracks to combine (maximum of their envelopes).
     * @param shape Envelope shape.
     */
    TrackEnvelope(const Score &score, const std::vector<std::string> &trackNames, AdsrShape shape);

    /** @brief Value at a song time, in [0, 1]. */
    [[nodiscard]] float value(double time) const;

    /** @brief Notes of all tracks starting in [from, to), e.g. to spawn effects per note. */
    [[nodiscard]] std::vector<ScoreNote> notesBetween(double from, double to) const;

    [[nodiscard]] AdsrShape &shape() { return m_shape; }
    [[nodiscard]] const AdsrShape &shape() const { return m_shape; }
    [[nodiscard]] bool empty() const { return m_tracks.empty(); }

private:
    std::vector<const ScoreTrack *> m_tracks;
    AdsrShape m_shape;
};

} // namespace ivfmusic
