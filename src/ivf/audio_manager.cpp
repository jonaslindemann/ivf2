/**
 * @file audio_manager.cpp
 * @brief AudioManager implementation using miniaudio.
 *
 * miniaudio is declared as a vcpkg dependency (vcpkg.json).
 * After `vcpkg install` the header is available as <miniaudio.h>.
 *
 * If miniaudio.h is absent the library still compiles — all methods return
 * safe no-op/failure values.
 */

// Detect miniaudio presence at compile time.
#if __has_include(<miniaudio.h>)
    #define IVF_HAS_MINIAUDIO 1
    #define MINIAUDIO_IMPLEMENTATION
    #define MA_NO_FLAC
    // Prevent Windows.h min/max macros from conflicting with std::min/std::max
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #ifdef _MSC_VER
    #pragma warning(push, 0)
    #endif
    #include <miniaudio.h>
    #ifdef _MSC_VER
    #pragma warning(pop)
    #endif
#else
    #define IVF_HAS_MINIAUDIO 0
#endif

#include <ivf/audio_manager.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <mutex>
#include <numbers>
#include <vector>
#include <string>

using namespace ivf;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr int   ANALYSIS_SAMPLE_RATE = 44100;
static constexpr int   ANALYSIS_RING_FRAMES = 4096;  // power-of-two; ~93 ms
static constexpr int   FFT_WINDOW           = 2048;  // DFT window length
static constexpr float FFT_MIN_FREQ         = 40.0f;
static constexpr float FFT_MAX_FREQ         = 16000.0f;

// ---------------------------------------------------------------------------
// Ring-buffer push — buffer is pre-allocated so no resize ever occurs.
// Audio callback threads write here; main thread reads it.  We use a relaxed
// atomic for the write-cursor; individual float writes are not guarded by a
// mutex because the main thread takes a snapshot copy (see getFFT/getAmplitude).
// ---------------------------------------------------------------------------
static inline void pushSample(std::vector<float>& buf,
                               std::atomic<size_t>& pos, float sample) noexcept
{
    buf[pos.fetch_add(1, std::memory_order_relaxed) % buf.size()] = sample;
}

// ---------------------------------------------------------------------------
// Pimpl
// ---------------------------------------------------------------------------
struct ivf::AudioManagerImpl {
    // --- pre-allocated analysis ring-buffer
    std::vector<float>  analysisBuffer;
    std::atomic<size_t> analysisPos{0};
    std::string         lastError;

#if IVF_HAS_MINIAUDIO
    ma_engine engine;
    bool      engineInit{false};
    std::vector<ma_sound> sounds;
    static constexpr int MAX_SOUNDS = 256;

    ma_device captureDevice;
    bool      captureActive{false};

    ma_device  analysisDevice;
    ma_decoder analysisDecoder;
    bool       analysisActive{false};
    bool       analysisLoop{false};
    std::string analysisPath;

    AudioManagerImpl()
    {
        // Pre-allocate so the ring-buffer never resizes (lock-free writes safe)
        analysisBuffer.assign(ANALYSIS_RING_FRAMES, 0.0f);
        sounds.reserve(MAX_SOUNDS);
    }

    ~AudioManagerImpl()
    {
        if (captureActive) { ma_device_stop(&captureDevice); ma_device_uninit(&captureDevice); }
        if (analysisActive) {
            ma_device_stop(&analysisDevice);
            ma_device_uninit(&analysisDevice);
            ma_decoder_uninit(&analysisDecoder);
        }
        for (auto& snd : sounds) ma_sound_uninit(&snd);
        if (engineInit) ma_engine_uninit(&engine);
    }
#else
    bool engineInit{false};
    bool captureActive{false};
    bool analysisActive{false};

    AudioManagerImpl() { analysisBuffer.assign(ANALYSIS_RING_FRAMES, 0.0f); }
#endif
};

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------
#if IVF_HAS_MINIAUDIO

static void captureCallback(ma_device* dev, void* pOutput,
                             const void* pInput, ma_uint32 frameCount)
{
    (void)pOutput;
    if (!pInput) return;
    auto* impl = static_cast<AudioManagerImpl*>(dev->pUserData);
    const float* src = static_cast<const float*>(pInput);
    int ch = static_cast<int>(dev->capture.channels);
    for (ma_uint32 f = 0; f < frameCount; ++f) {
        float mono = 0.0f;
        for (int c = 0; c < ch; ++c) mono += src[f * ch + c];
        pushSample(impl->analysisBuffer, impl->analysisPos, mono / ch);
    }
}

static void analysisPlaybackCallback(ma_device* dev, void* pOutput,
                                     const void* /*pInput*/, ma_uint32 frameCount)
{
    auto* impl = static_cast<AudioManagerImpl*>(dev->pUserData);
    float* out = static_cast<float*>(pOutput);
    int ch = static_cast<int>(dev->playback.channels);

    ma_uint64 framesRead = 0;
    ma_decoder_read_pcm_frames(&impl->analysisDecoder, out, frameCount, &framesRead);

    if (framesRead < frameCount)
        std::fill(out + framesRead * ch, out + frameCount * ch, 0.0f);

    for (ma_uint64 f = 0; f < framesRead; ++f) {
        float mono = 0.0f;
        for (int c = 0; c < ch; ++c) mono += out[f * ch + c];
        pushSample(impl->analysisBuffer, impl->analysisPos, mono / ch);
    }

    if (framesRead < frameCount && impl->analysisLoop)
        ma_decoder_seek_to_pcm_frame(&impl->analysisDecoder, 0);
}

#endif // IVF_HAS_MINIAUDIO

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
AudioManager* AudioManager::m_instance = nullptr;
AudioManager::AudioManager() : m_impl(std::make_unique<AudioManagerImpl>()) {}
AudioManager::~AudioManager() = default;
AudioManager* AudioManager::instance() { if (!m_instance) m_instance = new AudioManager(); return m_instance; }
AudioManager* AudioManager::create()   { return instance(); }
void          AudioManager::drop()     { delete m_instance; m_instance = nullptr; }

// ---------------------------------------------------------------------------
bool AudioManager::init()
{
#if IVF_HAS_MINIAUDIO
    if (ma_engine_init(nullptr, &m_impl->engine) != MA_SUCCESS) return false;
    m_impl->engineInit = true;
    return true;
#else
    return false;
#endif
}

// ---------------------------------------------------------------------------
// Playback
// ---------------------------------------------------------------------------
int AudioManager::loadSound(std::string_view path)
{
#if IVF_HAS_MINIAUDIO
    if (!m_impl->engineInit || m_impl->sounds.size() >= (size_t)AudioManagerImpl::MAX_SOUNDS) return -1;
    m_impl->sounds.emplace_back();
    std::string p(path);
    if (ma_sound_init_from_file(&m_impl->engine, p.c_str(),
                                MA_SOUND_FLAG_ASYNC, nullptr, nullptr,
                                &m_impl->sounds.back()) != MA_SUCCESS) {
        m_impl->sounds.pop_back(); return -1;
    }
    return static_cast<int>(m_impl->sounds.size()) - 1;
#else
    (void)path; return -1;
#endif
}

void AudioManager::play(int h, bool loop)
{
#if IVF_HAS_MINIAUDIO
    if (h < 0 || h >= (int)m_impl->sounds.size()) return;
    ma_sound_set_looping(&m_impl->sounds[h], loop ? MA_TRUE : MA_FALSE);
    ma_sound_seek_to_pcm_frame(&m_impl->sounds[h], 0);
    ma_sound_start(&m_impl->sounds[h]);
#else
    (void)h; (void)loop;
#endif
}

void AudioManager::stop(int h)
{
#if IVF_HAS_MINIAUDIO
    if (h < 0 || h >= (int)m_impl->sounds.size()) return;
    ma_sound_stop(&m_impl->sounds[h]);
#else
    (void)h;
#endif
}

void AudioManager::setVolume(int h, float v)
{
#if IVF_HAS_MINIAUDIO
    if (h < 0 || h >= (int)m_impl->sounds.size()) return;
    ma_sound_set_volume(&m_impl->sounds[h], v);
#else
    (void)h; (void)v;
#endif
}

void AudioManager::setSoundPosition(int h, glm::vec3 pos)
{
#if IVF_HAS_MINIAUDIO
    if (h < 0 || h >= (int)m_impl->sounds.size()) return;
    ma_sound_set_position(&m_impl->sounds[h], pos.x, pos.y, pos.z);
#else
    (void)h; (void)pos;
#endif
}

// ---------------------------------------------------------------------------
// File-based analysis
// ---------------------------------------------------------------------------
bool AudioManager::startFileAnalysis(std::string_view path, bool loop)
{
#if IVF_HAS_MINIAUDIO
    if (m_impl->analysisActive) stopFileAnalysis();

    m_impl->analysisPath = std::string(path);
    m_impl->analysisLoop = loop;
    // Reset ring-buffer to silence; keep its pre-allocated size
    std::fill(m_impl->analysisBuffer.begin(), m_impl->analysisBuffer.end(), 0.0f);
    m_impl->analysisPos.store(0, std::memory_order_relaxed);
    m_impl->lastError.clear();

    // Diagnostic path resolution
    std::string resolvedNote;
    {
        std::error_code ec;
        auto full = std::filesystem::current_path(ec) / m_impl->analysisPath;
        if (!ec) {
            resolvedNote = std::filesystem::exists(full, ec)
                ? " (found at: " + full.string() + ")"
                : " — file not found at: " + full.string();
        }
    }

    ma_decoder_config decCfg = ma_decoder_config_init(ma_format_f32, 0, ANALYSIS_SAMPLE_RATE);
    ma_result r = ma_decoder_init_file(m_impl->analysisPath.c_str(), &decCfg,
                                       &m_impl->analysisDecoder);
    if (r != MA_SUCCESS) {
        m_impl->lastError = "ma_decoder_init_file failed (code "
                          + std::to_string((int)r) + ")" + resolvedNote;
        return false;
    }

    ma_uint32 ch = m_impl->analysisDecoder.outputChannels;
    if (ch == 0) ch = 2;

    ma_device_config devCfg = ma_device_config_init(ma_device_type_playback);
    devCfg.playback.format   = ma_format_f32;
    devCfg.playback.channels = ch;
    devCfg.sampleRate        = ANALYSIS_SAMPLE_RATE;
    devCfg.dataCallback      = analysisPlaybackCallback;
    devCfg.pUserData         = m_impl.get();

    r = ma_device_init(nullptr, &devCfg, &m_impl->analysisDevice);
    if (r != MA_SUCCESS) {
        ma_decoder_uninit(&m_impl->analysisDecoder);
        m_impl->lastError = "ma_device_init failed (code " + std::to_string((int)r) + ")";
        return false;
    }
    if (ma_device_start(&m_impl->analysisDevice) != MA_SUCCESS) {
        ma_device_uninit(&m_impl->analysisDevice);
        ma_decoder_uninit(&m_impl->analysisDecoder);
        m_impl->lastError = "ma_device_start failed";
        return false;
    }
    m_impl->analysisActive = true;
    return true;
#else
    (void)path; (void)loop;
    m_impl->lastError = "miniaudio not available";
    return false;
#endif
}

void AudioManager::stopFileAnalysis()
{
#if IVF_HAS_MINIAUDIO
    if (!m_impl->analysisActive) return;
    ma_device_stop(&m_impl->analysisDevice);
    ma_device_uninit(&m_impl->analysisDevice);
    ma_decoder_uninit(&m_impl->analysisDecoder);
    m_impl->analysisActive = false;
#endif
}

bool AudioManager::fileAnalysisRunning() const { return m_impl->analysisActive; }

std::string AudioManager::lastError() const { return m_impl->lastError; }

// ---------------------------------------------------------------------------
// Mic capture
// ---------------------------------------------------------------------------
bool AudioManager::startCapture()
{
#if IVF_HAS_MINIAUDIO
    if (m_impl->captureActive) return true;
    std::fill(m_impl->analysisBuffer.begin(), m_impl->analysisBuffer.end(), 0.0f);
    m_impl->analysisPos.store(0, std::memory_order_relaxed);
    ma_device_config cfg = ma_device_config_init(ma_device_type_capture);
    cfg.capture.format   = ma_format_f32;
    cfg.capture.channels = 1;
    cfg.sampleRate       = ANALYSIS_SAMPLE_RATE;
    cfg.dataCallback     = captureCallback;
    cfg.pUserData        = m_impl.get();
    if (ma_device_init(nullptr, &cfg, &m_impl->captureDevice) != MA_SUCCESS) return false;
    if (ma_device_start(&m_impl->captureDevice) != MA_SUCCESS) {
        ma_device_uninit(&m_impl->captureDevice); return false;
    }
    m_impl->captureActive = true;
    return true;
#else
    return false;
#endif
}

void AudioManager::stopCapture()
{
#if IVF_HAS_MINIAUDIO
    if (!m_impl->captureActive) return;
    ma_device_stop(&m_impl->captureDevice);
    ma_device_uninit(&m_impl->captureDevice);
    m_impl->captureActive = false;
#endif
}

// ---------------------------------------------------------------------------
// Analysis — snapshot the ring-buffer then release the lock before computing
// ---------------------------------------------------------------------------

float AudioManager::getAmplitude()
{
    // Snapshot write-cursor, then read the whole ring buffer without a lock
    // (floats are written atomically on all target platforms for 4-byte alignment)
    size_t pos  = m_impl->analysisPos.load(std::memory_order_relaxed);
    size_t bufN = m_impl->analysisBuffer.size();
    size_t n    = std::min(pos, bufN);  // samples actually written so far
    if (n == 0) return 0.0f;

    double sum = 0.0;
    for (size_t i = 0; i < bufN; ++i) sum += double(m_impl->analysisBuffer[i]) * m_impl->analysisBuffer[i];
    return static_cast<float>(std::sqrt(sum / bufN));
}

// Discrete Fourier Transform at a single frequency bin (Goertzel-style direct DFT).
// Computes the complex magnitude: sqrt(real^2 + imag^2) / windowLen
static float dftBin(const float* samples, int n, float freq, float sampleRate)
{
    const float omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate;
    float re = 0.0f, im = 0.0f;
    for (int i = 0; i < n; ++i) {
        re += samples[i] * std::cos(omega * i);
        im -= samples[i] * std::sin(omega * i);
    }
    return std::sqrt(re * re + im * im) / n;
}

std::vector<float> AudioManager::getFFT(int bands)
{
    bands = std::max(1, std::min(bands, 64));
    std::vector<float> result(bands, 0.0f);

    // Snapshot the ring-buffer position and copy the most recent FFT_WINDOW samples.
    // We read without a mutex; individual float reads are safe on all targets because
    // the buffer is pre-allocated (no resize) and float writes are always aligned.
    size_t writePos = m_impl->analysisPos.load(std::memory_order_acquire);
    size_t bufSize  = m_impl->analysisBuffer.size();
    if (bufSize == 0 || writePos == 0) return result;

    int n = std::min(FFT_WINDOW, static_cast<int>(std::min(writePos, bufSize)));
    std::vector<float> window(n);

    // Extract the last n samples in chronological order
    for (int i = 0; i < n; ++i) {
        size_t idx = (writePos - n + i + bufSize * 2) % bufSize;
        window[i] = m_impl->analysisBuffer[idx];
    }

    // Apply Hann window to reduce spectral leakage
    for (int i = 0; i < n; ++i) {
        float w = 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> * i / (n - 1)));
        window[i] *= w;
    }

    // Compute magnitude at logarithmically-spaced frequency bins
    for (int b = 0; b < bands; ++b) {
        float t    = float(b) / float(bands - 1);
        float freq = FFT_MIN_FREQ * std::pow(FFT_MAX_FREQ / FFT_MIN_FREQ, t);
        result[b]  = dftBin(window.data(), n, freq, float(ANALYSIS_SAMPLE_RATE));
    }

    return result;
}

void AudioManager::update() {}
