#include <ivf/matrix_stack.h>

using namespace ivf;

MatrixStack::MatrixStack()
{
    m_stack.push_back(glm::mat4(1.0));
}

void MatrixStack::loadIdentity()
{
    m_stack.back() = glm::mat4(1.0);
}

void MatrixStack::push()
{
    m_stack.push_back(m_stack.back());
}

void MatrixStack::pop()
{
    m_stack.pop_back();
}

void MatrixStack::multiply(glm::mat4 m)
{
    m_stack.back() = m_stack.back() * m;
}

void MatrixStack::translate(glm::vec3 pos)
{
    m_stack.back() = glm::translate(m_stack.back(), pos);
}

void MatrixStack::rotate(float angle, glm::vec3 axis)
{
    m_stack.back() = glm::rotate(m_stack.back(), angle, axis);
}

void MatrixStack::scale(glm::vec3 scale)
{
    m_stack.back() = glm::scale(m_stack.back(), scale);
}

void MatrixStack::clear()
{
    m_stack.clear();
    m_stack.push_back(glm::mat4(1.0));
}

size_t MatrixStack::size()
{
    return m_stack.size();
}

glm::mat4 MatrixStack::top()
{
    return m_stack.back();
}
