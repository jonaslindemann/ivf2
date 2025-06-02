#include <ivf/vertex_buffer.h>

using namespace ivf;

#include <iostream>

using namespace std;

VertexBuffer::VertexBuffer(GLenum usage) : m_usage{usage}
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

void ivf::VertexBuffer::setUsage(GLenum usage)
{
    m_usage = usage;
}

GLenum ivf::VertexBuffer::usage() const
{
    return m_usage;
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
    glBufferData(GL_ARRAY_BUFFER, field->memSize(), field->data(), m_usage);
}

void ivf::VertexBuffer::updateArray(Field *field)
{
    this->bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, field->memSize(), field->data());
}
