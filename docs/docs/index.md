# Interactive Visualisation Framework 2

![ivf2](images/logo.png)

ivf2 is a modern C++23 library for real-time 3D graphics, visualisation and creative coding, built on OpenGL 3.3+. You build a scene from nodes, such as primitives, models, text and lights, give them materials and effects, and animate them over time. A thin application layer handles windows, input and ImGui panels, so a working interactive scene takes only a few dozen lines.

ivf2 is a complete rewrite of [ivfplusplus](https://github.com/jonaslindemann/ivfplusplus), the C++ OpenGL wrapper behind [ObjectiveFrame](https://jonaslindemann.github.io/objectiveframe/). It aims to be easy to start with and easy to extend, for teaching, research visualisation and generative art alike.

## Highlights

Physically based materials lit by HDR environments, soft shadows from the sun and from lamps, ambient occlusion and reflections, and an HDR image with filmic tone mapping:

<div class="ivf-gallery ivf-gallery-wide" markdown>

- [![Image-based lighting](images/gallery/ibl1.jpg) Image-based lighting](core/pbr_and_ibl.md)
- [![Screen-space reflections](images/gallery/ssr1.jpg) Reflections and SSAO](effects/screen_space.md)
- [![Cascaded shadows](images/gallery/cascaded_shadows1.jpg) Cascaded soft shadows](advanced/shadows.md#cascaded-shadow-maps)
- [![Point and spot light shadows](images/gallery/point_shadows1.jpg) Point and spot light shadows](advanced/shadows.md#point-lights)
- [![HDR and tone mapping](images/gallery/hdr1.jpg) HDR and tone mapping](effects/hdr.md)
- [![Ambient occlusion](images/gallery/ssao1.jpg) Ambient occlusion](effects/screen_space.md#ambient-occlusion)
- [![Bloom, glow and mist](images/gallery/mip_bloom1.jpg) Bloom, glow and mist](effects/blur.md#mipbloomeffect)
- [![Water reflections](images/gallery/reflection1.jpg) Water reflections](core/shader_materials.md#watermaterial)

</div>

All of these are example programs you can run and change. The [Example Gallery](examples/gallery.md) shows every example; the topics below lead to the guides.

<div class="ivf-gallery" markdown>

- [![Primitives](images/sc_primitives1.png) Primitives](examples/primitives_examples.md)
- [![Lighting](images/sc_lighting1.png) Lighting](examples/lighting_examples.md)
- [![Textures](images/sc_textures1.png) Textures](examples/texture_examples.md)
- [![Shadows and effects](images/sc_shadows1.png) Shadows](examples/effects_examples.md)
- [![Post-processing](images/sc_effects1.png) Effects](examples/effects_examples.md)
- [![Deformers](images/sc_deformer2.png) Deformers](examples/deformer_examples.md)
- [![Animation](images/sc_animation3.png) Animation](examples/animation_examples.md)
- [![Model loading](images/sc_assimp1.png) Models](examples/asset_examples.md)

</div>

## Features

### Rendering

- [Physically based materials](core/pbr_and_ibl.md) with image-based lighting: an HDR environment (`.hdr` file or a procedural sky) lights the scene with diffuse light and reflections that blur with roughness, and shows as the background
- [HDR rendering](effects/hdr.md): exposure, tone mapping (Reinhard, ACES, AgX), gamma, FXAA and multisampling, for lights far brighter than white
- [Shadows](advanced/shadows.md) from directional, point and spot lights: cascaded shadow maps for large scenes, shadows in all directions around lamps, and soft edges (PCF, or PCSS penumbras that widen with distance)
- [Ambient occlusion and screen-space reflections](effects/screen_space.md) as post-processing effects
- A [render queue](advanced/render_queue.md) that walks the scene once per frame, culls against each view, sorts to save state changes, merges copies of a mesh into instanced draw calls, and counts what it draws
- Several windows, or [ivf inside your own application](advanced/render_context.md) (Qt, SDL, ...), each with its own OpenGL context

### Scene and geometry

- A [scene graph](core/nodes.md) of nodes, transforms and groups, with `create()` factories and `shared_ptr` throughout
- [Basic primitives](primitives/basic_primitives.md) such as cubes, spheres, cylinders and cones, and [complex ones](primitives/complex_primitives.md) such as extrusions, rounded boxes, capsules and polyhedra
- [Procedural geometry](core/procedural_geometry.md) from the bundled `generator` library, with around 80 composable shapes, paths and meshes
- [3D text](primitives/text.md), and [model loading](advanced/model_loading.md): glTF 2.0 with PBR materials, skinning, morph targets and animations, and other formats through Assimp
- [Lighting](core/lighting.md) with point, directional and spot lights

### Materials and effects

- [Materials and textures](core/materials_and_textures.md), including physically based and normal-mapped materials, plus custom GLSL with [`ShaderMaterial`](core/shader_materials.md)
- Additive and alpha blending with `BlendShaderMaterial`, and [ready-made glow, mist, gradient sky and water materials](core/shader_materials.md#ready-made-materials)
- [Over 25 post-processing effects](effects/index.md): bloom, depth of field, fog, ambient occlusion, reflections, colour grading,film grain, chromatic aberration, halftone, kaleidoscope, glitch, video feedback and more, or [your own](effects/custom_effects.md)
- [Mesh deformers](core/deformers.md): twist, bend, scale, wave, turbulence, noise and custom functions
- [Hot reload](advanced/hot_reload.md): textures, models and shaders update while the program runs when their files are saved

### Animation and time

- [Keyframe](animation/keyframe_animation.md) and [spline](animation/spline_animation.md) animation, and procedural motion such as [Lissajous curves](animation/lissajous.md)
- [Camera fly-throughs](animation/camera_animation.md) along splines, with look-at targets and shake
- [Tweens](animation/choosing.md#tweens) with shared easing curves, and a [scene timeline](animation/choosing.md#scene-timelines) for sequencing
- [Global time control](core/time_controller.md): pause, slow motion and time scaling for the whole scene

### Creative coding

- [Flow fields](creative/flow_field.md): curl-noise particle motion that looks organic and fluid
- [Music-driven scenes](creative/music_scenes.md): the song is the clock. Parameters are keyed to bars in a JSON file that reloads while the music plays, notes from Renoise songs drive values, and MIDI controllers can be played live and recorded. Frame export renders the same picture as playback.

### Scientific visualisation

- [Data sets](visualisation/index.md) on regular grids and at scattered points, with named scalar and vector arrays, interpolation and gradients
- [Colormaps](visualisation/colormaps.md) (viridis, magma, cividis, turbo, coolwarm and more, or your own) with linear or log scales and discrete steps, mesh colouring by values, and colour bars over the scene
- [Point clouds](visualisation/point_clouds.md) of millions of points, coloured and sized by their values and picked point by point
- [Isosurfaces and slice planes](visualisation/scalar_fields.md) through 3D fields, with contour lines
- [Glyphs and streamlines](visualisation/vector_fields.md) for vector fields: arrows, field lines and stream tubes
- [Volume rendering](visualisation/volumes.md) with colour and opacity transfer functions, embedded among opaque objects

### User interface

- [GLFW windows](ui/scene_window.md) with built-in camera control and input events
- [ImGui panels](ui/ui_windows.md) that you compose into your own tools, [ready-made widgets](ui/widgets.md), and [built-in inspectors](ui/scene_ui_integration.md) for the scene, effects and camera
- Mouse [selection](advanced/selection.md) of objects, by hover, click or rectangle, and [ray picking](advanced/selection.md#ray-picking) with exact surface points and normals

## Getting started

1. [Install](getting_started/installation.md) the dependencies and build the library and examples.
2. Read [Basic Concepts](getting_started/basic_concepts.md) for how scenes, nodes and windows fit together.
3. Build [your first application](getting_started/your_first_app.md).

Then browse the [Example Gallery](examples/gallery.md) or the [Example Index](examples/index.md). Every example is a small, standalone program you can run and change.The [Cheat Sheet](reference/cheat_sheet.md) lists every class on one page. For generative and music-driven work, start with [Creative Programming](creative/index.md).

## API reference

Every class is documented in the [API Reference](api/index.md), which is generated from the source with Doxygen.
