#pragma once

#include <ivf/effect.h>

namespace ivf {

/**
 * @class FilmgrainEffect
 * @brief Post-processing effect that simulates film grain noise.
 *
 * The FilmgrainEffect class implements a film grain effect, adding random noise and blending
 * to the rendered image to simulate the appearance of analog film. The effect is controlled
 * by noise intensity and grain blending parameters.
 *
 * Inherits from Effect and can be used as part of a rendering pipeline.
 */
class FilmgrainEffect : public Effect {
private:
    float m_noiseIntensity{0.1}; ///< Intensity of the noise applied to the image.
    float m_grainBlending{0.1};  ///< Blending factor for the grain effect.

public:
    /**
     * @brief Default constructor.
     */
    FilmgrainEffect();

    /**
     * @brief Factory method to create a shared pointer to a FilmgrainEffect instance.
     * @return std::shared_ptr<FilmgrainEffect> New FilmgrainEffect instance.
     */
    static std::shared_ptr<FilmgrainEffect> create();

    /**
     * @brief Set the intensity of the film grain noise.
     * @param noiseIntensity The strength of the noise.
     */
    void setNoiseIntensity(float noiseIntensity);

    /**
     * @brief Get the current noise intensity value.
     * @return float The current noise intensity.
     */
    float noiseIntensity() const;

    /**
     * @brief Set the blending factor for the grain effect.
     * @param grainBlending The blending strength.
     */
    void setGrainBlending(float grainBlending);

    /**
     * @brief Get the current grain blending value.
     * @return float The current grain blending factor.
     */
    float grainBlending() const;

protected:
    /**
     * @brief Load and initialize the film grain effect resources (e.g., shaders).
     */
    virtual void doLoad();

    /**
     * @brief Update the film grain effect parameters (e.g., noise intensity, grain blending) in the shader or effect
     * pipeline.
     */
    virtual void doUpdateParams();

    /**
     * @brief Setup properties for the film grain effect, such as noise intensity and grain blending.
     */
    virtual void setupProperties() override;
};

/**
 * @typedef FilmgrainEffectPtr
 * @brief Shared pointer type for FilmgrainEffect.
 */
typedef std::shared_ptr<FilmgrainEffect> FilmgrainEffectPtr;

} // namespace ivf
