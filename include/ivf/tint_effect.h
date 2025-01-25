#pragma once

#include <ivf/effect.h>

namespace ivf {

class TintEffect : public Effect {
private:
    glm::vec3 m_tintColor;
    float m_tintStrength{0.5};
    glm::vec3 m_grayScaleWeights;

public:
    TintEffect();

    static std::shared_ptr<TintEffect> create();

    void setTintColor(glm::vec3 tintColor);
    glm::vec3 tintColor() const;

    void setTintStrength(float tintStrength);
    float tintStrength() const;

    void setGrayScaleWeights(glm::vec3 grayScaleWeights);
    glm::vec3 grayScaleWeights() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
};

typedef std::shared_ptr<TintEffect> TintEffectPtr;

} // namespace ivf
