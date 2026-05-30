#pragma once

#include <ivf/effect.h>
#include <glm/glm.hpp>

namespace ivf {

class NightVisionEffect : public Effect {
private:
    float m_noiseIntensity{0.05f};
    float m_glowStrength{0.3f};
    glm::vec3 m_phosphorColor{0.1f, 1.0f, 0.1f};

public:
    NightVisionEffect();
    virtual ~NightVisionEffect() = default;

    static std::shared_ptr<NightVisionEffect> create();

    void setNoiseIntensity(float noiseIntensity);
    void setGlowStrength(float glowStrength);
    void setPhosphorColor(glm::vec3 phosphorColor);

    float noiseIntensity() const;
    float glowStrength() const;
    glm::vec3 phosphorColor() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<NightVisionEffect> NightVisionEffectPtr;

} // namespace ivf
