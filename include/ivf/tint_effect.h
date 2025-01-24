#pragma once

#include <ivf/effect.h>

namespace ivf {

class TintEffect : public Effect {
private:
    float m_blurRadius{0.0f};

public:
    TintEffect();

    static std::shared_ptr<TintEffect> create();

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<TintEffect> TintEffectPtr;

} // namespace ivf
