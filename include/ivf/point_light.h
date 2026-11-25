#pragma once

#include <ivf/light.h>

namespace ivf {

/**
 * @class PointLight
 * @brief Represents a point light source with configurable attenuation.
 *
 * The PointLight class extends the Light base class to represent a point light source
 * in 3D space. It supports constant, linear, and quadratic attenuation factors for
 * realistic light falloff. The class provides methods to configure attenuation and
 * applies its parameters to the rendering context.
 */
class PointLight : public Light {
private:
    float m_constAttenuation;     ///< Constant attenuation factor.
    float m_linearAttenuation;    ///< Linear attenuation factor.
    float m_quadraticAttenuation; ///< Quadratic attenuation factor.
    float m_shadowRange{0.0f};    ///< Far plane of the shadow faces (0 = automatic).

public:
    /**
     * @brief Default constructor.
     */
    PointLight();

    /**
     * @brief Factory method to create a shared pointer to a PointLight instance.
     * @return std::shared_ptr<PointLight> New PointLight instance.
     */
    [[nodiscard]] static std::shared_ptr<PointLight> create();

    /**
     * @brief Set the attenuation factors for the point light.
     * @param cnst Constant attenuation.
     * @param linear Linear attenuation.
     * @param quadratic Quadratic attenuation.
     */
    void setAttenuation(float cnst, float linear, float quadratic);

    /**
     * @brief Get the constant attenuation factor.
     * @return float Constant attenuation.
     */
    float constAttenuation() const;

    /**
     * @brief Get the linear attenuation factor.
     * @return float Linear attenuation.
     */
    float linearAttenutation();

    /**
     * @brief Get the quadratic attenuation factor.
     * @return float Quadratic attenuation.
     */
    float quadraticAttenuation() const;

    /**
     * @brief Apply the point light's parameters to the rendering context.
     */
    void apply() override;

    /**
     * @brief Distance up to which the light casts shadows (far plane of its six shadow
     * faces). 0, the default, reaches the farthest point of the scene.
     *
     * Point lights cast shadows in all directions with a cube of six shadow maps; enable
     * them with setCastShadows(true) (at most four shadow-casting lights in total).
     */
    void setShadowRange(float range);

    /** @brief Distance up to which the light casts shadows (0 = automatic). */
    [[nodiscard]] float shadowRange() const;
};

/**
 * @typedef PointLightPtr
 * @brief Shared pointer type for PointLight.
 */
typedef std::shared_ptr<PointLight> PointLightPtr;

} // namespace ivf
