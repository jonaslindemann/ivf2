#include <ivfmusic/song_transport.h>

#include <ivf/audio_manager.h>
#include <ivf/logger.h>

#include <algorithm>
#include <cmath>

using namespace ivfmusic;

namespace {

// Larger differences between the smoothed time and the audio position are treated as a jump
constexpr double kSnapThreshold = 0.1;
// How quickly the smoothed time converges to the audio position (per second)
constexpr double kConvergenceRate = 8.0;

ivf::AudioManager *audioManager()
{
    static bool initialized = false;
    auto audio = ivf::AudioManager::instance();
    if (!initialized)
        initialized = audio->init();
    return initialized ? audio : nullptr;
}

} // namespace

bool SongTransport::load(const std::string &path)
{
    m_time = 0.0;
    m_playing = false;
    m_hasAudio = false;

    auto audio = audioManager();
    if (audio && audio->startMusic(path, false))
    {
        audio->pauseMusic();
        audio->seekMusic(0.0);
        m_length = audio->musicLength();
        m_hasAudio = true;
        audio->setMusicVolume(m_silentVolume);
        ivf::logInfo("Loaded " + path + " (" + std::to_string(m_length) + " s)", "SongTransport");
    }
    else
    {
        ivf::logWarning("No audio for " + path + (audio ? ": " + audio->lastError() : "") + ", using silent time",
                        "SongTransport");
    }
    return m_hasAudio;
}

void SongTransport::play()
{
    if (m_length > 0.0 && m_time >= m_length)
        seek(0.0);
    m_playing = true;
    if (m_hasAudio && !m_freeRunning)
        ivf::AudioManager::instance()->resumeMusic();
}

void SongTransport::pause()
{
    m_playing = false;
    if (m_hasAudio)
        ivf::AudioManager::instance()->pauseMusic();
}

void SongTransport::togglePlay()
{
    m_playing ? pause() : play();
}

void SongTransport::seek(double seconds)
{
    m_time = std::max(seconds, 0.0);
    if (m_length > 0.0)
        m_time = std::min(m_time, m_length);
    if (m_hasAudio)
        ivf::AudioManager::instance()->seekMusic(m_time);
}

void SongTransport::update(double dt)
{
    if (!m_hasAudio || m_freeRunning)
    {
        if (m_playing)
            m_time += dt;
        if (m_length > 0.0 && m_time >= m_length)
        {
            m_time = m_length;
            m_playing = false;
        }
        return;
    }

    auto audio = ivf::AudioManager::instance();
    const double audioTime = audio->musicPosition();

    if (!m_playing)
    {
        m_time = audioTime;
        return;
    }
    if (audio->musicAtEnd())
    {
        m_time = m_length;
        m_playing = false;
        return;
    }

    // Advance smoothly and converge to the (coarse) audio position
    const double predicted = m_time + dt;
    const double error = audioTime - predicted;
    if (std::abs(error) > kSnapThreshold)
        m_time = audioTime;
    else
        m_time = predicted + error * std::min(1.0, dt * kConvergenceRate);
}

void SongTransport::setLength(double seconds)
{
    if (!m_hasAudio)
        m_length = std::max(seconds, 0.0);
}

void SongTransport::setFreeRunning(bool freeRunning)
{
    if (freeRunning == m_freeRunning)
        return;
    m_freeRunning = freeRunning;
    if (!m_hasAudio)
        return;

    auto audio = ivf::AudioManager::instance();
    if (freeRunning)
    {
        audio->pauseMusic();
    }
    else
    {
        audio->seekMusic(m_time);
        if (m_playing)
            audio->resumeMusic();
    }
}

void SongTransport::setVolume(float volume)
{
    m_silentVolume = volume;
    if (m_hasAudio)
        ivf::AudioManager::instance()->setMusicVolume(volume);
}

float SongTransport::volume() const
{
    return m_hasAudio ? ivf::AudioManager::instance()->musicVolume() : m_silentVolume;
}

void SongTransport::setTempo(double bpm, double firstBeat, int beatsPerBar)
{
    m_bpm = std::max(bpm, 1.0);
    m_firstBeat = firstBeat;
    m_beatsPerBar = std::max(beatsPerBar, 1);
}

double SongTransport::beatsAt(double seconds) const
{
    return (seconds - m_firstBeat) * m_bpm / 60.0;
}

double SongTransport::timeAtBeat(double beat) const
{
    return m_firstBeat + beat * 60.0 / m_bpm;
}

double SongTransport::timeAtBar(double bar) const
{
    return timeAtBeat(bar * m_beatsPerBar);
}

double SongTransport::beatPhase() const
{
    const double b = beats();
    return b - std::floor(b);
}
