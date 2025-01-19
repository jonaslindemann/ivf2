#pragma once

#include <memory>
#include <ivfmath/keyframe_interpolator.h>
#include <ivf/transform_node.h>

namespace ivf {

class KeyframeAnimation {
public:
    explicit KeyframeAnimation(TransformNodePtr node);

    static std::shared_ptr<KeyframeAnimation> create(TransformNodePtr node);

    void setNode(TransformNodePtr node);
    TransformNodePtr node() const;

    void setInterpolator(const ivfmath::KeyframeInterpolatorPtr &interpolator);
    ivfmath::KeyframeInterpolatorPtr interpolator() const;

    void update(float deltaTime);
    void play();
    void stop();

    bool isPlaying() const;
    float getCurrentTime() const;

private:
    bool m_playing;
    TransformNodePtr m_node;
    ivfmath::KeyframeInterpolatorPtr m_interpolator;
    float m_currentTime;
};

typedef std::shared_ptr<KeyframeAnimation> KeyframeAnimationPtr;

} // namespace ivf
