# Procedural Geometry

ivf2 includes `generator`, a geometry library with around 80 building blocks for creating geometry in code. They are shapes, paths and meshes that you combine like LEGO: sweep a circle along a knot to get a torus knot, repeat a box in a row, subdivide and spherify a cube into a ball. Most of ivf2's own primitives, such as `Box`, `Sphere` and `Capsule`, are built on it.

Use it when the built-in primitives aren't enough and you want a shape described by a few parameters rather than modelled by hand.

## The three kinds of generator

| Kind | What it is | Examples |
|------|------------|----------|
| **Shape** | A 2D outline | `CircleShape`, `RectangleShape`, `RoundedRectangleShape`, `GridShape`, `BezierShape`, `ParametricShape` |
| **Path** | A 3D curve | `LinePath`, `HelixPath`, `KnotPath`, `PolyLinePath`, `ParametricPath` |
| **Mesh** | A 3D surface | `SphereMesh`, `TorusMesh`, `IcoSphereMesh`, `SpringMesh`, `TorusKnotMesh`, `TeapotMesh`, `ParametricMesh`, ... |

Shapes and paths are combined into meshes, and meshes are transformed and combined into new meshes.

## From generator to scene node

A generated mesh is turned into an `ivf::MeshNode` with `createFromGenerator()`. This small helper works for any generator mesh:

```cpp
#include <generator/generator.hpp>
#include <ivf/mesh_node.h>

template <typename GeneratorMesh>
ivf::MeshNodePtr toMeshNode(const GeneratorMesh &mesh)
{
    generator::AnyGenerator<generator::MeshVertex> vertices = mesh.vertices();
    generator::AnyGenerator<generator::Triangle> triangles = mesh.triangles();

    auto node = ivf::MeshNode::create();
    node->createFromGenerator(vertices, triangles);
    return node;
}
```

```cpp
auto knot = toMeshNode(generator::TorusKnotMesh(2, 3, 16, 256));
knot->setMaterial(material);
add(knot);
```

!!! note "Up axis"
    `generator` uses Z as the up axis, while ivf2 uses Y. `createFromGenerator()` swaps the two, so a shape that points up along Z in `generator` points up along Y in your scene.

## Ready-made meshes

A selection of the available meshes and their main parameters:

| Mesh | Parameters |
|------|------------|
| `SphereMesh(radius, slices, segments)` | UV sphere |
| `IcoSphereMesh(radius, segments)` | Evenly tessellated sphere, good for deforming |
| `TorusMesh(minor, major, slices, segments)` | Ring; `minor` is the tube radius, `major` the ring radius |
| `TorusKnotMesh(p, q, slices, segments)` | Tube winding `p` times around one axis and `q` times around the other |
| `SpringMesh(minor, major, size, slices, segments)` | Coil spring |
| `BoxMesh`, `RoundedBoxMesh`, `CapsuleMesh`, `CylinderMesh`, `ConeMesh`, `TubeMesh`, `DiskMesh`, `PlaneMesh` | The basics, also available as ivf2 primitives |
| `DodecahedronMesh`, `IcosahedronMesh`, `SphericalConeMesh`, `SphericalTriangleMesh`, `TeapotMesh` | Polyhedra and classics |

Every header in `include/generator/` documents its parameters.

## Building meshes from shapes and paths

### Extrusion: a shape swept along a path

`ExtrudeMesh` sweeps a 2D shape along a 3D path. A small circle along a helix gives a coil; a rounded rectangle along a knot gives a ribbon:

```cpp
using namespace generator;

auto coil = ExtrudeMesh<CircleShape, HelixPath>(
    CircleShape(0.1, 16),                                     // tube cross-section
    HelixPath(1.0, 1.5, 128, 0.0, gml::radians(360.0 * 4)));   // radius, half height, segments, start, sweep

add(toMeshNode(coil));
```

### Lathe: a shape spun around an axis

`LatheMesh` revolves a 2D shape around an axis, like a potter's wheel. It suits vases, bottles and anything round:

```cpp
// A circle moved 1 unit off the axis, spun into a ring
auto ring = LatheMesh<TranslateShape<CircleShape>>(
    translateShape(CircleShape(0.2, 24), {1.0, 0.0}),   // shape
    {0.0, 1.0},                                         // axis
    48);                                                // slices
```

Keep the shape on one side of the axis; a shape that crosses it produces a surface that folds through itself.

### Parametric surfaces

For any surface you can describe with a formula, `ParametricMesh` calls your function over a grid of (u, v) values from 0 to 1. Return a vertex with position, normal and texture coordinates:

```cpp
auto wave = ParametricMesh(
    [](const gml::dvec2 &t) {
        MeshVertex v;
        double x = (t[0] - 0.5) * 10.0, y = (t[1] - 0.5) * 10.0;
        v.position = {x, y, 0.3 * std::sin(x) * std::cos(y)};   // z is up in generator
        v.normal = {0.0, 0.0, 1.0};
        v.texCoord = t;
        return v;
    },
    {64, 64});                                                  // grid resolution
```

The normal here is a shortcut; for correct lighting on a strongly curved surface, compute it from the partial derivatives of the position.

## Modifying and combining meshes

Modifiers take a mesh and return a new one. Each has a helper function, so they nest naturally:

| Modifier | Effect |
|----------|--------|
| `translateMesh(mesh, offset)` | Move |
| `rotateMesh(mesh, angle, axis)` | Rotate (angle in radians) |
| `scaleMesh(mesh, factors)` | Scale per axis |
| `MirrorMesh<M>(mesh, Axis::X)` | Adds a copy mirrored along an axis, for symmetrical shapes |
| `flipMesh(mesh)` | Turns the mesh inside out (reverses triangles and normals) |
| `axisSwapMesh(mesh, x, y, z)` | Reassigns the axes, for example to turn Z-up content Y-up |
| `repeatMesh(mesh, count, offset)` | Several copies, each offset further |
| `mergeMesh(a, b, ...)` | Combine meshes into one |
| `SubdivideMesh<Mesh, N>(mesh)` | Split every triangle into four, `N` times |
| `spherifyMesh(mesh, radius, factor)` | Push vertices towards a sphere; `factor` 0–1 |

For example, a row of five rotated cubes merged with a base plate:

```cpp
using namespace generator;

auto row = repeatMesh(rotateMesh(BoxMesh(), gml::radians(45.0), Axis::Z), 5, {3.0, 0.0, 0.0});
auto scene = mergeMesh(row, translateMesh(PlaneMesh({10.0, 2.0}), {6.0, 0.0, -1.0}));
add(toMeshNode(scene));
```

Generators are lazy: nothing is computed until `createFromGenerator()` walks through the vertices, so building long chains of modifiers is cheap.

## When to use what

- **Fixed, parameterised shape:** a generator mesh, converted once in `onSetup()`.
- **Shape that changes every frame:** generate once, then animate it with [deformers](deformers.md); regenerating every frame is much slower.
- **Profile swept along a path with control over caps, twist and scale along the way:** ivf2's `Extrusion` primitive. See [Complex Primitives](../primitives/complex_primitives.md).
- **Many copies of the same shape:** instancing (see `instancing1`) instead of `repeatMesh`, so each copy can move independently.

## Related

- [Basic Primitives](../primitives/basic_primitives.md) and [Complex Primitives](../primitives/complex_primitives.md)
- [Deformers](deformers.md)
