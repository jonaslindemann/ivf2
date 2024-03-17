#include <ivf/int_field.h>

#include <iostream>

using namespace ivf;
using namespace std;

IntField::IntField(GLuint rows, GLuint cols)
{
    m_size[0] = rows;
    m_size[1] = cols;
    auto fieldSize = rows * cols;
    m_data = std::make_unique<GLuint[]>(fieldSize);
}

ivf::IntField::IntField(const IntField &other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    auto fieldSize = m_size[0] * m_size[1];
    m_data = std::make_unique<GLuint[]>(fieldSize);
    std::copy(other.m_data.get(), other.m_data.get() + fieldSize, m_data.get());
}

IntField &ivf::IntField::operator=(const IntField &other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    auto fieldSize = m_size[0] * m_size[1];
    m_data = std::make_unique<GLuint[]>(fieldSize);
    std::copy(other.m_data.get(), other.m_data.get() + fieldSize, m_data.get());
    return *this;
}

ivf::IntField::IntField(IntField &&other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    m_data = std::move(other.m_data);
    other.m_size[0] = 0;
    other.m_size[1] = 0;
    other.m_data = nullptr;
}

IntField &ivf::IntField::operator=(IntField &&other)
{
    m_size[0] = other.m_size[0];
    m_size[1] = other.m_size[1];
    m_data = std::move(other.m_data);
    other.m_size[0] = 0;
    other.m_size[1] = 0;
    other.m_data = nullptr;
    return *this;
}

std::shared_ptr<IntField> ivf::IntField::create(GLuint rows, GLuint cols)
{
    return std::make_shared<IntField>(rows, cols);
}

void IntField::zero()
{
    for (GLuint i = 0; i < m_size[0] * m_size[1]; i++)
        m_data[i] = 0;
}

void *IntField::data()
{
    return m_data.get();
}

void ivf::IntField::print()
{
    cout << "---------" << endl;
    cout << "Integer field (" << this->rows() << ", " << this->cols() << ")" << endl;
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

GLuint IntField::at(GLuint row, GLuint col)
{
    if ((col + m_size[1] * row) < (m_size[0] * m_size[1]))
        return m_data[col + m_size[1] * row];
    else
        return -1;
}

GLuint ivf::IntField::at(GLuint pos)
{
    if (pos < (m_size[0] * m_size[1]))
        return m_data[pos];
    else
        return -1;
}

void IntField::set(GLuint row, GLuint col, GLuint value)
{
    if ((col + m_size[1] * row) < (m_size[0] * m_size[1]))
        m_data[col + m_size[1] * row] = value;
}

size_t IntField::memSize()
{
    return this->size() * sizeof(GLint);
}

GLenum IntField::dataType()
{
    return GL_UNSIGNED_INT;
}
