#include <ivf/spline_animation.h>

using namespace ivf;
using namespace ivfmath;

ivf::SplineAnimation::SplineAnimation(ivfmath::SplinePtr spline)
    : m_spline(spline), m_time(0.0f), m_speed(1.0f), m_loop(false), m_animatedNode(nullptr)
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

void ivf::SplineAnimation::setLoop(bool loop)
{
    m_loop = loop;
}

bool ivf::SplineAnimation::loop()
{
    return m_loop;
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

    m_time += dt * m_speed;

    if (m_time > m_spline->size() - 1)
    {
        if (m_loop)
            m_time = 0.0;
        else
            m_time = m_spline->size() - 1;
    }
    if (m_animatedNode != nullptr)
    {
        auto pos = m_spline->getPointByT(m_time);
        std::printf("Time: %f, pos: %f %f %f\n", m_time, pos.x, pos.y, pos.z);
        m_animatedNode->setPos(m_spline->getPointByT(m_time));
    }
}
