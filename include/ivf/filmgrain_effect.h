#pragma once

#include <ivf/effect.h>

namespace ivf {

class FilmgrainEffect : public Effect {
private:
    float m_blurRadius{0.0f};

public:
    FilmgrainEffect();

    static std::shared_ptr<FilmgrainEffect> create();

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<FilmgrainEffect> FilmgrainEffectPtr;

} // namespace ivf
