# Example Index

ivf2 comes with around 50 small example programs. Each is a standalone executable that shows one feature, and they are the quickest way to learn the library: run one, read its source, change something, run it again.

## Running the examples

The examples are built together with the library. Executables end up in `build/<preset>/bin/<Config>/`; Debug builds have a `d` suffix, for example `hello_ivfd`.

```
build/release/bin/Release/hello_ivf
```

Run them from that folder, or anywhere else as long as the `shaders/`, `fonts/` and `assets/` folders next to the executable are present. CMake copies them there during the build.

The sources are in `examples/<name>/`.

## Getting started

| Example | Shows | Guide |
|---------|-------|-------|
| `hello_ivf` | A scene built by hand in a plain `GLFWWindow`: shaders, lights, a scene graph and drawing it yourself | [Basic Concepts](../getting_started/basic_concepts.md) |
| `hello_pipeline` | The minimal rendering pipeline, step by step | [Basic Concepts](../getting_started/basic_concepts.md) |

Most other examples use `GLFWSceneWindow`, which does the setup in `hello_ivf` for you. See [Your First Application](../getting_started/your_first_app.md).

## Geometry

| Example | Shows | Guide |
|---------|-------|-------|
| `primitives1` | All the basic primitives | [Primitive Examples](primitives_examples.md) |
| `extrusions1` | Profiles swept along paths | [Primitive Examples](primitives_examples.md) |
| `transforms1` | A grid of arrows with random orientations | [Primitive Examples](primitives_examples.md) |
| `text1` | 3D text with live editing | [Text](../primitives/text.md) |
| `custom_mesh1` | `DynamicMesh` for generated geometry: a Lissajous curve and a Lorenz attractor | |
| `instancing1` | Thousands of copies of a mesh with `InstancedMeshNode` | |
| `bbox1` | Bounding boxes of nested nodes | [Nodes](../core/nodes.md) |
| `deformer1` | Twist and bend deformers with inspectors | [Deformers](../core/deformers.md) |
| `deformer2` | Layered wave deformers for a water surface | [Deformers](../core/deformers.md) |
| `deformer3` | Random and noise deformers | [Deformers](../core/deformers.md) |

## Materials, textures and lighting

| Example | Shows | Guide |
|---------|-------|-------|
| `textures1` | Image textures and texture modes | [Texture Examples](texture_examples.md) |
| `textures2` | Multitexturing with blend modes | [Materials and Textures](../core/materials_and_textures.md) |
| `textures3` | Textures generated on the GPU | |
| `textures4` | `PBRMaterial`: a grid of spheres from rough to smooth and dielectric to metal | |
| `procedural1` | Textures generated on the CPU: noise, marble, wood | |
| `procedural2` | Animated GPU procedural textures | |
| `bump1` | Normal maps with `BumpMaterial` | |
| `shader_material1` | Your own GLSL shaders on nodes, reloaded on save | [Shader Materials](../core/shader_materials.md) |
| `reflection1` | Reflecting water with `PlanarReflection` and `WaterMaterial` | [Shader Materials](../core/shader_materials.md#watermaterial) |
| `skybox1` | A cubemap sky behind the scene | |
| `lighting1` | Point, directional and spot lights with editing windows | [Lighting](../core/lighting.md) |
| `shadows1` | Shadow mapping from a directional light | [Shadows](../advanced/shadows.md) |

## Effects

| Example | Shows | Guide |
|---------|-------|-------|
| `effects1` | Every stylisation effect, switchable from a panel | [Post-Processing](../effects/index.md) |
| `fog1` | Distance and height fog | [Blur, Bloom and Depth](../effects/blur.md#fog) |
| `dof1` | Depth of field | [Blur, Bloom and Depth](../effects/blur.md#depth-of-field) |
| `mip_bloom1` | Wide bloom with glow, mist and sky materials and a guided camera | [Blur, Bloom and Depth](../effects/blur.md#mipbloomeffect) |

## Animation and time

| Example | Shows | Guide |
|---------|-------|-------|
| `animation1` | Animating in `onUpdate()` | [Basic Animation](../animation/basic_animation.md) |
| `animation2` | A Lissajous curve with parameter sliders | [Lissajous Curves](../animation/lissajous.md) |
| `animation3` | Spline and keyframe animation with traces | [Keyframe Animation](../animation/keyframe_animation.md) |
| `tweening1` | Tweens with every easing curve | [Choosing an Animation Tool](../animation/choosing.md#tweens) |
| `timeline1` | A sequence of three scripted scenes | [Choosing an Animation Tool](../animation/choosing.md#scene-timelines) |
| `camera_anim1` | Camera fly-through with shake | [Camera Animation](../animation/camera_animation.md) |
| `time_control1` | Pause, slow motion and time scaling for a whole scene | [Time Control](../core/time_controller.md) |
| `behaviors1` | Reusable behaviours attached to nodes: spin, pulse, follow a spline | |

## Creative coding and sound

| Example | Shows | Guide |
|---------|-------|-------|
| `flow_field1` | 8 000 particles steered by curl noise | [Flow Fields](../creative/flow_field.md) |
| `particles1` | Interactive fire and sparks with `ParticleSystem` | |
| `audio_reactive1` | Visuals reacting to music or the microphone | |
| `music_scene1` | A scene driven by a song, with automation and a camera path | [Music-Driven Scenes](../creative/music_scenes.md) |
| `frame_export1` | Exporting a PNG sequence at an exact frame rate for video | |
| `solar_system` | A small solar system simulation with its own control panel | |

## Interaction and UI

| Example | Shows | Guide |
|---------|-------|-------|
| `selection1` | Highlighting objects under the mouse | [Selection](../advanced/selection.md) |
| `cursor1`, `cursor_optimization` | The 3D cursor and its ground projection | [Selection](../advanced/selection.md#the-3d-cursor-position) |
| `editor1` | Starting point for an editor: floor grid and the 3D mouse position | [Selection](../advanced/selection.md#the-3d-cursor-position) |
| `ui1`, `ui2`, `scene_ui1` | ImGui windows combined with a scene | [UI Windows](../ui/ui_windows.md) |
| `assimp1` | Opening any model file through a file dialog | [Model Loading](../advanced/model_loading.md) |
| `serialization1` | Saving, loading and clearing a scene as JSON | |
| `debug_draw1` | Immediate-mode debug lines and shapes | |
| `rigid_body1` | Rigid-body physics with ReactPhysics3D | |
