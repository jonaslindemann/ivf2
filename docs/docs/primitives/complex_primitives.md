# Complex primitives

Ivf2 provides a set of complex primitives that can be used to create more advanced 3D objects in the scene. These primitives are also implemented as `MeshNode` subclasses and can be added to the scene graph like any other node. The following complex primitives are available:

- **SolidLine**: A solid line defined by its start and end points. It can be created using the `SolidLine::create()` method.
- **SolidPolyLine**: A 3D object created by extruding a 2D shape along a path. It can be created using the `SolidPolyLine::create()` method.

## SolidLine

The `SolidLine` primitive is used to create a solid line between two points in 3D space. It is useful for visualizing connections or paths in the scene. The `SolidLine` class allows you to set the start and end points of the line, as well as its width and color and the radius of the line. The following code shows an example of how to create a solid line:

```cpp
auto line = SolidLine::create(glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 1.0, 0.0), 0.1);
line->setP0(glm::vec3(0.0, -1.0, 0.0));
line->setP1(glm::vec3(0.0, 1.0, 0.0));
```

## SolidPolyLine

The `SolidPolyLine` primitive is used to create a 3D object by extruding a 2D shape along a specified path. This is useful for creating complex shapes that are not easily defined by basic primitives. The following code shows an example of how to create an SolidPolyLine:

```cpp
auto extrusion = SolidPolyLine::create(0.1);
extrusion->addPoint(gml::dvec3(0.5, -1.0, -1.0));
extrusion->addPoint(gml::dvec3(0.5, 1.0, 1.0));
extrusion->addPoint(gml::dvec3(0.5, 3.0, -1.0));
extrusion->refresh();
```