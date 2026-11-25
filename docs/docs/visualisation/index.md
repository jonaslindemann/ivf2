# Scientific Visualisation

`ivfvis` is ivf2's module for visualising data: values on grids and at scattered points, coloured through colormaps and explained by colour bars. It builds on the scene graph, so visualisations combine freely with everything else in ivf2: lights, shadows, effects, picking and animation.

`ivfvis` is a separate library. Link it next to ivf2:

```cmake
target_link_libraries(my_app PRIVATE ivf2::ivf2 ivf2::ivfvis)
```

```cpp
#include <ivfvis/ivfvis.h>
```

It is built by default; `-DIVF2_BUILD_VIS=OFF` leaves it out. It needs no libraries beyond ivf itself.

!!! note "Work in progress"
    `ivfvis` grows in steps (see `ROADMAP.md`, phase 3.1). Available now: data sets, colormaps, vertex colouring, colour bars, point clouds, isosurfaces, slice planes, glyphs, streamlines, volume rendering and 3D axes. Next come file readers and 2D plots.

## Data sets

A data set is a set of points with named arrays of values: **scalars** (one `float` per point) and **vectors** (one `glm::vec3` per point).

| Class | Points |
|-------|--------|
| `UniformGrid` | A regular 3D grid: dimensions (points per axis), origin and spacing. A dimension of 1 makes a 2D grid. |
| `PointSet` | Points at arbitrary positions (measurements, particles, point clouds). |

```cpp
// 64 x 64 x 64 points covering [-1, 1]^3
auto grid = ivfvis::UniformGrid::create({64, 64, 64}, ivf::BoundingBox({-1, -1, -1}, {1, 1, 1}));

// Fill arrays from functions of the position ...
grid->fillScalars("density", [](const glm::vec3 &p) { return std::exp(-4.0f * glm::dot(p, p)); });
grid->fillVectors("velocity", [](const glm::vec3 &p) { return glm::vec3(-p.y, p.x, 0.0f); });

// ... or write the values directly (point i + nx * (j + ny * k))
auto temperature = grid->addScalars("temperature");
temperature[grid->index(3, 4, 5)] = 293.0f;

// Derived arrays and value ranges
grid->addMagnitude("velocity", "speed");
ivfvis::ValueRange range = grid->range("speed");   // range.min, range.max
```

`UniformGrid::sample()` and `sampleVector()` interpolate an array at any position (trilinearly; positions outside the grid are clamped to it), and `gradient()` gives the gradient of a scalar array. Point arrays are stored with x varying fastest, the layout of VTK image data and of C-ordered NumPy arrays indexed `[k][j][i]`.

Looking up an array that does not exist is treated as a programming error and throws `std::out_of_range`; check optional arrays with `hasScalars()` and `hasVectors()`.

## Vector fields

`VectorField` is the interface for vector data that can be sampled anywhere. [Glyphs and streamlines](vector_fields.md) use it.Three adapters cover the common sources:

```cpp
auto fromGrid = ivfvis::VectorField::fromGrid(grid, "velocity");          // interpolated grid data
auto analytic = ivfvis::VectorField::fromFunction(
    [](const glm::vec3 &p) { return glm::vec3(-p.y, p.x, 0.1f); },
    ivf::BoundingBox({-2, -2, -2}, {2, 2, 2}));                           // a function, optionally bounded
auto curl = ivfvis::VectorField::fromFlowField(ivf::FlowField::create()); // ivf's curl-noise flow field

glm::vec3 v = fromGrid->sample({0.2f, 0.1f, 0.0f});
bool inside = fromGrid->contains({3.0f, 0.0f, 0.0f});                      // false: outside the grid
```

## Next

- [Colormaps and Colour Bars](colormaps.md): turn values into colours, and show the scale.
- [Point Clouds](point_clouds.md): draw millions of points coloured and sized by their values.
- [Isosurfaces and Slices](scalar_fields.md): surfaces of constant value and cuts through 3D fields.
- [Glyphs and Streamlines](vector_fields.md): arrows and field lines of vector fields.
- [Volume Rendering](volumes.md): whole 3D fields as semi-transparent clouds.
- [Axes and Labels](axes.md): 3D axes with ticks, labels and grid walls that follow the camera.
