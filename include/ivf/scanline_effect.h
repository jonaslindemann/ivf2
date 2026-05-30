#pragma once

#include <ivf/effect.h>

namespace ivf {

class ScanlineEffect : public Effect {
private:
    float m_lineSpacing{4.0f};
    float m_lineIntensity{0.3f};
    float m_scrollSpeed{0.0f};

public:
    ScanlineEffect();
    virtual ~ScanlineEffect() = default;

    static std::shared_ptr<ScanlineEffect> create();

    void setLineSpacing(float lineSpacing);
    void setLineIntensity(float lineIntensity);
    void setScrollSpeed(float scrollSpeed);

    float lineSpacing() const;
    float lineIntensity() const;
    float scrollSpeed() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<ScanlineEffect> ScanlineEffectPtr;

} // namespace ivf
