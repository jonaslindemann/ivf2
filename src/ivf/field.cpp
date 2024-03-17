#include <ivf/field.h>

#include <iostream>

using namespace ivf;
using namespace std;

GLuint Field::rows() const
{
    return m_size[0];
}

GLuint Field::cols() const
{
    return m_size[1];
}

GLuint Field::size() const
{
    return m_size[0] * m_size[1];
}

void Field::zero()
{}

size_t Field::memSize()
{
    return 0;
}

void *Field::data()
{
    return nullptr;
}

GLenum Field::dataType()
{
    return GL_NONE;
}

void ivf::Field::print()
{}
