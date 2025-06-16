#pragma once

#include <memory>

#include <ivf/deformer.h>
#include <ivf/property_inspectable.h>

namespace ivf {

/**
 * @class BendDeformer
 * @brief Deformer that applies a bend effect to mesh vertices.
 *
 * The BendDeformer modifies vertex positions by bending them around a specified axis,
 * with optional curvature and distance range controls.
 */
class BendDeformer : public Deformer {
private:
    glm::vec3 m_axis;      ///< Bend axis (should be normalized)
    glm::vec3 m_center;    ///< Center point of the bend
    float m_curvature;     ///< Bend curvature (radians per unit distance)
    float m_startDistance; ///< Distance where bend starts
    float m_endDistance;   ///< Distance where bend ends

public:
    /**
     * @brief Construct a BendDeformer with optional axis and center.
     * @param axis The bend axis (default: Y axis).
     * @param center The center point of the bend (default: origin).
     */
    BendDeformer(const glm::vec3 &axis = glm::vec3(0, 1, 0), const glm::vec3 &center = glm::vec3(0, 0, 0));

    /**
     * @brief Create a shared pointer to a BendDeformer.
     * @param axis The bend axis (default: Y axis).
     * @param center The center point of the bend (default: origin).
     * @return Shared pointer to a new BendDeformer.
     */
    static std::shared_ptr<BendDeformer> create(const glm::vec3 &axis = glm::vec3(0, 1, 0),
                                                const glm::vec3 &center = glm::vec3(0, 0, 0));

    /**
     * @brief Set the bend axis.
     * @param axis The axis to bend around (should be normalized).
     */
    void setAxis(const glm::vec3 &axis);

    /**
     * @brief Set the center point of the bend.
     * @param center The center point.
     */
    void setCenter(const glm::vec3 &center);

    /**
     * @brief Set the curvature of the bend.
     * @param curvature The curvature (radians per unit distance).
     */
    void setCurvature(float curvature);

    /**
     * @brief Set the distance range where the bend is applied.
     * @param start The start distance.
     * @param end The end distance.
     */
    void setDistanceRange(float start, float end);

    /**
     * @brief Apply the bend deformation to the input vertices.
     */
    virtual void apply() override;

    /**
     * @brief Clone this deformer.
     * @return Unique pointer to a new BendDeformer with the same parameters.
     */
    virtual std::unique_ptr<Deformer> clone() const override;

protected:
    /**
     * @brief Register properties for inspection.
     */
    virtual void setupProperties() override;
};

/**
 * @typedef BendDeformerPtr
 * @brief Shared pointer to a BendDeformer.
 */
typedef std::shared_ptr<BendDeformer> BendDeformerPtr;

}; // namespace ivf
