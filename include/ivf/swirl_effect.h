#pragma once

#include <ivf/effect.h>

namespace ivf {

class SwirlEffect : public Effect {
private:
    float m_radius{0.5f};
    float m_angle{3.14159f};

public:
    SwirlEffect();
    virtual ~SwirlEffect() = default;

    static std::shared_ptr<SwirlEffect> create();

    void setRadius(float radius);
    void setAngle(float angle);

    float radius() const;
    float angle() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<SwirlEffect> SwirlEffectPtr;

} // namespace ivf
