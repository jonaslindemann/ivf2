# Basic primitives

Ivf2 provides a set of basic primitives that can be used to create 3D objects in the scene. These primitives are implemented as `MeshNode` subclasses and can be added to the scene graph like any other node. The following basic primitives are available:

- **Box**: A rectangular prism defined by its width, height, and depth. It can be created using the `Box::create()` method.
- **Sphere**: A sphere defined by its radius. It can be created using the `Sphere::create()` method.
- **Plane**: A flat surface defined by its width and height. It can be created using the `Plane::create()` method.
- **Cylinder**: A cylinder defined by its radius and height. It can be created using the `Cylinder::create()` method.
- **Cone**: A cone defined by its radius and height. It can be created using the `Cone::create()` method.
- **RoundedBox**: A box with rounded edges defined by its width, height, depth, and radius of the rounded corners. It can be created using the `RoundedBox::create()` method.
- **Disk**: A flat disk defined by its radius and number of segments. It can be created using the `Disk::create()` method.
- **Dodecahedron**: A 12-sided polyhedron defined by its radius. It can be created using the `Dodecahedron::create()` method.
- **CappedCylinder**: A cylinder with a cap on each end defined by its radius and height. It can be created using the `CappedCylinder::create()` method.
- **CappedCone**: A cone with a cap on the base defined by its radius and height. It can be created using the `CappedCone::create()` method.
- **Tube**: A tube defined by its inner and outer radius and height. It can be created using the `Tube::create()` method.
- **CappedTube**: A tube with caps on both ends defined by its inner and outer radius and height. It can be created using the `CappedTube::create()` method.
- **Capsule**: A capsule defined by its radius and height. It can be created using the `Capsule::create()` method.

All of these primitives have been implemented by the excellent library [Generator](https://github.com/ilmola/generator). 

## Working with basic primitives

As the geometry of these primitives are quite costly to create it is recommended to create them once and reuse them in the scene. The `MeshNode` class provides methods to set the position, rotation, and scale of the primitive, as well as to apply materials and textures. Most of the primitive classes have a set method which allows you to set all the parameters at once. For example, the `Box` class has a `set` method that allows you to set the width, height, and depth of the box in one call and the node geometry will be refreshed automatically. The following code shows an example of how to create a box and set its properties:

```cpp
auto box = Box::create();
box->set(glm::vec3(1.0f, 2.0f, 3.0f)); // Set width, height, depth (refreshes automatically)
box->setSize(glm::vec3(1.0f, 2.0f, 3.0f)); // Set size (does not refresh automatically)
box->refresh(); // Refresh the geometry manually if needed
```

The `refresh()` method can be used to update the geometry of the primitive if any of its properties have changed. This is useful when you want to change the size or shape of the primitive after it has been created. The refresh method will call the virtual `doSetup()` method of the primitive, which will update the vertex buffer and other properties of the primitive.

## Other primitives

There are some other useful primitives that are not part of the basic primitives that have specific use cases. These include:

- **Axis**: A simple axis representation that can be used to visualize the orientation of objects in the scene. It can be created using the `Axis::create()` method.
- **Grid**: A grid representation that can be used to visualize the ground plane or a reference grid in the scene. It can be created using the `Grid::create()` method.
- **LineGrid**: A grid representation that can be used to visualize a grid of lines in the scene. It can be created using the `LineGrid::create()` method.
- **LineTrace**: A line trace representation that can be used to visualize a line in the scene. It can be created using the `LineTrace::create()` method.
