#pragma once

#include <ivf/effect.h>

namespace ivf {

class VignetteEffect : public Effect {
private:
    float m_vignetteSize{1.5f};
    float m_vignetteSmoothness{1.0f};

public:
    VignetteEffect();

    static std::shared_ptr<VignetteEffect> create();

    void setSize(float size);
    float size();

    void setSmoothness(float smoothness);
    float smoothness();

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<VignetteEffect> VignetteEffectPtr;

} // namespace ivf
