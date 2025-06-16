#pragma once

#include <ivf/float_field.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class Normals
 * @brief Stores and manages an array of vertex normals for 3D meshes.
 *
 * The Normals class provides a convenient interface for storing and manipulating a collection
 * of vertex normals, each represented as a 3D vector (x, y, z). It inherits from FloatField,
 * which provides the underlying storage and access methods for floating-point data.
 */
class Normals : public FloatField {
public:
    /**
     * @brief Constructor.
     * @param nNormals Number of normals to allocate.
     */
    Normals(GLuint nNormals);

    /**
     * @brief Factory method to create a shared pointer to a Normals instance.
     * @param nNormals Number of normals to allocate.
     * @return std::shared_ptr<Normals> New Normals instance.
     */
    static std::shared_ptr<Normals> create(GLuint nNormals);

    /**
     * @brief Set the normal vector at the specified index.
     * @param idx Index of the normal to set.
     * @param nx X component.
     * @param ny Y component.
     * @param nz Z component.
     */
    void setNormal(GLuint idx, GLfloat nx, GLfloat ny, GLfloat nz);

    /**
     * @brief Get the normal vector at the specified index.
     * @param idx Index of the normal to retrieve.
     * @param nx Reference to store the X component.
     * @param ny Reference to store the Y component.
     * @param nz Reference to store the Z component.
     */
    void getNormal(GLuint idx, GLfloat &nx, GLfloat &ny, GLfloat &nz);

    /**
     * @brief Get the normal vector at the specified index as a glm::vec3.
     * @param idx Index of the normal to retrieve.
     * @return glm::vec3 The normal vector.
     */
    glm::vec3 normal(GLuint idx);

    /**
     * @brief Set the normal vector at the specified index using a glm::vec3.
     * @param idx Index of the normal to set.
     * @param normal The normal vector.
     */
    void setNormal(GLuint idx, glm::vec3 normal);
};

/**
 * @typedef NormalsPtr
 * @brief Shared pointer type for Normals.
 */
typedef std::shared_ptr<Normals> NormalsPtr;

}; // namespace ivf
