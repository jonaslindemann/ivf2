#pragma once

#include <ivf/deformer.h>

namespace ivf {

/**
 * @class ScaleDeformer
 * @brief Deformer that scales mesh vertices around a center point, with optional falloff.
 *
 * The ScaleDeformer class applies a scaling transformation to mesh vertices relative to a
 * specified center. The scaling can be uniform or non-uniform along each axis, and a falloff
 * parameter can be used to attenuate the effect based on distance from the center.
 */
class ScaleDeformer : public Deformer {
private:
    glm::vec3 m_center; ///< Center point for scaling.
    glm::vec3 m_scale;  ///< Scale factors for each axis.
    float m_falloff;    ///< Falloff factor for distance-based scaling.

public:
    /**
     * @brief Constructor.
     * @param center Center point for scaling.
     * @param scale Scale factors for each axis.
     */
    ScaleDeformer(const glm::vec3 &center = glm::vec3(0, 0, 0), const glm::vec3 &scale = glm::vec3(1, 1, 1));

    /**
     * @brief Factory method to create a shared pointer to a ScaleDeformer instance.
     * @param center Center point for scaling.
     * @param scale Scale factors for each axis.
     * @return std::shared_ptr<ScaleDeformer> New ScaleDeformer instance.
     */
    static std::shared_ptr<ScaleDeformer> create(const glm::vec3 &center = glm::vec3(0, 0, 0),
                                                 const glm::vec3 &scale = glm::vec3(1, 1, 1));

    /**
     * @brief Set the center point for scaling.
     * @param center Center point.
     */
    void setCenter(const glm::vec3 &center);

    /**
     * @brief Set the scale factors for each axis.
     * @param scale Scale vector.
     */
    void setScale(const glm::vec3 &scale);

    /**
     * @brief Set the falloff factor for distance-based scaling.
     * @param falloff Falloff value.
     */
    void setFalloff(float falloff);

    /**
     * @brief Apply the scaling deformation to the input vertices.
     */
    virtual void apply() override;

    /**
     * @brief Create a copy of the deformer for animation keyframes.
     * @return Unique pointer to the cloned Deformer.
     */
    virtual std::unique_ptr<Deformer> clone() const override;
};

} // namespace ivf
