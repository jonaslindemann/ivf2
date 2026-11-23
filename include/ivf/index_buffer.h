#pragma once

#include <ivf/glbase.h>
#include <ivf/field.h>

namespace ivf {

/**
 * @class IndexBuffer
 * @brief Manages an OpenGL index buffer object (IBO/EBO) for indexed drawing.
 *
 * The IndexBuffer class encapsulates the creation, binding, and management of an OpenGL
 * index buffer object, which stores indices for indexed rendering. It provides methods
 * to bind/unbind the buffer and upload index data from a Field.
 */
class IndexBuffer : public GLBase {
private:
    GLuint m_id; ///< OpenGL buffer object ID.
public:
    /**
     * @brief Default constructor.
     */
    IndexBuffer();

    /**
     * @brief Destructor.
     */
    virtual ~IndexBuffer();

    /**
     * @brief Factory method to create a shared pointer to an IndexBuffer instance.
     * @return std::shared_ptr<IndexBuffer> New IndexBuffer instance.
     */
    [[nodiscard]] static std::shared_ptr<IndexBuffer> create();

    /**
     * @brief Bind the index buffer for use in OpenGL.
     */
    void bind();

    /**
     * @brief Unbind the index buffer.
     */
    void unbind();

    /**
     * @brief Give up ownership of the OpenGL object without deleting it.
     *
     * Used when the object has to be deleted later in its own context (see
     * RenderContext::deleteLater()) or was already freed with its context.
     * @return The object name; this wrapper no longer refers to it.
     */
    [[nodiscard]] GLuint release();

    /**
     * @brief Upload index data from a Field to the buffer.
     * @param field Pointer to the Field containing index data.
     */
    void setArray(Field *field);
};

/**
 * @typedef IndexBufferPtr
 * @brief Shared pointer type for IndexBuffer.
 */
typedef std::shared_ptr<IndexBuffer> IndexBufferPtr;
}; // namespace ivf
