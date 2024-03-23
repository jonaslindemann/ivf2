#include <ivf/vertex_buffer.h>

using namespace ivf;

#include <iostream>

using namespace std;

VertexBuffer::VertexBuffer()
{
    glGenBuffers(1, &m_id);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_id);
}

std::shared_ptr<VertexBuffer> ivf::VertexBuffer::create()
{
    return std::make_shared<VertexBuffer>();
}

void VertexBuffer::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::setArray(Field *field)
{
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, field->memSize(), field->data(), GL_STATIC_DRAW);
}
