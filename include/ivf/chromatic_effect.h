#pragma once

#include <ivf/effect.h>

namespace ivf {

class ChromaticEffect : public Effect {
private:
    float m_offset{0.01};

public:
    ChromaticEffect();
    virtual ~ChromaticEffect();

    static std::shared_ptr<ChromaticEffect> create();

    void setOffset(float offset);
    float offset() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<ChromaticEffect> ChromaticEffectPtr;

} // namespace ivf
