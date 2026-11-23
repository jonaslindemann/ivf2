#include <ivf/index_buffer.h>

using namespace ivf;

#include <iostream>

using namespace std;

IndexBuffer::IndexBuffer()
{
    glGenBuffers(1, &m_id);
}

IndexBuffer::~IndexBuffer()
{
    if (m_id != 0)
        glDeleteBuffers(1, &m_id);
}

GLuint IndexBuffer::release()
{
    const GLuint id = m_id;
    m_id = 0;
    return id;
}

std::shared_ptr<IndexBuffer> ivf::IndexBuffer::create()
{
    return std::make_shared<IndexBuffer>();
}

void IndexBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IndexBuffer::unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::setArray(Field* field)
{
    this->bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, field->memSize(), field->data(), GL_STATIC_DRAW);
}
