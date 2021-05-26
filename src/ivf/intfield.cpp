#include <ivf/intfield.h>

using namespace ivf;

IntField::IntField(GLuint rows, GLuint cols)
:Field(rows, cols)
{
    auto fieldSize = rows * cols;
    m_data = new GLuint[fieldSize];
}

IntField::~IntField()
{
    delete [] m_data;
}

std::shared_ptr<IntField> ivf::IntField::create(GLuint rows, GLuint cols)
{
    return std::make_shared<IntField>(rows, cols);
}

void IntField::zero()
{
    for (GLuint i=0; i<m_size[0]*m_size[1]; i++)
        m_data[i] = 0;
}

void* IntField::data()
{
    return m_data;
}

GLuint IntField::at(GLuint row, GLuint col)
{
    if ((col + m_size[1]*row)<(m_size[0]*m_size[1]))
        return m_data[col+m_size[1]*row];
    else
        return -1;
}

void IntField::set(GLuint row, GLuint col, GLuint value)
{
    if ((col + m_size[1]*row)<(m_size[0]*m_size[1]))
        m_data[col+m_size[1]*row] = value;
}

size_t IntField::memSize()
{
    return this->size()*sizeof(GLint);
}

GLenum IntField::dataType()
{
	return GL_UNSIGNED_INT;
}
