#include <ivf/keyframe_animation.h>

using namespace ivf;

KeyframeAnimation::KeyframeAnimation(TransformNodePtr node) : m_node(node), m_currentTime(0.0f), m_playing(false)
{}

std::shared_ptr<KeyframeAnimation> ivf::KeyframeAnimation::create(TransformNodePtr node)
{
    return std::make_shared<KeyframeAnimation>(node);
}

void ivf::KeyframeAnimation::setNode(TransformNodePtr node)
{
    m_node = node;
}

TransformNodePtr ivf::KeyframeAnimation::node() const
{
    return m_node;
}

void ivf::KeyframeAnimation::setInterpolator(const ivfmath::KeyframeInterpolatorPtr &interpolator)
{
    m_interpolator = interpolator;
}

ivfmath::KeyframeInterpolatorPtr ivf::KeyframeAnimation::interpolator() const
{
    return m_interpolator;
}

void KeyframeAnimation::update(float deltaTime)
{
    if (!m_interpolator || !m_node)
        return;

    if (m_playing)
        m_currentTime += deltaTime;

    auto interpolated = m_interpolator->interpolate(m_currentTime);

    m_node->setPos(interpolated.position);
    m_node->setEulerAngles(interpolated.rotation);
    // Note: You might want to add scale support to TransformNode
}

void KeyframeAnimation::play()
{
    m_currentTime = 0.0f;
    m_playing = true;
}

void KeyframeAnimation::stop()
{
    m_interpolator.reset();
    m_currentTime = 0.0f;
    m_playing = false;
}

bool KeyframeAnimation::isPlaying() const
{
    return m_playing;
}

float KeyframeAnimation::getCurrentTime() const
{
    return m_currentTime;
}
