#pragma once

#include <ivfmath/spline.h>
#include <ivf/glbase.h>
#include <ivf/transform_node.h>

namespace ivf {

class SplineAnimation : public GLBase {
private:
    ivfmath::SplinePtr m_spline;
    double m_time{0.0};
    double m_speed{1.0};
    bool m_loop{false};

    TransformNodePtr m_animatedNode;

public:
    SplineAnimation(ivfmath::SplinePtr spline = nullptr);

    static std::shared_ptr<SplineAnimation> create(ivfmath::SplinePtr spline = nullptr);

    void setSpline(ivfmath::SplinePtr spline);
    ivfmath::SplinePtr spline();

    void setAnimatedNode(TransformNodePtr node);
    TransformNodePtr animatedNode();

    void setSpeed(double speed);
    double speed();
    void setLoop(bool loop);
    bool loop();
    void setTime(double time);
    double time();
    void update(double dt);
};

typedef std::shared_ptr<SplineAnimation> SplineAnimationPtr;

}; // namespace ivf
