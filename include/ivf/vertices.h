#pragma once

/**
 * @file vertices.h
 * @brief Declares the Vertices class for managing 3D vertex data in the ivf library.
 */

#include <ivf/float_field.h>
#include <glm/glm.hpp>

namespace ivf {

/**
 * @class Vertices
 * @brief 3D vertex array for mesh geometry.
 *
 * The Vertices class provides storage and access for an array of 3D vertex positions.
 * Inherits from FloatField and offers convenient methods for setting and retrieving
 * vertex coordinates as individual floats or glm::vec3 objects.
 */
class Vertices : public FloatField {
public:
    /**
     * @brief Construct a Vertices object with the specified number of vertices.
     * @param nVertices Number of 3D vertices.
     */
    Vertices(GLuint nVertices);

    /**
     * @brief Factory method to create a shared pointer to a Vertices instance.
     * @param nVertices Number of 3D vertices.
     * @return std::shared_ptr<Vertices> New Vertices instance.
     */
    static std::shared_ptr<Vertices> create(GLuint nVertices);

    /**
     * @brief Set the coordinates of a vertex.
     * @param idx Index of the vertex.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     */
    void setVertex(GLuint idx, GLfloat x, GLfloat y, GLfloat z);

    /**
     * @brief Get the coordinates of a vertex.
     * @param idx Index of the vertex.
     * @param x Reference to store X coordinate.
     * @param y Reference to store Y coordinate.
     * @param z Reference to store Z coordinate.
     */
    void getVertex(GLuint idx, GLfloat &x, GLfloat &y, GLfloat &z);

    /**
     * @brief Get the coordinates of a vertex as a glm::vec3.
     * @param idx Index of the vertex.
     * @return glm::vec3 Vertex coordinates.
     */
    glm::vec3 vertex(GLuint idx);
};

/**
 * @typedef VerticesPtr
 * @brief Shared pointer type for Vertices.
 */
typedef std::shared_ptr<Vertices> VerticesPtr;

}; // namespace ivf
