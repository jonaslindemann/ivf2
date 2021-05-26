#include <ivf/floatfield.h>

using namespace ivf;

FloatField::FloatField(GLuint rows, GLuint cols)
:Field(rows, cols)
{
    auto fieldSize = rows * cols;
    m_data = new GLfloat[fieldSize];
}

FloatField::~FloatField()
{
    delete [] m_data;
}

std::shared_ptr<FloatField> ivf::FloatField::create(GLuint rows, GLuint cols)
{
    return std::make_shared<FloatField>(rows, cols);
}

void FloatField::zero()
{
    for (GLuint i=0; i<m_size[0]*m_size[1]; i++)
        m_data[i] = 0.0;
}

void* FloatField::data()
{
    return m_data;
}

GLfloat FloatField::at(GLuint row, GLuint col)
{
    if ((col + m_size[1]*row)<(m_size[0]*m_size[1]))
        return m_data[col+m_size[1]*row];
    else
        return -1.0;
}

void FloatField::set(GLuint row, GLuint col, GLfloat value)
{
    if ((col + m_size[1]*row)<(m_size[0]*m_size[1]))
        m_data[col+m_size[1]*row] = value;
}

GLfloat ivf::FloatField::get(GLuint row, GLuint col)
{
    if ((col + m_size[1] * row) < (m_size[0] * m_size[1]))
        return m_data[col + m_size[1] * row];
    else
        return 0.0f;
}

size_t FloatField::memSize()
{
    return this->size()*sizeof(GLfloat);
}

GLenum FloatField::dataType()
{
	return GL_FLOAT;
}
