# Node System

The ivf2 node system is the foundation of the scene graph hierarchy in the library.

## Overview

The node system is designed to represent a hierarchical structure of objects in a 3D scene. All nodes can be transformed, rendered, and manipulated. There are 5 types of nodes in the Ivf2 scene graph system:

- `TransformNode`: The base class for all nodes in the scene graph. It provides common functionality such as position, rotation, and scale transformations.
- `MeshNode`: Represents a mesh in the scene. It is used to render 3D models and can be used to create complex scenes with multiple meshes.
- `CompositeNode` / `Transform`: Represents a group of nodes in the scene. It is used to create complex scenes with multiple nodes and can be used to create hierarchical structures.
- `InstanceNode`: Represents an instance of a mesh in the scene. It is used to create multiple instances of a mesh with different transformations and properties.
- `TextNode`: Represents a text in the scene. It is used to render text in the scene and can be used to create complex scenes with multiple text nodes.

## Node Base class

The `Node` class is the base class for all nodes in the ivf2 scene graph. It provides common functionality such as:

- Rendering capabilities
- Material assignment
- Visibility control

`Node` is an abstract class and cannot be instantiated directly. It provides a common interface for all nodes in the scene graph.

The `Node` class provides the following methods:

- `setVisible(bool visible)`: Sets the visibility of the node. If the node is not visible, it will not be rendered.
- `isVisible()`: Returns true if the node is visible, false otherwise.

When a node is set to invisible, it will not be rendered. This is useful for controlling the visibility of nodes in the scene graph.

The `Node` class also provides methods for setting and getting the material of the node:

- `setMaterial(std::shared_ptr<Material> material);`: Sets the material of the node. The material is used to define the appearance of the node.
- `material()`: Returns the material of the node.
- `setUseMaterial(bool use)`: Sets whether the node should use the material. If set to false, the node will not use the material and will be rendered with a default material.
- `useMaterial()`: Returns true if the node is using the material, false otherwise.


The `Node` class also provides methods for setting and getting the texture of the node:

- `setTexture(std::shared_ptr<Texture> texture);`: Sets the texture of the node. The texture is used to define the appearance of the node.
- `texture()`: Returns the texture of the node.
- `setUseTexture(bool use)`: Sets whether the node should use the texture. If set to false, the node will not use the texture and will be rendered with a default texture.
- `useTexture()`: Returns true if the node is using the texture, false otherwise.

A node is rendered by calling its `draw()` method. The method itself calls the following virtual methods:

- `preDraw()`: Called before the node is drawn. This method can be overridden by derived classes to perform any necessary setup before rendering.
- `doDraw()`: Called to render the node. The method is called if the visible flag is set to true. This method should be overridden by derived classes to perform the actual rendering of the node.
- `postDraw()`: Called after the node is drawn. This method can be overridden by derived classes to perform any necessary cleanup after rendering.

The example below shows an example of how to implement a custom node class that derives from the `Node` class:

```cpp
class CustomNode : public Node {
public:
    CustomNode() {
        // Constructor implementation
        setUseMaterial(true);
        setUseTexture(true);
    }

    void preDraw() override {
        // Custom pre-draw implementation
    }

    void doDraw() override {
        // Custom draw implementation

        // For example, rendering a custom mesh

        // ...

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); // be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // ... 

        x += (ch.glyphAdvance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }        
    }

    void postDraw() override {
        // Custom post-draw implementation
    }
};
```



## TransformNode Base Class

The `TransformNode` class is the base class for all scene graph elements in ivf2. It provides common functionality such as:

- Position, rotation, and scale transformations

The `TransformNode` derivde classes can all be transformed in 3D space. The most basic operation is to set the position, rotation, and scale of the node as shown below:

```cpp
auto sphere = Sphere::create();
transform->setPos(glm::vec3(1.0, 0.0, 0.0));
transform->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
transform->setRotAngle(45.0);
```

## CompositeNode

The `CompositeNode` or `Transform` classes are containers for multiple child nodes, implementing the Composite pattern.

```cpp
auto composite = CompositeNode::create();
composite->add(node1);
composite->add(node2);
```

## MeshNode

The `MeshNode` class is a specialized node that represents a 3D mesh in the scene. The mesh node is an abstract class and cannot be instantiated directly. The purpose of the `MeshNode` class is to provide a common interface for implementing different types of nodes that represent 3D meshes in the scene. The `MeshNode` class provides methods for easily creating meshes using a similar approach as the classic immediate mode rendering in OpenGL.

The key method of the `MeshNode` class is `doSetup()`, which is called to setup the mesh for rendering. The method is not called directly. Instead it is called when the object is created or refreshed.

The `MeshNode` class provides the following methods for creating meshes:

- `clear()`: Clears all mesh data. This method is called when the object is created or refreshed.
- `newMesh(int vsize, int isize)`: Creates a new mesh with `vsize` vertices and `isize` indices.
- `mesh()`: Returns the active `Nesh` object. 

The `Mesh` object has several methods for creating and manipulating meshes. The most important methods are:

- `begin(GLuint primType)`: Begins a new mesh with the specified primitive type. The method takes an argument that specifies the primitive type (e.g., `GL_TRIANGLES`, `GL_LINES`, etc.).
- `vertex3f(GLfloat x, GLfloat y, GLfloat z)`: Adds a vertex to the mesh with the specified coordinates. The method takes three arguments that specify the x, y, and z coordinates of the vertex.
- `vertex3d(GLdouble x, GLdouble y, GLdouble z)`: Adds a vertex to the mesh with the specified coordinates. The method takes three arguments that specify the x, y, and z coordinates of the vertex.
- `vertex2f(GLfloat x, GLfloat y)`: Adds a 2D vertex to the mesh with the specified coordinates. The method takes two arguments that specify the x and y coordinates of the vertex.
- `vertex2d(GLdouble x, GLdouble y)`: Adds a 2D vertex to the mesh with the specified coordinates. The method takes two arguments that specify the x and y coordinates of the vertex.
- `vertex3d(const glm::dvec3 &v)`: Adds a vertex to the mesh with the specified coordinates. The method takes a `glm::dvec3` object that specifies the x, y, and z coordinates of the vertex.
- `vertex3f(const glm::vec3 v)`: Adds a vertex to the mesh with the specified coordinates. The method takes a `glm::vec3` object that specifies the x, y, and z coordinates of the vertex.
- `color3f(GLfloat r, GLfloat g, GLfloat b)`: Adds a color to the mesh with the specified RGB values. The method takes three arguments that specify the red, green, and blue values of the color.- `color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)`: Adds a color to the mesh with the specified RGBA values. The method takes four arguments that specify the red, green, blue, and alpha values of the color.'
- `index1i(GLuint i0)`: Adds a single index to the mesh. The method takes a single argument that specifies the index.
- `index2i(GLuint i0, GLuint i1)`: Adds two indices to the mesh. The method takes two arguments that specify the indices.
- `index3i(GLuint i0, GLuint i1, GLuint i2)`: Adds three indices to the mesh. The method takes three arguments that specify the indices.
- `index4i(GLuint i0, GLuint i1, GLuint i2, GLuint i3)`: Adds four indices to the mesh. The method takes four arguments that specify the indices.
- `end()`: Ends the current mesh. This method is called to finalize the mesh and prepare it for rendering.

An example of how to implement a simple `MeshNode` derived class is shown below:

```cpp
class SimpleMeshNode : public MeshNode {
public:
    SimpleMeshNode() 
    {
        // Constructor implementation
        setUseMaterial(true);
        setUseTexture(true);

        doSetup();
    }

    void doSetup() override 
    {
        // Setup the mesh for rendering
        this->clear();
        this->newMesh(3, 3);
        
        mesh()->begin(GL_TRIANGLES);

        mesh()->vertex3f(0.0f, 1.0f, 0.0f);
        mesh()->vertex3f(-1.0f, -1.0f, 0.0f);
        mesh()->vertex3f(1.0f, -1.0f, 0.0f);

        mesh()->color3f(1.0f, 0.0f, 0.0f);
        mesh()->color3f(0.0f, 1.0f, 0.0f);
        mesh()->color3f(0.0f, 0.0f, 1.0f);

        mesh()->index3i(0, 1, 2);

        mesh()->end();
    }
};
```


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
auto scene = CompositeNode::create();
auto transform = Transform::create();
auto cube = Cube::create();

transform->add(cube);
scene->add(transform);
```