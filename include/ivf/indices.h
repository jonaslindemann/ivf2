#pragma once

#include <ivf/int_field.h>

namespace ivf {

/**
 * @class Indices
 * @brief Stores and manages index data for mesh primitives.
 *
 * The Indices class provides a convenient interface for storing and manipulating index data
 * (such as for triangles, quads, or polygons) used in mesh rendering. It inherits from IntField,
 * which provides the underlying storage and access methods for integer data.
 */
class Indices : public IntField {
public:
    /**
     * @brief Constructor.
     * @param nIndices Number of index entries (rows).
     * @param cols Number of indices per entry (columns).
     */
    Indices(GLuint nIndices, GLuint cols);

    /**
     * @brief Factory method to create a shared pointer to an Indices instance.
     * @param nIndices Number of index entries (rows).
     * @param cols Number of indices per entry (columns).
     * @return std::shared_ptr<Indices> New Indices instance.
     */
    static std::shared_ptr<Indices> create(GLuint nIndices, GLuint cols);

    /**
     * @brief Set a single index value at the specified entry.
     * @param idx Entry index.
     * @param i0 Index value.
     */
    void setIndex(GLuint idx, GLuint i0);

    /**
     * @brief Set two index values at the specified entry.
     * @param idx Entry index.
     * @param i0 First index value.
     * @param i1 Second index value.
     */
    void setIndex(GLuint idx, GLuint i0, GLuint i1);

    /**
     * @brief Set three index values at the specified entry.
     * @param idx Entry index.
     * @param i0 First index value.
     * @param i1 Second index value.
     * @param i2 Third index value.
     */
    void setIndex(GLuint idx, GLuint i0, GLuint i1, GLuint i2);

    /**
     * @brief Set four index values at the specified entry.
     * @param idx Entry index.
     * @param i0 First index value.
     * @param i1 Second index value.
     * @param i2 Third index value.
     * @param i3 Fourth index value.
     */
    void setIndex(GLuint idx, GLuint i0, GLuint i1, GLuint i2, GLuint i3);

    /**
     * @brief Get a single index value from the specified entry.
     * @param idx Entry index.
     * @param i0 Reference to store the index value.
     */
    void index(GLuint idx, GLuint &i0);

    /**
     * @brief Get two index values from the specified entry.
     * @param idx Entry index.
     * @param i0 Reference to store the first index value.
     * @param i1 Reference to store the second index value.
     */
    void index(GLuint idx, GLuint &i0, GLuint &i1);

    /**
     * @brief Get three index values from the specified entry.
     * @param idx Entry index.
     * @param i0 Reference to store the first index value.
     * @param i1 Reference to store the second index value.
     * @param i2 Reference to store the third index value.
     */
    void index(GLuint idx, GLuint &i0, GLuint &i1, GLuint &i2);

    /**
     * @brief Get four index values from the specified entry.
     * @param idx Entry index.
     * @param i0 Reference to store the first index value.
     * @param i1 Reference to store the second index value.
     * @param i2 Reference to store the third index value.
     * @param i3 Reference to store the fourth index value.
     */
    void index(GLuint idx, GLuint &i0, GLuint &i1, GLuint &i2, GLuint &i3);
};

/**
 * @typedef IndicesPtr
 * @brief Shared pointer type for Indices.
 */
typedef std::shared_ptr<Indices> IndicesPtr;
}; // namespace ivf
