#pragma once

#include <ivf/effect.h>

namespace ivf {

class FilmgrainEffect : public Effect {
private:
    float m_noiseIntensity{0.1};
    float m_grainBlending{0.1};

public:
    FilmgrainEffect();

    static std::shared_ptr<FilmgrainEffect> create();

    void setNoiseIntensity(float noiseIntensity);
    float noiseIntensity() const;

    void setGrainBlending(float grainBlending);
    float grainBlending() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<FilmgrainEffect> FilmgrainEffectPtr;

} // namespace ivf
