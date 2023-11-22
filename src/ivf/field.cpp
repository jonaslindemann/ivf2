#include <ivf/field.h>

#include <iostream>

using namespace ivf;
using namespace std;

Field::Field(GLuint rows, GLuint cols)
{
    m_size[0] = rows;
    m_size[1] = cols;
}

Field::~Field()
{
    
}

std::shared_ptr<Field> ivf::Field::create(GLuint rows, GLuint cols)
{
    return std::make_shared<Field>(rows, cols);
}

GLuint Field::rows()
{
    return m_size[0];
}

GLuint Field::cols()
{
    return m_size[1];
}

GLuint Field::size()
{
    return m_size[0]*m_size[1];
}

void Field::zero()
{
    
}

size_t Field::memSize()
{
    return 0;
}

void* Field::data()
{
    return 0;
}

GLenum Field::dataType()
{
	return GL_NONE;
}

void ivf::Field::print()
{
}
