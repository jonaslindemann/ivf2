#include <ivf/float_field.h>

#include <iostream>

using namespace ivf;
using namespace std;

FloatField::FloatField(GLuint rows, GLuint cols)
{
    m_size[0] = rows;
    m_size[1] = cols;
    auto fieldSize = rows * cols;
    m_data = std::make_unique<GLfloat[]>(fieldSize);
}

ivf::FloatField::FloatField(const FloatField &other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    auto fieldSize = m_size[0] * m_size[1];
    m_data = std::make_unique<GLfloat[]>(fieldSize);
    std::copy(other.m_data.get(), other.m_data.get() + fieldSize, m_data.get());
}

FloatField &ivf::FloatField::operator=(const FloatField &other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    auto fieldSize = m_size[0] * m_size[1];
    m_data = std::make_unique<GLfloat[]>(fieldSize);
    std::copy(other.m_data.get(), other.m_data.get() + fieldSize, m_data.get());
    return *this;
}

ivf::FloatField::FloatField(FloatField &&other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    m_data = std::move(other.m_data);
    other.m_size[0] = 0;
    other.m_size[1] = 0;
    other.m_data = nullptr;
}

FloatField &ivf::FloatField::operator=(FloatField &&other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    m_data = std::move(other.m_data);
    other.m_size[0] = 0;
    other.m_size[1] = 0;
    other.m_data = nullptr;
    return *this;
}

std::shared_ptr<FloatField> ivf::FloatField::create(GLuint rows, GLuint cols)
{
    return std::make_shared<FloatField>(rows, cols);
}

void FloatField::zero()
{
    for (GLuint i = 0; i < m_size[0] * m_size[1]; i++)
        m_data[i] = 0.0;
}

void *FloatField::data()
{
    return m_data.get();
}

GLfloat FloatField::at(GLuint row, GLuint col)
{
    if ((col + m_size[1] * row) < (m_size[0] * m_size[1]))
        return m_data[col + m_size[1] * row];
    else
        return -1.0;
}

GLfloat ivf::FloatField::at(GLuint pos)
{
    if (pos < (m_size[0] * m_size[1]))
        return m_data[pos];
    else
        return -1.0;
}

void FloatField::set(GLuint row, GLuint col, GLfloat value)
{
    if ((col + m_size[1] * row) < (m_size[0] * m_size[1]))
        m_data[col + m_size[1] * row] = value;
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
    return this->size() * sizeof(GLfloat);
}

GLenum FloatField::dataType()
{
    return GL_FLOAT;
}

void ivf::FloatField::print()
{
    cout << "---------" << endl;
    cout << "Float field (" << this->rows() << ", " << this->cols() << ")" << endl;
    for (auto i = 0; i < this->rows(); i++)
    {
        for (auto j = 0; j < this->cols(); j++)
        {
            cout << this->at(i, j);
            if (j < this->cols() - 1)
                cout << ", ";
            else
                cout << endl;
        }
    }
}
