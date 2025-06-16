#pragma once

#include <ivf/float_field.h>

namespace ivf {

/**
 * @class Colors
 * @brief Stores and manages an array of RGBA color values.
 *
 * The Colors class provides a convenient interface for storing and manipulating a collection
 * of colors, each represented as RGBA floating-point values. It inherits from FloatField,
 * which provides the underlying storage and access methods.
 */
class Colors : public FloatField {
public:
    /**
     * @brief Constructor.
     * @param nColors Number of colors to allocate.
     */
    Colors(GLuint nColors);

    /**
     * @brief Factory method to create a shared pointer to a Colors instance.
     * @param nColors Number of colors to allocate.
     * @return std::shared_ptr<Colors> New Colors instance.
     */
    static std::shared_ptr<Colors> create(GLuint nColors);

    /**
     * @brief Set the RGBA value for a specific color index.
     * @param idx Index of the color to set.
     * @param r Red component (0.0 - 1.0).
     * @param g Green component (0.0 - 1.0).
     * @param b Blue component (0.0 - 1.0).
     * @param a Alpha component (0.0 - 1.0, default is 1.0).
     */
    void setColor(GLuint idx, GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0);

    /**
     * @brief Get the RGBA value for a specific color index.
     * @param idx Index of the color to retrieve.
     * @param r Reference to store the red component.
     * @param g Reference to store the green component.
     * @param b Reference to store the blue component.
     * @param a Reference to store the alpha component.
     */
    void getColor(GLuint idx, GLfloat &r, GLfloat &g, GLfloat &b, GLfloat &a);
};

/**
 * @typedef ColorsPtr
 * @brief Shared pointer type for Colors.
 */
typedef std::shared_ptr<Colors> ColorsPtr;

}; // namespace ivf
