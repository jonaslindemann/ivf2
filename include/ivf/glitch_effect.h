#pragma once

#include <ivf/effect.h>

namespace ivf {

class GlitchEffect : public Effect {
private:
    float m_intensity{0.05f};
    float m_blockSize{0.1f};
    float m_speed{2.0f};

public:
    GlitchEffect();
    virtual ~GlitchEffect() = default;

    static std::shared_ptr<GlitchEffect> create();

    void setIntensity(float intensity);
    void setBlockSize(float blockSize);
    void setSpeed(float speed);

    float intensity() const;
    float blockSize() const;
    float speed() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<GlitchEffect> GlitchEffectPtr;

} // namespace ivf
