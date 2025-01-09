#include <ivfmath/sphere.h>

#include <glm/geometric.hpp>

using namespace ivfmath;

void ivfmath::Sphere::setCenter(const glm::vec3 &center)
{
    m_center = center;
}

void ivfmath::Sphere::setCenter(float x, float y, float z)
{
    m_center = glm::vec3(x, y, z);
}

void ivfmath::Sphere::setRadius(float radius)
{
    m_radius = radius;
}

glm::vec3 ivfmath::Sphere::center() const
{
    return m_center;
}

float ivfmath::Sphere::radius() const
{
    return m_radius;
}

bool ivfmath::Sphere::contains(const glm::vec3 &point) const
{
    glm::vec3 difference = point - m_center;
    float distanceSquared = glm::dot(difference, difference);
    return distanceSquared <= m_radius * m_radius;
}

bool ivfmath::Sphere::contains(float x, float y, float z) const
{
    return contains(glm::vec3(x, y, z));
}

bool ivfmath::Sphere::contains(const Sphere &sphere) const
{
    glm::vec3 difference = sphere.center() - m_center;
    float distanceSquared = glm::dot(difference, difference);
    float sumRadii = m_radius + sphere.radius();
    return distanceSquared <= sumRadii * sumRadii;
}
