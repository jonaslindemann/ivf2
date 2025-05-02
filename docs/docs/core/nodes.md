# Node System

The ivf2 node system is the foundation of the scene graph hierarchy in the library.

## Overview

The node system in ivf2 follows a classic scene graph architecture, where each node can have child nodes and a single parent node. This hierarchical structure allows for complex scenes to be built from simple components.

## Node Base Class

The `Node` class is the base class for all scene graph elements in ivf2. It provides common functionality such as:

- Position, rotation, and scale transformations
- Parent-child relationships
- Rendering capabilities
- Material assignment
- Visibility control

## Key Node Types

### TransformNode

The `TransformNode` class allows for coordinate transformations that affect all of its children.

```cpp
auto transform = ivf::Transform::create();
transform->setPos(glm::vec3(1.0, 0.0, 0.0));
transform->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
transform->setRotAngle(45.0);
```

### CompositeNode

The `CompositeNode` class is a container for multiple child nodes, implementing the Composite pattern.

```cpp
auto composite = ivf::CompositeNode::create();
composite->add(node1);
composite->add(node2);
```

### Other Node Types

- `Geometry` - Base class for all geometric entities
- `Light` - Base class for light sources
- `Camera` - Defines the viewpoint and projection

## Working with Nodes

### Creating Nodes

Most node classes in ivf2 follow the factory pattern with static `create()` methods:

```cpp
auto cube = ivf::Cube::create();
auto sphere = ivf::Sphere::create(1.0f); // radius
```

### Node Hierarchy

Building a scene graph hierarchy:

```cpp
auto scene = ivf::CompositeNode::create();
auto transform = ivf::Transform::create();
auto cube = ivf::Cube::create();

transform->add(cube);
scene->add(transform);
```