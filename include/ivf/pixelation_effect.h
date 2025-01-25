#pragma once

#include <ivf/effect.h>

namespace ivf {

class PixelationEffect : public Effect {
private:
    float m_pixelSize{4.0f};

public:
    PixelationEffect();
    virtual ~PixelationEffect();

    static std::shared_ptr<PixelationEffect> create();

    void setPixelSize(float size);
    float pixelSize();

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<PixelationEffect> PixelationEffectPtr;

} // namespace ivf
