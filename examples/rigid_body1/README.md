# Rigid Body Physics Example

This example demonstrates integration of ReactPhysics3D for rigid body dynamics simulation.

## Classes

### PhysicsObject (Base Class)

Base class for physics-enabled objects using ReactPhysics3D. Provides a convenient wrapper around ReactPhysics3D's RigidBody with GLM math types.

**Key Features:**
- Position and rotation management using `glm::vec3` and `glm::quat`
- Force and torque application
- Velocity control
- Mass, friction, and restitution (bounciness) properties
- Automatic conversion between GLM and ReactPhysics3D types

**Usage:**
```cpp
// PhysicsObject is abstract - use derived classes like PhysicsBox
```

### PhysicsBox (Derived Class)

A box-shaped physics object with collision detection.

**Key Features:**
- Configurable size (half-extents)
- Can be static (mass = 0) or dynamic (mass > 0)
- Material properties (friction, restitution)

**Usage:**
```cpp
// Create a dynamic box (1 kg mass, 1x1x1 size)
auto box = PhysicsBox::create(
    m_world,
    m_physicsCommon,
    glm::vec3(0.5f, 0.5f, 0.5f),  // Half-extents
    glm::vec3(0.0f, 5.0f, 0.0f),   // Position
    glm::quat(1.0f, 0.0f, 0.0f, 0.0f),  // Rotation
    1.0f  // Mass (kg)
);

// Set material properties
box->setRestitution(0.6f);  // Bounciness (0-1)
box->setFriction(0.3f);

// Update visual from physics
box->updateFromPhysics();
glm::vec3 pos = box->getPosition();
glm::quat rot = box->getRotation();

// Apply forces
box->applyForce(glm::vec3(0.0f, 10.0f, 0.0f));
box->setVelocity(glm::vec3(1.0f, 0.0f, 0.0f));
```

## Example Scene

The rigid_body1 example creates:
1. A static ground plane (20x1x20 box)
2. Five dynamic boxes that fall and bounce
3. Visual representation synchronized with physics

**Physics World Setup:**
```cpp
rp3d::PhysicsWorld::WorldSettings settings;
settings.defaultVelocitySolverNbIterations = 20;
settings.gravity = rp3d::Vector3(0, -9.81, 0);
m_world = m_physicsCommon.createPhysicsWorld(settings);
```

**Update Loop:**
```cpp
// In onUpdate()
float timeStep = 1.0f / 60.0f;
m_world->update(timeStep);

// Update visuals from physics
for (auto& physicsBox : m_physicsBoxes) {
    physicsBox->updateFromPhysics();
    glm::vec3 pos = physicsBox->getPosition();
    visualBox->setPos(pos);
}
```

## Extending the System

To create new physics shapes:

1. Derive from `PhysicsObject`
2. Implement `createCollisionShape()` method
3. Use PhysicsCommon to create the appropriate ReactPhysics3D shape

**Example - Sphere:**
```cpp
class PhysicsSphere : public PhysicsObject {
private:
    rp3d::SphereShape* m_sphereShape;
    rp3d::PhysicsCommon& m_physicsCommon;
    float m_radius;
    
public:
    virtual rp3d::CollisionShape* createCollisionShape() override {
        m_sphereShape = m_physicsCommon.createSphereShape(m_radius);
        return m_sphereShape;
    }
};
```

## API Reference

### PhysicsObject Methods

**Transform:**
- `glm::vec3 getPosition() const`
- `void setPosition(const glm::vec3&)`
- `glm::quat getRotation() const`
- `void setRotation(const glm::quat&)`
- `glm::vec3 getEulerAngles() const`
- `void setEulerAngles(const glm::vec3&)`

**Physics:**
- `void applyForce(const glm::vec3& force)`
- `void applyForceAtPoint(const glm::vec3& force, const glm::vec3& point)`
- `void applyTorque(const glm::vec3& torque)`
- `void setVelocity(const glm::vec3& velocity)`
- `glm::vec3 getVelocity() const`
- `void setAngularVelocity(const glm::vec3& angularVelocity)`
- `glm::vec3 getAngularVelocity() const`

**Properties:**
- `void setMass(float mass)`
- `float getMass() const`
- `void setRestitution(float restitution)`  // 0-1, bounciness
- `float getRestitution() const`
- `void setFriction(float friction)`
- `float getFriction() const`

**Synchronization:**
- `void updateFromPhysics()`  // Call after physics update

## Tips

1. **Half-Extents**: ReactPhysics3D uses half-extents for box sizes. A box with half-extents (0.5, 0.5, 0.5) is 1x1x1 in size.

2. **Static Objects**: Set mass to 0.0f to make objects immovable (like ground planes).

3. **Restitution**: 
   - 0.0 = no bounce (inelastic collision)
   - 1.0 = perfect bounce (elastic collision)
   - Typical values: 0.2-0.8

4. **Friction**: 
   - 0.0 = no friction (ice-like)
   - Higher values = more friction
   - Typical values: 0.3-0.7

5. **Performance**: Call `updateFromPhysics()` only once per frame after the physics update.

6. **Coordinate Systems**: The physics wrapper handles conversions between GLM (column-major) and ReactPhysics3D coordinate systems automatically.
