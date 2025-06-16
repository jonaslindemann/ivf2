#pragma once

/**
 * @file vertex_array.h
 * @brief Declares the VertexArray class for managing OpenGL vertex array objects in the ivf library.
 */

#include <ivf/glbase.h>

namespace ivf {

/**
 * @class VertexArray
 * @brief Wrapper for OpenGL Vertex Array Object (VAO) management.
 *
 * The VertexArray class encapsulates the creation, binding, and destruction of OpenGL VAOs.
 * It provides a convenient and RAII-safe interface for managing VAO resources in the ivf library.
 */
class VertexArray : public GLBase {
private:
    GLuint m_id; ///< OpenGL Vertex Array Object ID.

public:
    /**
     * @brief Default constructor. Generates a new OpenGL VAO.
     */
    VertexArray();

    /**
     * @brief Destructor. Deletes the OpenGL VAO.
     */
    virtual ~VertexArray();

    /**
     * @brief Factory method to create a shared pointer to a VertexArray instance.
     * @return std::shared_ptr<VertexArray> New VertexArray instance.
     */
    static std::shared_ptr<VertexArray> create();

    /**
     * @brief Bind the vertex array object to the OpenGL context.
     */
    void bind();

    /**
     * @brief Unbind the vertex array object from the OpenGL context.
     */
    void unbind();
};

/**
 * @typedef VertexArrayPtr
 * @brief Shared pointer type for VertexArray.
 */
typedef std::shared_ptr<VertexArray> VertexArrayPtr;

}; // namespace ivf
