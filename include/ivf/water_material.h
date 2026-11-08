#pragma once

/**
 * @file water_material.h
 * @brief Reflective water surface material using a PlanarReflection.
 */

#include <ivf/planar_reflection.h>
#include <ivf/shader_material.h>

#include <glm/glm.hpp>

#include <memory>

namespace ivf {

/**
 * @class WaterMaterial
 * @brief ShaderMaterial for a reflecting water surface with animated ripples.
 *
 * Blends the PlanarReflection texture with a water color using a Fresnel term
 * (more reflection at grazing angles) and distorts the reflection with ripple
 * normals. Ripples are animated by the `time` uniform; set it every frame with
 * ShaderMaterial::setGlobalUniform("time", t) or setUniform("time", t). With
 * ripple strength 0 the surface is a perfect mirror.
 *
 * Use it on a horizontal surface lying in the reflection plane, e.g. a Plane.
 */
class WaterMaterial : public ShaderMaterial {
private:
    PlanarReflectionPtr m_reflection;

public:
    /**
     * @brief Constructor. Use create() so the shader program is set up.
     */
    WaterMaterial(PlanarReflectionPtr reflection = nullptr);

    /**
     * @brief Create a water material.
     * @param reflection Reflection rendered for the surface.
     * @return The material, or nullptr if the water shader failed to compile.
     */
    static std::shared_ptr<WaterMaterial> create(PlanarReflectionPtr reflection = nullptr);

    void setReflection(PlanarReflectionPtr reflection);
    [[nodiscard]] PlanarReflectionPtr reflection() const;

    /** @brief Color seen looking straight down into the water. */
    void setWaterColor(glm::vec3 color);
    /** @brief Reflection amount at grazing angles [0, 1]. */
    void setReflectivity(float reflectivity);
    /** @brief Reflection amount looking straight down [0, 1]. */
    void setMinReflectivity(float reflectivity);
    /** @brief How quickly reflection increases towards grazing angles. */
    void setFresnelPower(float power);
    /** @brief Height of the ripples; drives both shading and distortion. */
    void setRippleStrength(float strength);
    /** @brief Spatial frequency of the ripples (waves per world unit). */
    void setRippleScale(float scale);
    /** @brief Animation speed of the ripples. */
    void setRippleSpeed(float speed);
    /** @brief How strongly ripples offset the reflection lookup. */
    void setDistortion(float distortion);

    void apply() override;
};

using WaterMaterialPtr = std::shared_ptr<WaterMaterial>;

} // namespace ivf
