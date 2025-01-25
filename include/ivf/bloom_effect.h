#pragma once

#include <ivf/effect.h>

namespace ivf {

class BloomEffect : public Effect {
private:
    float m_threshold = 1.0;
    float m_intensity = 1.0;

public:
    BloomEffect();
    static std::shared_ptr<BloomEffect> create();
    void setThreshold(float threshold);
    float threshold() const;
    void setIntensity(float intensity);
    float intensity() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<BloomEffect> BloomEffectPtr;

} // namespace ivf
