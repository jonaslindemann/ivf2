#pragma once

#include <ivf/effect.h>
#include <glm/glm.hpp>

namespace ivf {

class ColorGradingEffect : public Effect {
private:
    glm::vec3 m_shadows{0.6f, 0.5f, 0.5f};
    glm::vec3 m_midtones{0.5f, 0.5f, 0.5f};
    glm::vec3 m_highlights{0.5f, 0.5f, 0.6f};
    float m_contrast{1.0f};
    float m_saturation{1.0f};

public:
    ColorGradingEffect();
    virtual ~ColorGradingEffect() = default;

    static std::shared_ptr<ColorGradingEffect> create();

    void setShadows(glm::vec3 shadows);
    void setMidtones(glm::vec3 midtones);
    void setHighlights(glm::vec3 highlights);
    void setContrast(float contrast);
    void setSaturation(float saturation);

    glm::vec3 shadows() const;
    glm::vec3 midtones() const;
    glm::vec3 highlights() const;
    float contrast() const;
    float saturation() const;

protected:
    virtual void doLoad() override;
    virtual void doUpdateParams() override;
    virtual void setupProperties() override;
};

typedef std::shared_ptr<ColorGradingEffect> ColorGradingEffectPtr;

} // namespace ivf
