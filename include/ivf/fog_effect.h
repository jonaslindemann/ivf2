#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class FogEffect
 * @brief Depth-based distance and height fog post-processing effect.
 *
 * Reconstructs each pixel's position from the scene depth buffer and blends
 * the fog color in with exponential falloff over distance. With a height
 * falloff the fog is densest at the base height and thins out above it, which
 * gives ground mist; density is integrated along the view ray, so it also looks
 * right with the camera inside the fog layer.
 *
 * Pixels where nothing was drawn (the background) receive skyAmount() of fog.
 * Because fog is applied in the effect chain, put it before effects such as
 * bloom or vignette.
 *
 * @code
 * auto fog = FogEffect::create();
 * fog->setColor({0.05f, 0.04f, 0.10f});
 * fog->setDensity(0.08f);
 * fog->setHeightFalloff(0.6f);
 * fog->load();
 * addEffect(fog);
 * @endcode
 */
class FogEffect : public Effect {
private:
    glm::vec3 m_color{0.5f, 0.6f, 0.7f}; ///< Fog color.
    float m_density{0.05f};              ///< Fog density per world unit at the base height.
    float m_start{0.0f};                 ///< Distance from the camera before fog begins.
    float m_heightFalloff{0.0f};         ///< How fast fog thins above the base height (0 = uniform fog).
    float m_baseHeight{0.0f};            ///< World height where the fog is densest.
    float m_maxOpacity{1.0f};            ///< Upper limit of the fog amount [0, 1].
    float m_skyAmount{1.0f};             ///< Fog amount applied to the background [0, 1].

public:
    FogEffect();

    static std::shared_ptr<FogEffect> create();

    void setColor(glm::vec3 color);
    glm::vec3 color() const;

    void setDensity(float density);
    float density() const;

    void setStart(float start);
    float start() const;

    void setHeightFalloff(float falloff);
    float heightFalloff() const;

    void setBaseHeight(float height);
    float baseHeight() const;

    void setMaxOpacity(float opacity);
    float maxOpacity() const;

    void setSkyAmount(float amount);
    float skyAmount() const;

protected:
    virtual void doLoad();
    virtual void doUpdateParams();
    virtual void setupProperties() override;
};

typedef std::shared_ptr<FogEffect> FogEffectPtr;

} // namespace ivf
