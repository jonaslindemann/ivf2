#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ivfmath/keyframe_data.h>

namespace ivfmath {

enum class InterpolationType {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Step,
    CatmullRom
};

class KeyframeInterpolator {
public:
    explicit KeyframeInterpolator(const std::string &name = "");

    static std::shared_ptr<KeyframeInterpolator> create(const std::string &name = "");

    void addKeyframe(const KeyframeData &keyframe);
    KeyframeData interpolate(float time) const;
    void setInterpolationType(InterpolationType type);

    float getDuration() const;
    const std::string &getName() const;

    // Modified to support default tension for new keyframes
    void setSplineTension(float tension);

    // New method to modify tension of existing keyframe
    void setKeyframeTension(size_t index, float tension);

    // New method to get tension of specific keyframe
    float getKeyframeTension(size_t index) const;

private:
    float applyEasing(float t) const;

    std::tuple<glm::vec3, glm::vec3, glm::vec3, glm::vec3> getSplinePoints(size_t currentIndex, float time) const;

    glm::vec3 interpolateSpline(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
                                float t, float tension1, float tension2) const;

    std::string m_name;
    std::vector<KeyframeData> m_keyframes;
    float m_duration;
    InterpolationType m_interpolationType;

    float m_defaultTension; // Renamed from m_splineTension
};

typedef std::shared_ptr<KeyframeInterpolator> KeyframeInterpolatorPtr;

} // namespace ivfmath
