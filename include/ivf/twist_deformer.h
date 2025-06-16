#pragma once

#include <ivf/deformer.h>
#include <ivf/property_inspectable.h>

namespace ivf {

/**
 * @class TwistDeformer
 * @brief Deformer that applies a twist effect to mesh vertices.
 *
 * The TwistDeformer modifies vertex positions by rotating them around a specified axis,
 * with optional falloff and distance range controls.
 */
class TwistDeformer : public Deformer {
private:
    glm::vec3 m_axis;      ///< Twist axis (normalized)
    glm::vec3 m_center;    ///< Center point of twist
    float m_angle;         ///< Twist angle in radians
    float m_falloff;       ///< Distance falloff factor
    float m_startDistance; ///< Distance where twist starts
    float m_endDistance;   ///< Distance where twist ends

public:
    /**
     * @brief Construct a TwistDeformer with optional axis and center.
     * @param axis The twist axis (default: Y axis).
     * @param center The center point of the twist (default: origin).
     */
    TwistDeformer(const glm::vec3 &axis = glm::vec3(0, 1, 0), const glm::vec3 &center = glm::vec3(0, 0, 0));

    /**
     * @brief Create a shared pointer to a TwistDeformer.
     * @param axis The twist axis (default: Y axis).
     * @param center The center point of the twist (default: origin).
     * @return Shared pointer to a new TwistDeformer.
     */
    static std::shared_ptr<TwistDeformer> create(const glm::vec3 &axis = glm::vec3(0, 1, 0),
                                                 const glm::vec3 &center = glm::vec3(0, 0, 0));

    /**
     * @brief Set the twist axis.
     * @param axis The axis to twist around (should be normalized).
     */
    void setAxis(const glm::vec3 &axis);

    /**
     * @brief Get the twist axis.
     * @return The current twist axis.
     */
    glm::vec3 axis() const;

    /**
     * @brief Set the center point of the twist.
     * @param center The center point.
     */
    void setCenter(const glm::vec3 &center);

    /**
     * @brief Get the center point of the twist.
     * @return The current center point.
     */
    glm::vec3 center() const;

    /**
     * @brief Set the twist angle in radians.
     * @param angle The twist angle.
     */
    void setAngle(float angle);

    /**
     * @brief Get the twist angle in radians.
     * @return The current twist angle.
     */
    float angle() const;

    /**
     * @brief Set the falloff factor for the twist effect.
     * @param falloff The falloff factor.
     */
    void setFalloff(float falloff);

    /**
     * @brief Get the falloff factor for the twist effect.
     * @return The current falloff factor.
     */
    float falloff() const;

    /**
     * @brief Set the distance range where the twist is applied.
     * @param start The start distance.
     * @param end The end distance.
     */
    void setDistanceRange(float start, float end);

    /**
     * @brief Get the start distance for the twist effect.
     * @return The start distance.
     */
    float startDistance() const;

    /**
     * @brief Get the end distance for the twist effect.
     * @return The end distance.
     */
    float endDistance() const;

    /**
     * @brief Apply the twist deformation to the input vertices.
     */
    virtual void apply() override;

    /**
     * @brief Clone this deformer.
     * @return Unique pointer to a new TwistDeformer with the same parameters.
     */
    virtual std::unique_ptr<Deformer> clone() const override;

protected:
    /**
     * @brief Register properties for inspection.
     */
    virtual void setupProperties() override;
};

/**
 * @typedef TwistDeformerPtr
 * @brief Shared pointer to a TwistDeformer.
 */
typedef std::shared_ptr<TwistDeformer> TwistDeformerPtr;

}; // namespace ivf
