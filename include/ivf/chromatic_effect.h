#pragma once

#include <ivf/effect.h>

namespace ivf {

class ChromaticEffect : public Effect {
private:
    float m_blurRadius{0.0f};

public:
    ChromaticEffect();
    virtual ~ChromaticEffect();

    static std::shared_ptr<ChromaticEffect> create();

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<ChromaticEffect> ChromaticEffectPtr;

} // namespace ivf
