#pragma once

#include <ivf/field.h>

#include <glad/glad.h>

namespace ivf {

/**
 * @class IntField
 * @brief 2D data field for storing unsigned integer values (GLuint).
 *
 * The IntField class provides storage and access for 2D arrays of unsigned integer values,
 * commonly used for index buffers or other per-element integer data in graphics applications.
 * Inherits from Field and implements storage, access, and utility methods for integer data.
 */
class IntField : public Field {
private:
    std::unique_ptr<GLuint[]> m_data; ///< Pointer to the field's integer data.

public:
    /**
     * @brief Constructor.
     * @param rows Number of rows in the field.
     * @param cols Number of columns in the field.
     */
    IntField(GLuint rows, GLuint cols);

    /**
     * @brief Copy constructor.
     * @param other IntField to copy from.
     */
    IntField(const IntField &other);

    /**
     * @brief Move constructor.
     * @param other IntField to move from.
     */
    IntField(IntField &&other);

    /**
     * @brief Copy assignment operator.
     * @param other IntField to copy from.
     * @return Reference to this IntField.
     */
    IntField &operator=(const IntField &other);

    /**
     * @brief Move assignment operator.
     * @param other IntField to move from.
     * @return Reference to this IntField.
     */
    IntField &operator=(IntField &&other);

    /**
     * @brief Factory method to create a shared pointer to an IntField instance.
     * @param rows Number of rows in the field.
     * @param cols Number of columns in the field.
     * @return std::shared_ptr<IntField> New IntField instance.
     */
    static std::shared_ptr<IntField> create(GLuint rows, GLuint cols);

    /**
     * @brief Get the value at the specified row and column.
     * @param row Row index.
     * @param col Column index.
     * @return GLuint Value at the specified position.
     */
    GLuint at(GLuint row, GLuint col);

    /**
     * @brief Get the value at the specified linear position.
     * @param pos Linear index.
     * @return GLuint Value at the specified position.
     */
    GLuint at(GLuint pos);

    /**
     * @brief Set the value at the specified row and column.
     * @param row Row index.
     * @param col Column index.
     * @param value Value to set.
     */
    void set(GLuint row, GLuint col, GLuint value);

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
     * @brief Get the OpenGL data type of the field's elements.
     * @return GLenum OpenGL data type (GL_UNSIGNED_INT).
     */
    virtual GLenum dataType();

    /**
     * @brief Get a pointer to the field's raw data.
     * @return void* Pointer to the data.
     */
    virtual void *data();

    /**
     * @brief Print the field's contents for debugging.
     */
    virtual void print();
};

/**
 * @typedef IntFieldPtr
 * @brief Shared pointer type for IntField.
 */
typedef std::shared_ptr<IntField> IntFieldPtr;

}; // namespace ivf
