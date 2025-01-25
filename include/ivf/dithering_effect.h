#pragma once

#include <ivf/effect.h>

namespace ivf {

class DitheringEffect : public Effect {
private:
public:
    DitheringEffect();

    static std::shared_ptr<DitheringEffect> create();

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<DitheringEffect> DitheringEffectPtr;

} // namespace ivf
