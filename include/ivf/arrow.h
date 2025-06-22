#pragma once

#include <ivf/arrow.h>
#include <ivf/capped_cone.h>
#include <ivf/capped_cylinder.h>
#include <ivf/composite_node.h>

namespace ivf {

/**
 * @enum ArrowType
 * @brief Specifies the type of arrow to be rendered.
 *
 * - DoubleSided: Arrowheads on both ends.
 * - LeftSided: Arrowhead only on the left side.
 * - RightSided: Arrowhead only on the right side.
 */
enum class ArrowType {
    DoubleSided, ///< Arrowheads on both ends.
    LeftSided,   ///< Arrowhead only on the left side.
    RightSided   ///< Arrowhead only on the right side.
};

/**
 * @class Arrow
 * @brief Composite node representing a 3D arrow with customizable body and arrowheads.
 *
 * The Arrow class allows creation and manipulation of a 3D arrow, which consists of a cylindrical body
 * and one or two conical arrowheads. The arrow can be configured to be double-sided, left-sided, or right-sided.
 * The dimensions of the body and cones can be adjusted, and the geometry is updated via the refresh() method.
 */
class Arrow : public CompositeNode {
private:
    ArrowType m_arrowType;          ///< Type of the arrow (double-sided, left-sided, right-sided).
    CappedCylinderPtr m_arrowBody;  ///< Pointer to the cylindrical body of the arrow.
    CappedConePtr m_leftArrowCone;  ///< Pointer to the left arrowhead cone.
    CappedConePtr m_rightArrowCone; ///< Pointer to the right arrowhead cone.
    GLfloat m_length;               ///< Total length of the arrow.
    GLfloat m_bodyRadius;           ///< Radius of the arrow's cylindrical body.
    GLfloat m_coneRadius;           ///< Radius of the arrowhead cones.
    GLfloat m_coneLength;           ///< Length of the arrowhead cones.

public:
    /**
     * @brief Default constructor.
     */
    Arrow();

    /**
     * @brief Factory method to create a shared pointer to an Arrow instance.
     * @return std::shared_ptr<Arrow> New Arrow instance.
     */
    static std::shared_ptr<Arrow> create();

    /**
     * @brief Set the type of the arrow (double-sided, left-sided, right-sided).
     * @param arrowType The desired ArrowType.
     */
    void setArrowType(ArrowType arrowType);

    /**
     * @brief Get the current arrow type.
     * @return ArrowType The current arrow type.
     */
    ArrowType arrowType() const;

    /**
     * @brief Set the total length of the arrow.
     * @param length The new length.
     */
    void setLength(GLfloat length);

    /**
     * @brief Set the radius of the arrow's cylindrical body.
     * @param radius The new body radius.
     */
    void setBodyRadius(GLfloat radius);

    /**
     * @brief Set the radius of the arrowhead cones.
     * @param radius The new cone radius.
     */
    void setConeRadius(GLfloat radius);

    /**
     * @brief Set the length of the arrowhead cones.
     * @param length The new cone length.
     */
    void setConeLength(GLfloat length);

    /**
     * @brief Get the total length of the arrow.
     * @return GLfloat The arrow's length.
     */
    GLfloat length() const;

    /**
     * @brief Get the radius of the arrow's cylindrical body.
     * @return GLfloat The body radius.
     */
    GLfloat bodyRadius() const;

    /**
     * @brief Get the radius of the arrowhead cones.
     * @return GLfloat The cone radius.
     */
    GLfloat coneRadius() const;

    /**
     * @brief Get the length of the arrowhead cones.
     * @return GLfloat The cone length.
     */
    GLfloat coneLength() const;

    /**
     * @brief Update the geometry of the arrow to reflect current parameters.
     *
     * Call this method after changing any of the arrow's properties to update its shape.
     */
    void refresh();

protected:
    /**
     * @brief Internal setup method for initializing the arrow's components.
     *
     * This method is called during construction and when the arrow is refreshed.
     */
    virtual void doSetup();

    virtual void doInitialize();
};

/**
 * @typedef ArrowPtr
 * @brief Shared pointer type for Arrow.
 */
typedef std::shared_ptr<Arrow> ArrowPtr;

} // namespace ivf
