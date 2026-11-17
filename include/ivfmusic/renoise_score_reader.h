#pragma once

/**
 * @file renoise_score_reader.h
 * @brief Reads note events from a Renoise song (.xrns).
 */

#include <ivfmusic/score.h>

#include <string>

namespace ivfmusic {

/**
 * @class RenoiseScoreReader
 * @brief Loads tempo, pattern sequence and notes of a Renoise song into a Score.
 *
 * Reads Song.xml from the .xrns archive. Each sequence entry becomes a
 * ScoreSection; every sequencer track with notes becomes a ScoreTrack. Note-offs
 * end notes, delay columns shift them within a line, and tracks muted in a
 * sequence slot are skipped. Tempo changes during the song are not supported.
 *
 * Requires ivf2 built with IVF_WITH_RENOISE (miniz and pugixml); otherwise
 * read() fails with an explanatory error.
 */
class RenoiseScoreReader {
public:
    /** @brief true if ivf2 was built with Renoise support. */
    static bool available();

    /**
     * @param path Path to the .xrns file.
     * @param score Receives the score.
     * @param error Receives a description when loading fails.
     * @return true on success.
     */
    static bool read(const std::string &path, Score &score, std::string *error = nullptr);
};

} // namespace ivfmusic
