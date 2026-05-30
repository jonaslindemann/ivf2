#pragma once

#include <ivf/effect.h>

namespace ivf {

class WaveDistortionEffect : public Effect {
private:
    float m_frequency{10.0f};
    float m_amplitude{0.01f};
    float m_speed{1.0f};

public:
    WaveDistortionEffect();
    virtual ~WaveDistortionEffect() = default;

    static std::shared_ptr<WaveDistortionEffect> create();

    void setFrequency(float frequency);
    void setAmplitude(float amplitude);
    void setSpeed(float speed);

    float frequency() const;
    float amplitude() const;
    float speed() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<WaveDistortionEffect> WaveDistortionEffectPtr;

} // namespace ivf
