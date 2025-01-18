#include <ivf/spline_animation.h>

using namespace ivf;
using namespace ivfmath;

ivf::SplineAnimation::SplineAnimation(ivfmath::SplinePtr spline)
    : m_spline(spline), m_time(0.0f), m_speed(1.0f), m_animatedNode(nullptr), m_mode(SplineAnimMode::Once),
      m_interpolationMode(SplineInterpolationMode::Time)
{}

std::shared_ptr<SplineAnimation> ivf::SplineAnimation::create(ivfmath::SplinePtr spline)
{
    return std::make_shared<SplineAnimation>(spline);
}

void ivf::SplineAnimation::setSpline(ivfmath::SplinePtr spline)
{
    m_spline = spline;
}

ivfmath::SplinePtr ivf::SplineAnimation::spline()
{
    return m_spline;
}

void ivf::SplineAnimation::setAnimatedNode(TransformNodePtr node)
{
    m_animatedNode = node;
}

ivf::TransformNodePtr ivf::SplineAnimation::animatedNode()
{
    return m_animatedNode;
}

void ivf::SplineAnimation::setSpeed(double speed)
{
    m_speed = speed;
}

double ivf::SplineAnimation::speed()
{
    return m_speed;
}

void ivf::SplineAnimation::setAnimMode(SplineAnimMode mode)
{
    m_mode = mode;
}

SplineAnimMode ivf::SplineAnimation::animMode()
{
    return m_mode;
}

void ivf::SplineAnimation::setInterpolationMode(SplineInterpolationMode mode)
{
    m_interpolationMode = mode;
}

SplineInterpolationMode ivf::SplineAnimation::interpolationMode()
{
    return m_interpolationMode;
}

void ivf::SplineAnimation::setTime(double time)
{
    m_time = time;
}

double ivf::SplineAnimation::time()
{
    return m_time;
}

void ivf::SplineAnimation::update(double dt)
{
    if (m_spline == nullptr)
        return;

    if (m_interpolationMode == SplineInterpolationMode::Time)
    {
        m_time += dt * m_speed;

        if (m_time > m_spline->size() - 1)
        {
            if (m_mode == SplineAnimMode::Once)
                m_time = m_spline->size() - 1;
            else if (m_mode == SplineAnimMode::Loop)
                m_time = 0.0;
            else if (m_mode == SplineAnimMode::PingPong)
            {
                m_time = m_spline->size() - 1;
                m_speed = -m_speed;
            }
        }
        else
        {
            if (m_time < 0.0)
            {
                if (m_mode == SplineAnimMode::Once)
                    m_time = 0.0;
                else if (m_mode == SplineAnimMode::Loop)
                    m_time = m_spline->size() - 1;
                else if (m_mode == SplineAnimMode::PingPong)
                {
                    m_time = 0.0;
                    m_speed = -m_speed;
                }
            }
        }

        if (m_animatedNode != nullptr)
            m_animatedNode->setPos(m_spline->positionByT(m_time));
    }
    else
    {
        m_distance += dt * m_speed;
        if (m_distance > m_spline->totalLength())
        {
            if (m_mode == SplineAnimMode::Once)
                m_distance = m_spline->totalLength();
            else if (m_mode == SplineAnimMode::Loop)
                m_distance = 0.0;
            else if (m_mode == SplineAnimMode::PingPong)
            {
                m_distance = m_spline->totalLength();
                m_speed = -m_speed;
            }
        }
        else
        {
            if (m_distance < 0.0)
            {
                if (m_mode == SplineAnimMode::Once)
                    m_distance = 0.0;
                else if (m_mode == SplineAnimMode::Loop)
                    m_distance = m_spline->totalLength();
                else if (m_mode == SplineAnimMode::PingPong)
                {
                    m_distance = 0.0;
                    m_speed = -m_speed;
                }
            }
        }
        if (m_animatedNode != nullptr)
            m_animatedNode->setPos(m_spline->positionByDistance(m_distance));
    }
}
