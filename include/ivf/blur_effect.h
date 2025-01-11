#pragma once

#include <ivf/effect.h>

namespace ivf {

class BlurEffect : public Effect {
private:
    float m_blurRadius{0.0f};

public:
    BlurEffect();
    virtual ~BlurEffect();

    static std::shared_ptr<BlurEffect> create();

    void setBlurRadius(float radius);
    float blurRadius();

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<BlurEffect> BlurEffectPtr;

} // namespace ivf
