#include <ivfui/camera_animation.h>

#include <cmath>
#include <random>

using namespace ivfui;
using namespace ivfmath;
using namespace ivf;

// Simple thread-local RNG for shake noise
static float randUnit()
{
    static thread_local std::mt19937 rng{std::random_device{}()};
    static thread_local std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(rng);
}

CameraAnimation::CameraAnimation(CameraManipulatorPtr cam)
    : m_cam(cam)
    , m_posSpline(Spline::create())
    , m_targetSpline(Spline::create())
{}

std::shared_ptr<CameraAnimation> CameraAnimation::create(CameraManipulatorPtr cam)
{
    return std::make_shared<CameraAnimation>(cam);
}

void CameraAnimation::addKeyframe(glm::vec3 pos, glm::vec3 target)
{
    m_posSpline->addPoint(pos);
    m_targetSpline->addPoint(target);
}

void CameraAnimation::clearKeyframes()
{
    m_posSpline->clear();
    m_targetSpline->clear();
    m_elapsed = 0.0;
    m_playing = false;
}

void CameraAnimation::setDuration(float seconds)
{
    m_duration = std::max(0.001f, seconds);
}

float CameraAnimation::duration() const { return m_duration; }

void CameraAnimation::setMode(SplineAnimMode mode) { m_mode = mode; }
SplineAnimMode CameraAnimation::mode() const { return m_mode; }

void CameraAnimation::setAutoLookForward(bool autoLook) { m_autoLookForward = autoLook; }
bool CameraAnimation::autoLookForward() const { return m_autoLookForward; }

void CameraAnimation::setBlockManipulatorDuringPlayback(bool block) { m_blockManipulator = block; }

void CameraAnimation::play()
{
    if (m_posSpline->size() < 2)
        return;
    m_playing = true;
    m_forward = true;
    if (m_blockManipulator)
        m_cam->setManipulationBlocked(true);
}

void CameraAnimation::stop()
{
    m_playing = false;
    m_elapsed = 0.0;
    m_forward = true;
    if (m_blockManipulator)
        m_cam->setManipulationBlocked(false);
}

void CameraAnimation::rewind()
{
    m_elapsed = 0.0;
    m_forward = true;
}

bool CameraAnimation::isPlaying() const { return m_playing; }

float CameraAnimation::progress() const
{
    if (m_duration <= 0.0f)
        return 1.0f;
    float p = static_cast<float>(m_elapsed) / m_duration;
    return std::min(1.0f, std::max(0.0f, p));
}

void CameraAnimation::shake(float intensity, float shakeDuration)
{
    m_shakeIntensity = intensity;
    m_shakeDuration  = shakeDuration;
    m_shakeElapsed   = 0.0;
}

glm::vec3 CameraAnimation::shakeOffset() const
{
    if (m_shakeDuration <= 0.0f || m_shakeElapsed >= m_shakeDuration)
        return glm::vec3(0.0f);
    float decay = 1.0f - static_cast<float>(m_shakeElapsed) / m_shakeDuration;
    float s = m_shakeIntensity * decay;
    return glm::vec3(randUnit() * s, randUnit() * s, randUnit() * s);
}

void CameraAnimation::applyToCamera()
{
    if (m_posSpline->size() < 2)
        return;

    float posLen    = m_posSpline->totalLength();
    float targetLen = m_targetSpline->totalLength();

    // Compute normalised progress [0,1] based on current elapsed and mode
    float raw = (m_duration > 0.0f) ? static_cast<float>(m_elapsed) / m_duration : 1.0f;

    float p; // [0,1] position along path
    if (m_mode == SplineAnimMode::PingPong) {
        // phase in [0,2): [0,1) = forward, [1,2) = backward
        float phase = std::fmod(raw, 2.0f);
        p = (phase < 1.0f) ? phase : (2.0f - phase);
    } else {
        p = std::min(raw, 1.0f);
    }

    glm::vec3 camPos    = m_posSpline->positionByDistance(p * posLen);
    glm::vec3 camTarget;

    if (m_autoLookForward) {
        // Sample slightly ahead along the position spline as the look-at point
        constexpr float kLookAhead = 0.05f;
        float ahead = std::min(1.0f, p + kLookAhead);
        camTarget   = m_posSpline->positionByDistance(ahead * posLen);
        if (glm::length(camTarget - camPos) < 1e-4f) {
            // At the very end of the path — use a slightly behind point
            float behind = std::max(0.0f, p - kLookAhead);
            camTarget    = m_posSpline->positionByDistance(behind * posLen);
        }
    } else if (m_targetSpline->size() >= 2) {
        camTarget = m_targetSpline->positionByDistance(p * targetLen);
    } else {
        camTarget = m_cam->cameraTarget(); // no target spline — keep current target
    }

    camPos += shakeOffset();

    m_cam->setCameraPosition(camPos);
    m_cam->setCameraTarget(camTarget);
}

void CameraAnimation::update(double dt)
{
    // Advance shake regardless of play state
    if (m_shakeElapsed < m_shakeDuration)
        m_shakeElapsed += dt;

    if (!m_playing)
        return;

    m_elapsed += dt;

    float raw = (m_duration > 0.0f) ? static_cast<float>(m_elapsed) / m_duration : 1.0f;

    if (m_mode == SplineAnimMode::Once && raw >= 1.0f) {
        m_elapsed = m_duration; // clamp
        applyToCamera();
        m_playing = false;
        if (m_blockManipulator)
            m_cam->setManipulationBlocked(false);
        return;
    }

    if (m_mode == SplineAnimMode::Loop) {
        m_elapsed = std::fmod(m_elapsed, static_cast<double>(m_duration));
    }
    // PingPong: wrap at 2*duration
    if (m_mode == SplineAnimMode::PingPong) {
        double cycle = 2.0 * m_duration;
        m_elapsed = std::fmod(m_elapsed, cycle);
    }

    applyToCamera();
}
