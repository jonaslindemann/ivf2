#pragma once

/**
 * @file vertex_buffer.h
 * @brief Declares the VertexBuffer class for managing OpenGL vertex buffer objects in the ivf library.
 */

#include <ivf/glbase.h>
#include <ivf/field.h>

namespace ivf {

/**
 * @class VertexBuffer
 * @brief Wrapper for OpenGL Vertex Buffer Object (VBO) management.
 *
 * The VertexBuffer class encapsulates the creation, binding, and management of OpenGL VBOs.
 * It supports setting and updating buffer data from Field objects, and allows configuration
 * of buffer usage for efficient graphics rendering.
 */
class VertexBuffer : public GLBase {
private:
    GLuint m_id{0};                 ///< OpenGL Vertex Buffer Object ID.
    GLenum m_usage{GL_STATIC_DRAW}; ///< OpenGL buffer usage hint.

public:
    /**
     * @brief Construct a VertexBuffer with the specified usage.
     * @param usage OpenGL buffer usage (default GL_STATIC_DRAW).
     */
    VertexBuffer(GLenum usage = GL_STATIC_DRAW);

    /**
     * @brief Destructor. Deletes the OpenGL VBO.
     */
    virtual ~VertexBuffer();

    /**
     * @brief Factory method to create a shared pointer to a VertexBuffer instance.
     * @return std::shared_ptr<VertexBuffer> New VertexBuffer instance.
     */
    static std::shared_ptr<VertexBuffer> create();

    /**
     * @brief Set the OpenGL buffer usage hint.
     * @param usage Usage flag (e.g., GL_STATIC_DRAW, GL_DYNAMIC_DRAW).
     */
    void setUsage(GLenum usage);

    /**
     * @brief Get the current OpenGL buffer usage hint.
     * @return GLenum Usage flag.
     */
    GLenum usage() const;

    /**
     * @brief Bind the vertex buffer object to the OpenGL context.
     */
    void bind();

    /**
     * @brief Unbind the vertex buffer object from the OpenGL context.
     */
    void unbind();

    /**
     * @brief Set the buffer data from a Field object.
     * @param field Pointer to the Field containing data to upload.
     */
    void setArray(Field *field);

    /**
     * @brief Update the buffer data from a Field object.
     * @param field Pointer to the Field containing data to update.
     */
    void updateArray(Field *field);
};

/**
 * @typedef VertexBufferPtr
 * @brief Shared pointer type for VertexBuffer.
 */
typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;

}; // namespace ivf
