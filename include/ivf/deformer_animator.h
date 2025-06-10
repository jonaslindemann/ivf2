#pragma once

#include <ivf/deformer.h>

namespace ivf {

struct DeformerKeyframe {
    float time;
    std::unique_ptr<Deformer> deformer;
    
    DeformerKeyframe(float t, std::unique_ptr<Deformer> def) 
        : time(t), deformer(std::move(def)) {}
};

// Deformer animator for keyframe animation
class DeformerAnimator : public Base {
private:
    std::vector<DeformerKeyframe> m_keyframes;
    std::shared_ptr<Deformer> m_targetDeformer;
    float m_currentTime;
    bool m_looping;
    
public:
    DeformerAnimator(std::shared_ptr<Deformer> target);
    
    static std::shared_ptr<DeformerAnimator> create(std::shared_ptr<Deformer> target);
    
    void addKeyframe(float time, const Deformer& deformer);
    void removeKeyframe(size_t index);
    void clear();
    
    void setTime(float time);
    float currentTime() const;
    void setLooping(bool loop);
    bool looping() const;
    
    void update(); // Update target deformer based on current time
    
    float duration() const; // Total animation duration
    size_t keyframeCount() const;
};

};