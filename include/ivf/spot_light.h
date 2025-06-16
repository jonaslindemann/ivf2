#pragma once

#include <ivf/light.h>

namespace ivf {

/**
 * @class SpotLight
 * @brief Represents a spot light source with direction, cutoff, and attenuation parameters.
 *
 * The SpotLight class extends the Light base class to provide a spot light source.
 * It supports setting the direction, cutoff angles (inner and outer), and attenuation
 * factors (constant, linear, quadratic). The class provides methods to configure and
 * query these parameters, and to apply the light's state to the rendering context.
 */
class SpotLight : public Light {
private:
    glm::vec3 m_direction;        ///< Direction of the spot light.
    float m_cutoff;               ///< Inner cutoff angle (in degrees or radians, depending on usage).
    float m_outerCutoff;          ///< Outer cutoff angle (in degrees or radians, depending on usage).
    float m_constAttenuation;     ///< Constant attenuation factor.
    float m_linearAttenuation;    ///< Linear attenuation factor.
    float m_quadraticAttenuation; ///< Quadratic attenuation factor.

public:
    /**
     * @brief Constructor.
     */
    SpotLight();

    /**
     * @brief Factory method to create a shared pointer to a SpotLight instance.
     * @return std::shared_ptr<SpotLight> New SpotLight instance.
     */
    static std::shared_ptr<SpotLight> create();

    /**
     * @brief Set the direction of the spot light.
     * @param direction Direction vector.
     */
    void setDirection(glm::vec3 direction);

    /**
     * @brief Set the attenuation factors for the spot light.
     * @param cnst Constant attenuation.
     * @param linear Linear attenuation.
     * @param quadratic Quadratic attenuation.
     */
    void setAttenuation(float cnst, float linear, float quadratic);

    /**
     * @brief Set the inner and outer cutoff angles for the spot light.
     * @param inner Inner cutoff angle.
     * @param outer Outer cutoff angle.
     */
    void setCutoff(float inner, float outer);

    /**
     * @brief Get the direction of the spot light.
     * @return glm::vec3 Direction vector.
     */
    glm::vec3 direction();

    /**
     * @brief Get the constant attenuation factor.
     * @return float Constant attenuation.
     */
    float constAttenuation();

    /**
     * @brief Get the linear attenuation factor.
     * @return float Linear attenuation.
     */
    float linearAttenutation();

    /**
     * @brief Get the quadratic attenuation factor.
     * @return float Quadratic attenuation.
     */
    float quadraticAttenuation();

    /**
     * @brief Get the inner cutoff angle.
     * @return float Inner cutoff angle.
     */
    float innerCutoff();

    /**
     * @brief Get the outer cutoff angle.
     * @return float Outer cutoff angle.
     */
    float outerCutoff();

    /**
     * @brief Apply the spot light's parameters to the rendering context.
     */
    void apply();
};

/**
 * @typedef SpotLightPtr
 * @brief Shared pointer type for SpotLight.
 */
typedef std::shared_ptr<SpotLight> SpotLightPtr;

} // namespace ivf
