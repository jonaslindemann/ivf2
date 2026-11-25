# ivf2 API Reference {#mainpage}

This is the class and function reference of **ivf2**, generated from the source code. For
introductions, concepts and examples, see the [user guide](../../index.html).

## Libraries

| Library | Namespace | Contents |
|---------|-----------|----------|
| `ivf2::ivf` | `ivf` | Scene graph, nodes and primitives, materials and shaders, lights and shadows, render queue, effects, animation, model loading |
| `ivf2::ivfui` | `ivfui` | GLFW windows and input, camera manipulators, ImGui (and ImPlot) user interfaces |
| `ivf2::ivfvis` | `ivfvis` | Scientific visualisation: data sets, colormaps, point clouds, isosurfaces, slices, glyphs, streamlines, volumes, axes, file readers |
| `ivf2::ivfmath` | `ivfmath` | Keyframe and spline interpolation, spherical math |
| `ivf2::ivfmusic` | `ivfmusic` | Scenes driven by a song: transport, automation, scores, live controls |

## Where to start

- `ivfui::GLFWSceneWindow`: the window most applications derive from
- `ivf::Node`, `ivf::TransformNode`, `ivf::CompositeNode`: the scene graph
- `ivf::Material`, `ivf::PBRMaterial`, `ivf::ShaderMaterial`: how things look
- `ivf::RenderContext`: OpenGL state and resources of a window
- `ivfvis::UniformGrid`, `ivfvis::Colormap`: data and colours for visualisation

The [user guide](../../index.html) explains each of these with examples; the
[example gallery](../../examples/gallery/index.html) shows what the example programs draw.
