#pragma once

#include <ivf/glbase.h>
#include <ivf/field.h>

namespace ivf {

class VertexBuffer : public GLBase {
private:
    GLuint m_id{0};
    GLenum m_usage{GL_STATIC_DRAW};

public:
    VertexBuffer();
    virtual ~VertexBuffer();

    static std::shared_ptr<VertexBuffer> create();

    void setUsage(GLenum usage);
    GLenum usage() const;

    void bind();
    void unbind();

    void setArray(Field *field);
    void updateArray(Field *field);
};

typedef std::shared_ptr<VertexBuffer> VertexBufferPtr;

}; // namespace ivf
