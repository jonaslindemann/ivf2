#pragma once

#include <ivfmath/spline.h>
#include <ivf/glbase.h>
#include <ivf/transform_node.h>

namespace ivf {

enum class SplineAnimMode {
    Once,
    Loop,
    PingPong
};

enum class SplineInterpolationMode {
    Time,
    Distance
};

class SplineAnimation : public GLBase {
private:
    ivfmath::SplinePtr m_spline;
    double m_time{0.0};
    double m_distance{0.0};
    double m_speed{1.0};
    SplineAnimMode m_mode{SplineAnimMode::Once};
    SplineInterpolationMode m_interpolationMode{SplineInterpolationMode::Time};

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

    void setAnimMode(SplineAnimMode mode);
    SplineAnimMode animMode();

    void setInterpolationMode(SplineInterpolationMode mode);
    SplineInterpolationMode interpolationMode();

    void setTime(double time);
    double time();

    void update(double dt);
};

typedef std::shared_ptr<SplineAnimation> SplineAnimationPtr;

}; // namespace ivf
