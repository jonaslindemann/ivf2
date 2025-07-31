#include <ivf/bounding_box.h>

#include <algorithm>
#include <limits>

using namespace ivf;

BoundingBox::BoundingBox()
{
    clear();
}

BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max)
    : m_min(min), m_max(max)
{
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

void BoundingBox::add(const BoundingBox& other)
{
    if (!other.isValid())
        return;
        
    if (!isValid())
    {
        m_min = other.m_min;
        m_max = other.m_max;
    }
    else
    {
        add(other.m_min);
        add(other.m_max);
    }
}

BoundingBox BoundingBox::transform(const glm::mat4& transform) const
{
    if (!isValid())
        return BoundingBox();
        
    BoundingBox result;
    
    // Transform all 8 corners and compute new AABB
    auto cornerVerts = corners();
    for (const auto& corner : cornerVerts)
    {
        glm::vec4 transformedCorner = transform * glm::vec4(corner, 1.0f);
        result.add(glm::vec3(transformedCorner) / transformedCorner.w);
    }
    
    return result;
}

glm::vec3 BoundingBox::min() const
{
    return m_min;
}

glm::vec3 BoundingBox::max() const
{
    return m_max;
}

glm::vec3 BoundingBox::center() const
{
    return 0.5f * (m_min + m_max);
}

glm::vec3 BoundingBox::size() const
{
    return m_max - m_min;
}

glm::vec3 BoundingBox::extent() const
{
    return 0.5f * size();
}

bool BoundingBox::isValid() const
{
    return m_min.x <= m_max.x && m_min.y <= m_max.y && m_min.z <= m_max.z;
}

bool BoundingBox::isEmpty() const
{
    return !isValid() || (m_min == m_max);
}

bool BoundingBox::contains(const glm::vec3& point) const
{
    if (!isValid())
        return false;
        
    return point.x >= m_min.x && point.x <= m_max.x &&
           point.y >= m_min.y && point.y <= m_max.y &&
           point.z >= m_min.z && point.z <= m_max.z;
}

bool BoundingBox::intersects(const BoundingBox& other) const
{
    if (!isValid() || !other.isValid())
        return false;
        
    return m_min.x <= other.m_max.x && m_max.x >= other.m_min.x &&
           m_min.y <= other.m_max.y && m_max.y >= other.m_min.y &&
           m_min.z <= other.m_max.z && m_max.z >= other.m_min.z;
}

std::vector<glm::vec3> BoundingBox::corners() const
{
    return {
        glm::vec3(m_min.x, m_min.y, m_min.z), // 0: min corner
        glm::vec3(m_max.x, m_min.y, m_min.z), // 1: +x
        glm::vec3(m_min.x, m_max.y, m_min.z), // 2: +y
        glm::vec3(m_max.x, m_max.y, m_min.z), // 3: +x+y
        glm::vec3(m_min.x, m_min.y, m_max.z), // 4: +z
        glm::vec3(m_max.x, m_min.y, m_max.z), // 5: +x+z
        glm::vec3(m_min.x, m_max.y, m_max.z), // 6: +y+z
        glm::vec3(m_max.x, m_max.y, m_max.z)  // 7: max corner
    };
}

void BoundingBox::clear()
{
    m_min = glm::vec3(std::numeric_limits<float>::max());
    m_max = glm::vec3(std::numeric_limits<float>::lowest());
}

void BoundingBox::setMin(glm::vec3 min)
{
    m_min = min;
}

void BoundingBox::setMax(glm::vec3 max)
{
    m_max = max;
}

void BoundingBox::setBounds(const glm::vec3& min, const glm::vec3& max)
{
    m_min = min;
    m_max = max;
}
