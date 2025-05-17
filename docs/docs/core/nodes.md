# Node System

The ivf2 node system is the foundation of the scene graph hierarchy in the library.

## Overview

Ivf2 uses a scene graph system to manage the objects in your scene. The scene graph is a hierarchical structure that represents the objects in your scene and their relationships to each other. It allows you to organize your objects in a way that makes it easy to manage and render them.

The scene graph system in Ivf2 is built around the concept of nodes, which represent the objects in your scene. All nodes in Ivf2 derives from the `Node` and `TransformNode` classes, which provide a common interface for all nodes such as orientation, assigning an id, setting a parent, setting visibility and other properties such as material and texture. Rendering of the node is implemented by derived classed in the `doPreDraw()`, `doDraw()` , and `doPostDraw()` methods. These methods are called when calling the `draw()` method on the node, allowing you to implement custom rendering logic for your nodes. The Node class should not be used directly, but rather as a base class for your own nodes. Position, rotation, and scaling is handled by the derived `Node` class `TransformNode` class. The `TransformNode` class provides methods for setting and getting the position, rotation, and scaling of the node. It also provides methods for converting between world and local coordinates.

## Node Types

There are 4 types of nodes in the Ivf2 scene graph system:

- `MeshNode`: Represents a mesh in the scene. It is used to render 3D models and can be used to create complex scenes with multiple meshes.
- `CompositeNode` / `Transform`: Represents a group of nodes in the scene. It is used to create complex scenes with multiple nodes and can be used to create hierarchical structures.
- `InstanceNode`: Represents an instance of a mesh in the scene. It is used to create multiple instances of a mesh with different transformations and properties.
- `TextNode`: Represents a text in the scene. It is used to render text in the scene and can be used to create complex scenes with multiple text nodes.

## Node Base Class

The `Node` class is the base class for all scene graph elements in ivf2. It provides common functionality such as:

- Rendering capabilities
- Material assignment
- Visibility control

## TransformNode Base Class

The `TransformNode` class allows for coordinate transformations that affect all of its children.

```cpp
auto transform = Transform::create();
transform->setPos(glm::vec3(1.0, 0.0, 0.0));
transform->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
transform->setRotAngle(45.0);
```

## CompositeNode/Transform Class

The `CompositeNode` class is a container for multiple child nodes, implementing the Composite pattern.

```cpp
auto composite = CompositeNode::create();
composite->add(node1);
composite->add(node2);
```

Calling `draw()` on a `CompositeNode` will recursively call `draw()` on all its children.
This allows you to create complex scenes with multiple nodes and can be used to create hierarchical structures. In the example below, we create a car with 4 wheels using the `CompositeNode` and `Transform` classes:

```cpp
auto car = CompositeNode::create();

auto tire = Cylinder::create(0.5f, 0.2f); 
auto rim = Cylinder::create(0.5f, 0.1f); 

auto wheel1 = Transform::create();
wheel1->add(tire);
wheel1->add(rim);
wheel1->setPos(glm::vec3(0.0, 0.0, 1.0)); 

auto wheel2 = Transform::create();
wheel2->add(tire);
wheel2->add(rim);
wheel2->setPos(glm::vec3(0.0, 0.0, -1.0));

auto wheel3 = Transform::create();
wheel3->add(tire);
wheel3->add(rim);
wheel3->setPos(glm::vec3(1.0, 0.0, 0.0));

auto wheel4 = Transform::create();
wheel4->add(tire);
wheel4->add(rim);
wheel4->setPos(glm::vec3(-1.0, 0.0, 0.0));

car->add(wheel1);
car->add(wheel2);
car->add(wheel3);
car->add(wheel4);
```

If child nodes don't have a material assigned, the parent node's material will be used. 

Individual child nodes can be accessed using the `at(...)` method:

```cpp
auto child = composite->at(0); // Get the first child node
```
You can also iterate over all child nodes using a range-based for loop:

```cpp
for (auto& child : composite) 
{
    // Do something with each child node
}
```

Nodes can be removed using the `remove()` method:

```cpp
composite->remove(node1);
```

You can also clear all child nodes using the `clear()` method:

```cpp
composite->clear();
```

## MeshNode Base Class

The `MeshNode` class is a specialized node for creating and implemented 3D meshes. It provides easy to use methods for implementing nodes such as cubes, spheres, and cylinders. To implement a custom node based on the `MeshNode` class, you need to implement the `doSetup()` method, which is called when the node is created and refreshed. This method is used to set up the mesh data for the node. The `MeshNode` class provides methods for creating vertices, colors, normals, and indices for the mesh. The methods mimic the OpenGL API, so you can use them to create meshes in a way that is similar to how you would render in immediate mode OpenGL. A typical `doSetup()` method would look like this:

```cpp
void MyNode::doSetup()
{
    this->clear();
    this->newMesh(24, 12); // Create a new mesh with 24 vertices and 12 indices

    // Set up the mesh data
    mesh()->begin(GL_TRIANGLES);
    
    // Define vertices, colors, normals, and indices here
    // Example:
    mesh()->vertex3d(0.0f, 0.0f, 0.0f);
    mesh()->color3f(1.0f, 0.0f, 0.0f);
    mesh()->index3i(0, 1, 2);

    mesh()->end(); // End the mesh definition
}
```

A more complete example is shown below, where we implement a `Cube` class that inherits from `MeshNode`. The `Cube` class creates a cube mesh with a specified size and color for each face.

```cpp
#include <ivf/cube.h>

using namespace ivf;

Cube::Cube(GLfloat size) : m_size(size)
{
    this->newMesh(24, 12);
    this->doSetup();
}

// ...

void Cube::setSize(GLfloat size)
{
    m_size = size;
    this->refresh();
}

GLfloat Cube::size()
{
    return m_size;
}

void Cube::doSetup()
{
    //   y  o--------o
    //   ^ /|       /|
    //   |/ |     2/ |
    //  3o--------o  |
    //   |  o-----|--o
    //   | /      | /
    //   |/       |/
    //   o--------o --> x
    //   0        1

    this->clear();
    this->newMesh(24, 12);

    double n = m_size / 2.0;

    mesh()->begin(GL_TRIANGLES);
    mesh()->vertex3d(-n, -n, n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);
    mesh()->vertex3d(n, -n, n);
    mesh()->color3f(1.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, n, n);
    mesh()->color3f(1.0f, 1.0f, 0.0f);
    mesh()->vertex3d(-n, n, n);
    mesh()->color3f(1.0f, 1.0f, 1.0f);

    mesh()->index3i(0, 1, 2); // front
    mesh()->index3i(0, 2, 3);

    mesh()->vertex3d(-n, -n, -n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(-n, n, -n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(4, 6, 5); // back
    mesh()->index3i(4, 7, 6);

    mesh()->vertex3d(-n, -n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(-n, n, n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(-n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(-n, -n, -n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(8, 9, 10);
    mesh()->index3i(8, 10, 11); // left

    mesh()->vertex3d(n, -n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(n, n, n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(12, 13, 14);
    mesh()->index3i(12, 14, 15); // right

    mesh()->vertex3d(-n, n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(n, n, n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(-n, n, -n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(16, 17, 18);
    mesh()->index3i(16, 18, 19); // right

    mesh()->vertex3d(-n, -n, n);
    mesh()->color3f(0.0f, 0.0f, 1.0f);
    mesh()->vertex3d(-n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 0.0f);
    mesh()->vertex3d(n, -n, -n);
    mesh()->color3f(0.0f, 1.0f, 1.0f);
    mesh()->vertex3d(n, -n, n);
    mesh()->color3f(1.0f, 0.0f, 0.0f);

    mesh()->index3i(20, 21, 22);
    mesh()->index3i(20, 22, 23); // right

    mesh()->end();
}
```

## InstanceNode Class

The `InstanceNode` class is a specialized node for rendering instances of a mesh. It allows you to create multiple instances of a mesh with different transformations and properties.


## Working with Nodes

### Creating Nodes

Most node classes in ivf2 follow the factory pattern with static `create()` methods:

```cpp
auto cube = Cube::create();
auto sphere = Sphere::create(1.0f); // radius
```

### Node Hierarchy

Building a scene graph hierarchy:

```cpp
auto scene = CompositeNode::create();
auto transform = Transform::create();
auto cube = Cube::create();

transform->add(cube);
scene->add(transform);
```