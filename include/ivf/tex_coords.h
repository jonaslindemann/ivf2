#pragma once

#include <ivf/float_field.h>

#include <glm/glm.hpp>

namespace ivf {

/**
 * @class TexCoords
 * @brief Stores and manages 2D texture coordinates for mesh vertices.
 *
 * The TexCoords class provides storage and access for 2D texture coordinates (s, t)
 * for each vertex in a mesh. It inherits from FloatField and offers convenient methods
 * for setting and retrieving texture coordinates as individual floats or as glm::vec2.
 */
class TexCoords : public FloatField {
public:
    /**
     * @brief Constructor.
     * @param nVertices Number of vertices (texture coordinate pairs).
     */
    TexCoords(GLuint nVertices);

    /**
     * @brief Factory method to create a shared pointer to a TexCoords instance.
     * @param nVertices Number of vertices (texture coordinate pairs).
     * @return std::shared_ptr<TexCoords> New TexCoords instance.
     */
    static std::shared_ptr<TexCoords> create(GLuint nVertices);

    /**
     * @brief Set the texture coordinate for a given vertex.
     * @param idx Vertex index.
     * @param s S (U) coordinate.
     * @param t T (V) coordinate.
     */
    void setTexCoord(GLuint idx, GLfloat s, GLfloat t);

    /**
     * @brief Get the texture coordinate for a given vertex as separate floats.
     * @param idx Vertex index.
     * @param s Output S (U) coordinate.
     * @param t Output T (V) coordinate.
     */
    void getTexCoord(GLuint idx, GLfloat &s, GLfloat &t);

    /**
     * @brief Get the texture coordinate for a given vertex as a glm::vec2.
     * @param idx Vertex index.
     * @return glm::vec2 Texture coordinate.
     */
    glm::vec2 texCoord(GLuint idx);
};

/**
 * @typedef TexCoordsPtr
 * @brief Shared pointer type for TexCoords.
 */
typedef std::shared_ptr<TexCoords> TexCoordsPtr;

}; // namespace ivf
