#include <ivfmusic/score.h>

#include <ivfmusic/renoise_score_reader.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>

using namespace ivfmusic;

float AdsrShape::evaluate(const ScoreNote &note, double time) const
{
    const double t = time - note.time;
    if (t < 0.0)
        return 0.0f;

    // Level while the note is held: attack, then decay towards sustain
    auto heldLevel = [&](double u) {
        if (u < attack)
            return float(u / std::max(attack, 1e-4f));
        const double d = (u - attack) / std::max(decay, 1e-4f);
        return float(sustain + (1.0 - sustain) * std::exp(-3.0 * d));
    };

    float level;
    if (t < note.duration)
    {
        level = heldLevel(t);
    }
    else
    {
        const double sinceRelease = t - note.duration;
        if (sinceRelease >= release)
            return 0.0f;
        level = heldLevel(note.duration) * float(1.0 - sinceRelease / std::max(release, 1e-4f));
    }

    return useVelocity ? level * note.velocity : level;
}

size_t ScoreTrack::firstNoteAtOrAfter(double time) const
{
    auto it = std::lower_bound(notes.begin(), notes.end(), time,
                               [](const ScoreNote &note, double t) { return note.time < t; });
    return size_t(it - notes.begin());
}

float ScoreTrack::envelope(double time, const AdsrShape &shape) const
{
    // Only notes that started within the longest note plus release can still sound
    const double window = maxDuration + shape.release;
    size_t i = firstNoteAtOrAfter(std::nextafter(time, 1e300));

    float value = 0.0f;
    while (i > 0)
    {
        const ScoreNote &note = notes[--i];
        if (note.time < time - window)
            break;
        value = std::max(value, shape.evaluate(note, time));
    }
    return value;
}

bool Score::load(const std::string &path, std::string *error)
{
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });

    if (extension == ".xrns")
        return RenoiseScoreReader::read(path, *this, error);

    if (error)
        *error = "Unsupported score format: " + path;
    return false;
}

void Score::clear()
{
    *this = Score();
}

const ScoreTrack *Score::track(std::string_view name) const
{
    for (const auto &track : m_tracks)
        if (track.name == name)
            return &track;
    return nullptr;
}

void Score::setTempo(double bpm, int beatsPerBar)
{
    m_bpm = bpm;
    m_beatsPerBar = beatsPerBar;
}

void Score::setLength(double seconds)
{
    m_length = seconds;
}

void Score::addTrack(ScoreTrack track)
{
    m_tracks.push_back(std::move(track));
}

void Score::addSection(ScoreSection section)
{
    m_sections.push_back(std::move(section));
}

TrackEnvelope::TrackEnvelope(const Score &score, const std::vector<std::string> &trackNames, AdsrShape shape)
    : m_shape(shape)
{
    for (const auto &name : trackNames)
        if (const ScoreTrack *track = score.track(name))
            m_tracks.push_back(track);
}

float TrackEnvelope::value(double time) const
{
    float value = 0.0f;
    for (const ScoreTrack *track : m_tracks)
        value = std::max(value, track->envelope(time, m_shape));
    return value;
}

std::vector<ScoreNote> TrackEnvelope::notesBetween(double from, double to) const
{
    std::vector<ScoreNote> result;
    for (const ScoreTrack *track : m_tracks)
        for (size_t i = track->firstNoteAtOrAfter(from); i < track->notes.size() && track->notes[i].time < to; ++i)
            result.push_back(track->notes[i]);
    std::sort(result.begin(), result.end(), [](const ScoreNote &a, const ScoreNote &b) { return a.time < b.time; });
    return result;
}
