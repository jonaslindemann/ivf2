#pragma once

#include <ivf/base.h>

#include <cstdlib>

#include <glad/glad.h>

namespace ivf {

/**
 * @class Field
 * @brief Abstract base class for 2D data fields (e.g., vertex, color, or index buffers).
 *
 * The Field class provides a generic interface for managing 2D data arrays, such as those used
 * for vertex attributes, colors, or indices in graphics applications. It supports querying the
 * field's dimensions, zeroing data, memory size calculation, and data access. Derived classes
 * should implement the storage and data access logic for specific data types.
 */
class Field : public Base {
protected:
    GLuint m_size[2] = {0, 0}; ///< Dimensions of the field: [rows, columns].

public:
    /**
     * @brief Default constructor.
     */
    Field() = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Field() = default;

    /**
     * @brief Get the number of rows in the field.
     * @return GLuint Number of rows.
     */
    GLuint rows() const;

    /**
     * @brief Get the number of columns in the field.
     * @return GLuint Number of columns.
     */
    GLuint cols() const;

    /**
     * @brief Get the total number of elements in the field.
     * @return GLuint Total size (rows * columns).
     */
    GLuint size() const;

    /**
     * @brief Set all field data to zero.
     */
    virtual void zero();

    /**
     * @brief Get the memory size of the field in bytes.
     * @return size_t Memory size in bytes.
     */
    virtual size_t memSize();

    /**
     * @brief Get a pointer to the field's raw data.
     * @return void* Pointer to the data.
     */
    virtual void *data();

    /**
     * @brief Get the OpenGL data type of the field's elements.
     * @return GLenum OpenGL data type (e.g., GL_FLOAT, GL_UNSIGNED_INT).
     */
    virtual GLenum dataType();

    /**
     * @brief Print the field's contents for debugging.
     */
    virtual void print();
};

}; // namespace ivf
