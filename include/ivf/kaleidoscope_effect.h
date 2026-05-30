#pragma once

#include <ivf/effect.h>

namespace ivf {

class KaleidoscopeEffect : public Effect {
private:
    float m_segments{6.0f};
    float m_rotation{0.0f};

public:
    KaleidoscopeEffect();
    virtual ~KaleidoscopeEffect() = default;

    static std::shared_ptr<KaleidoscopeEffect> create();

    void setSegments(float segments);
    void setRotation(float rotation);

    float segments() const;
    float rotation() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<KaleidoscopeEffect> KaleidoscopeEffectPtr;

} // namespace ivf
