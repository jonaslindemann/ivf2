#include <ivf/bounding_box.h>

#include <algorithm>

using namespace ivf;

BoundingBox::BoundingBox()
{
    m_min = glm::vec3(1.0e6, 1.0e6, 1.0e6);
    m_max = glm::vec3(-1.0e6, -1.0e6, -1.0e6);
}
void BoundingBox::add(glm::vec3 pos)
{
    m_min.x = std::min(m_min.x, pos.x);
    m_min.y = std::min(m_min.y, pos.y);
    m_min.z = std::min(m_min.z, pos.z);
    m_max.x = std::max(m_max.x, pos.x);
    m_max.y = std::max(m_max.y, pos.y);
    m_max.z = std::max(m_max.z, pos.z);
}
glm::vec3 BoundingBox::min()
{
    return m_min;
}

glm::vec3 BoundingBox::max()
{
    return m_max;
}

glm::vec3 BoundingBox::center()
{
    return 0.5f * (m_min + m_max);
}

glm::vec3 BoundingBox::size()
{
    return m_max - m_min;
}
