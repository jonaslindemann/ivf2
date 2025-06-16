#pragma once

#include <ivf/field.h>

#include <glad/glad.h>

namespace ivf {

/**
 * @class FloatField
 * @brief 2D data field for storing floating-point values (GLfloat).
 *
 * The FloatField class provides storage and access for 2D arrays of floating-point values,
 * commonly used for vertex attributes, colors, or other per-vertex data in graphics applications.
 * Inherits from Field and implements storage, access, and utility methods for float data.
 */
class FloatField : public Field {
private:
    std::unique_ptr<GLfloat[]> m_data; ///< Pointer to the field's float data.

public:
    /**
     * @brief Constructor.
     * @param rows Number of rows in the field.
     * @param cols Number of columns in the field.
     */
    FloatField(GLuint rows, GLuint cols);

    /**
     * @brief Copy constructor.
     * @param other FloatField to copy from.
     */
    FloatField(const FloatField &other);

    /**
     * @brief Copy assignment operator.
     * @param other FloatField to copy from.
     * @return Reference to this FloatField.
     */
    FloatField &operator=(const FloatField &other);

    /**
     * @brief Move constructor.
     * @param other FloatField to move from.
     */
    FloatField(FloatField &&other);

    /**
     * @brief Move assignment operator.
     * @param other FloatField to move from.
     * @return Reference to this FloatField.
     */
    FloatField &operator=(FloatField &&other);

    /**
     * @brief Factory method to create a shared pointer to a FloatField instance.
     * @param rows Number of rows in the field.
     * @param cols Number of columns in the field.
     * @return std::shared_ptr<FloatField> New FloatField instance.
     */
    static std::shared_ptr<FloatField> create(GLuint rows, GLuint cols);

    /**
     * @brief Get the value at the specified row and column.
     * @param rows Row index.
     * @param cols Column index.
     * @return GLfloat Value at the specified position.
     */
    GLfloat at(GLuint rows, GLuint cols);

    /**
     * @brief Get the value at the specified linear position.
     * @param pos Linear index.
     * @return GLfloat Value at the specified position.
     */
    GLfloat at(GLuint pos);

    /**
     * @brief Set the value at the specified row and column.
     * @param rows Row index.
     * @param cols Column index.
     * @param value Value to set.
     */
    void set(GLuint rows, GLuint cols, GLfloat value);

    /**
     * @brief Get the value at the specified row and column.
     * @param rows Row index.
     * @param cols Column index.
     * @return GLfloat Value at the specified position.
     */
    GLfloat get(GLuint rows, GLuint cols);

    /**
     * @brief Get the memory size of the field in bytes.
     * @return size_t Memory size in bytes.
     */
    virtual size_t memSize();

    /**
     * @brief Set all field data to zero.
     */
    virtual void zero();

    /**
     * @brief Get a pointer to the field's raw data.
     * @return void* Pointer to the data.
     */
    virtual void *data();

    /**
     * @brief Get the OpenGL data type of the field's elements.
     * @return GLenum OpenGL data type (GL_FLOAT).
     */
    virtual GLenum dataType();

    /**
     * @brief Print the field's contents for debugging.
     */
    virtual void print();
};

/**
 * @typedef FloatFieldPtr
 * @brief Shared pointer type for FloatField.
 */
typedef std::shared_ptr<FloatField> FloatFieldPtr;

}; // namespace ivf
