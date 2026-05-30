#pragma once

#include <ivf/effect.h>

namespace ivf {

class PosterizeEffect : public Effect {
private:
    float m_levels{4.0f};

public:
    PosterizeEffect();
    virtual ~PosterizeEffect() = default;

    static std::shared_ptr<PosterizeEffect> create();

    void setLevels(float levels);
    float levels() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<PosterizeEffect> PosterizeEffectPtr;

} // namespace ivf
