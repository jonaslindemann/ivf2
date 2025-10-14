/**
 * @file physics_box.cpp
 * @brief Implementation of PhysicsBox class
 * @author Jonas Lindemann
 */

#include "physics_box.h"

PhysicsBox::PhysicsBox(rp3d::PhysicsWorld* world,
                       rp3d::PhysicsCommon& physicsCommon,
                       const glm::vec3& size,
                       const glm::vec3& position,
                       const glm::quat& rotation,
                       float mass)
    : PhysicsObject(world, position, rotation, mass)
    , m_physicsCommon(physicsCommon)
    , m_boxShape(nullptr)
{
    m_size = size;
}

PhysicsBox::~PhysicsBox()
{
    // Box shape will be destroyed automatically by PhysicsCommon
    m_boxShape = nullptr;
}

rp3d::CollisionShape* PhysicsBox::createCollisionShape()
{
    // Create box shape using half-extents
    rp3d::Vector3 halfExtents(m_size.x, m_size.y, m_size.z);
    m_boxShape = m_physicsCommon.createBoxShape(halfExtents);
    
    return m_boxShape;
}

void PhysicsBox::setSize(const glm::vec3& size)
{
    m_size = size;
    
    // Note: To change the size after initialization, you would need to:
    // 1. Remove the old collider
    // 2. Destroy the old shape
    // 3. Create a new shape with new size
    // 4. Add new collider
    // This is left as an exercise or can be implemented if needed
}

std::shared_ptr<PhysicsBox> PhysicsBox::create(
    rp3d::PhysicsWorld* world,
    rp3d::PhysicsCommon& physicsCommon,
    const glm::vec3& size,
    const glm::vec3& position,
    const glm::quat& rotation,
    float mass)
{
    auto box = std::make_shared<PhysicsBox>(world, physicsCommon, size, position, rotation, mass);
    box->initialize();
    return box;
}
