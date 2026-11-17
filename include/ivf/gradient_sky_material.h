#pragma once

/**
 * @file gradient_sky_material.h
 * @brief Procedural sky: zenith-to-horizon gradient, horizon glow and optional stars.
 */

#include <ivf/blend_shader_material.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivf {

/**
 * @class GradientSkyMaterial
 * @brief Background-mode BlendShaderMaterial for a sky dome.
 *
 * Put it on a Sphere and add that node first. The dome is always centered on
 * the camera, so it never comes closer; its radius only has to lie within the
 * camera's far plane. It is drawn without depth writes, so everything added
 * later covers it and depth-based effects such as FogEffect still see empty sky.
 *
 * The color blends from the horizon color to the zenith color going up and to
 * the ground color below the horizon. A glow band sits on the horizon. Stars
 * are off by default; they twinkle with the global `time` uniform
 * (ShaderMaterial::setGlobalUniform("time", t)).
 *
 * @code
 * auto sky = GradientSkyMaterial::create({0.0f, 0.0f, 0.02f}, {0.1f, 0.08f, 0.2f});
 * sky->setStarIntensity(1.0f);
 * auto dome = Sphere::create(50.0, 64, 32);
 * dome->setMaterial(sky);
 * add(dome); // first
 * @endcode
 */
class GradientSkyMaterial : public BlendShaderMaterial {
public:
    /**
     * @brief Constructor. Use create() so the shader program is set up.
     */
    GradientSkyMaterial(glm::vec3 zenithColor = glm::vec3(0.1f, 0.25f, 0.6f),
                        glm::vec3 horizonColor = glm::vec3(0.7f, 0.8f, 0.9f));

    /**
     * @brief Create a sky material.
     * @param zenithColor  Color straight up.
     * @param horizonColor Color at the horizon; also used below it until setGroundColor() is called.
     * @return The material, or nullptr if the sky shader failed to compile.
     */
    static std::shared_ptr<GradientSkyMaterial> create(glm::vec3 zenithColor = glm::vec3(0.1f, 0.25f, 0.6f),
                                                       glm::vec3 horizonColor = glm::vec3(0.7f, 0.8f, 0.9f));

    /** @brief Color straight up. */
    void setZenithColor(glm::vec3 color);
    [[nodiscard]] glm::vec3 zenithColor() const;

    /** @brief Color at the horizon. */
    void setHorizonColor(glm::vec3 color);
    [[nodiscard]] glm::vec3 horizonColor() const;

    /** @brief Color below the horizon. */
    void setGroundColor(glm::vec3 color);
    [[nodiscard]] glm::vec3 groundColor() const;

    /**
     * @brief Shape of the gradient going up.
     *
     * 1 blends evenly with height, smaller values keep the horizon color low (default 0.5).
     */
    void setGradientExponent(float exponent);
    [[nodiscard]] float gradientExponent() const;

    /** @brief Color added in a band along the horizon; black for none (the default). */
    void setGlowColor(glm::vec3 color);
    [[nodiscard]] glm::vec3 glowColor() const;

    /** @brief How narrow the horizon glow band is; higher is narrower (default 10). */
    void setGlowSharpness(float sharpness);
    [[nodiscard]] float glowSharpness() const;

    /** @brief Brightness of the stars; 0 hides them (the default). */
    void setStarIntensity(float intensity);
    [[nodiscard]] float starIntensity() const;

private:
    glm::vec3 m_zenithColor;
    glm::vec3 m_horizonColor;
    glm::vec3 m_groundColor;
    float m_gradientExponent{0.5f};
    glm::vec3 m_glowColor{0.0f};
    float m_glowSharpness{10.0f};
    float m_starIntensity{0.0f};
};

using GradientSkyMaterialPtr = std::shared_ptr<GradientSkyMaterial>;

} // namespace ivf
