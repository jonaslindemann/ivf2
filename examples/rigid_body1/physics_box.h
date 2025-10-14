/**
 * @file physics_box.h
 * @brief Box-shaped physics object for ReactPhysics3D
 * @author Jonas Lindemann
 */

#pragma once

#include "physics_object.h"
#include <glm/gtc/quaternion.hpp>

/**
 * @class PhysicsBox
 * @brief A box-shaped physics object
 * 
 * This class represents a box with physics properties using ReactPhysics3D's
 * BoxShape. It can be used for creating dynamic or static box objects in the
 * physics simulation.
 */
class PhysicsBox : public PhysicsObject {
private:
    rp3d::BoxShape* m_boxShape;  ///< Box collision shape
    
public:
    /**
     * @brief Constructor
     * @param world Pointer to ReactPhysics3D world
     * @param physicsCommon Reference to PhysicsCommon (for creating shapes)
     * @param size Half-extents of the box (half width, half height, half depth)
     * @param position Initial position
     * @param rotation Initial rotation
     * @param mass Mass of the box (0 for static)
     */
    PhysicsBox(rp3d::PhysicsWorld* world,
               rp3d::PhysicsCommon& physicsCommon,
               const glm::vec3& size = glm::vec3(0.5f, 0.5f, 0.5f),
               const glm::vec3& position = glm::vec3(0.0f),
               const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
               float mass = 1.0f);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~PhysicsBox() override;
    
    /**
     * @brief Create the box collision shape
     * @return Pointer to the created BoxShape
     */
    virtual rp3d::CollisionShape* createCollisionShape() override;
    
    /**
     * @brief Set the size of the box
     * @param size New half-extents (half width, half height, half depth)
     * 
     * Note: This recreates the collision shape, so should be done before initialize()
     * or the object needs to be reinitialized after changing size.
     */
    void setSize(const glm::vec3& size);
    
    /**
     * @brief Get the current size
     * @return Current half-extents
     */
    glm::vec3 getSize() const { return m_size; }
    
    /**
     * @brief Create a new PhysicsBox instance
     * @param world Pointer to ReactPhysics3D world
     * @param physicsCommon Reference to PhysicsCommon
     * @param size Half-extents of the box
     * @param position Initial position
     * @param rotation Initial rotation
     * @param mass Mass of the box
     * @return Shared pointer to the created PhysicsBox
     */
    static std::shared_ptr<PhysicsBox> create(
        rp3d::PhysicsWorld* world,
        rp3d::PhysicsCommon& physicsCommon,
        const glm::vec3& size = glm::vec3(0.5f, 0.5f, 0.5f),
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        float mass = 1.0f);

private:
    rp3d::PhysicsCommon& m_physicsCommon;  ///< Reference to PhysicsCommon for shape creation
};

using PhysicsBoxPtr = std::shared_ptr<PhysicsBox>;
