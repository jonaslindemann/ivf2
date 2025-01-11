#pragma once

#include <ivf/gl.h>

namespace ivf {

class Effect : public GLBase {
private:
    float m_time{0.0};
    int m_width{0};
    int m_height{0};

public:
    Effect();
    virtual ~Effect();

    static std::shared_ptr<Effect> create();

    void setTime(float time);
    void setResolution(int width, int height);
    float time();
    int width();
    int height();

    void load();
    void use();

    ProgramPtr program();

protected:
    ProgramPtr m_program;

    virtual void doLoad(){};
    virtual void doUpdateParams(){};
};

typedef std::shared_ptr<Effect> EffectPtr;

} // namespace ivf
