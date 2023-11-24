#include <ivf/vertex_array.h>

using namespace ivf;

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

std::shared_ptr<VertexArray> ivf::VertexArray::create()
{
    return std::make_shared<VertexArray>();
}

void VertexArray::bind()
{
    glBindVertexArray(m_id);
}

void VertexArray::unbind()
{
    glBindVertexArray(0);
}