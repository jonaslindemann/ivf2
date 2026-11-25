# Cheat Sheet

Everything ivf2 offers, on one page. Each entry links to the guide that covers it. Classes without a guide are documented in the [API Reference](../api/index.md).

All classes are created with `ClassName::create(...)` and returned as `std::shared_ptr`. Core classes live in the `ivf` namespace, windows and UI in `ivfui`, and math helpers in `ivfmath`.

## Windows and application

| Class | What it is | Guide |
|-------|------------|-------|
| `GLFWApplication` | Runs the event loop for one or more windows | [Your First Application](../getting_started/your_first_app.md) |
| `GLFWWindow` | Plain OpenGL window with input events | [Basic Concepts](../getting_started/basic_concepts.md) |
| `GLFWSceneWindow` | Window with a scene, camera, effects, selection and built-in tools | [Scene Window](../ui/scene_window.md) |
| `GLFWMusicSceneWindow` | Scene window driven by a song | [Music-Driven Scenes](../creative/music_scenes.md) |

## Scene graph

| Class | What it is | Guide |
|-------|------------|-------|
| `Node` | Base of everything drawable: material, texture, visibility | [Nodes](../core/nodes.md) |
| `TransformNode` | Node with position, rotation and scale | [Nodes](../core/nodes.md) |
| `CompositeNode` | Groups child nodes | [Nodes](../core/nodes.md) |
| `MeshNode` | Node holding mesh geometry | [Nodes](../core/nodes.md) |
| `InstanceNode` | Draws another node again, sharing its geometry | |
| `InstancedMeshNode` | Draws many copies of a mesh in one call | `instancing1` example |
| `NodeVisitor` | Walks the scene graph | [Visitors](../advanced/visitor_pattern.md) |
| `SceneSerializer` | Saves and loads scenes as JSON | `serialization1` example |
| `GltfModel` | glTF 2.0 model: hierarchy, PBR materials, skinning, morph targets, animations | [Model Loading](../advanced/model_loading.md#gltf-models) |
| `ModelLoader` | Loads glTF and, through Assimp, OBJ, FBX, 3DS, ... | [Model Loading](../advanced/model_loading.md) |
| `ivfvis::Colormap`, `ColorBar` | Map values to colours; show the scale over the scene | [Colormaps](../visualisation/colormaps.md) |
| `ivfvis::UniformGrid`, `PointSet` | Data on grids and at points: named scalar and vector arrays, interpolation | [Visualisation](../visualisation/index.md) |
| `OverlayNode` | Children drawn in window pixels over the scene (HUDs, legends) | [Overlays](../visualisation/colormaps.md#overlays) |
| `pick()`, `RayHit`, `Ray` |Ray picking: the object, point and normal under the mouse | [Selection](../advanced/selection.md#ray-picking) |
| `ResourceWatcher` |Reloads textures, models, shaders and other files when they are saved | [Hot Reload](../advanced/hot_reload.md) |

## Primitives

| Class | Shape | Guide |
|-------|-------|-------|
| `Cube`, `Box`, `RoundedBox` | Cubes and boxes, optionally with rounded edges | [Basic Primitives](../primitives/basic_primitives.md) |
| `Sphere`, `Capsule`, `Dodecahedron` | Round shapes and polyhedra | [Basic Primitives](../primitives/basic_primitives.md) |
| `Cylinder`, `CappedCylinder`, `Cone`, `CappedCone`, `Tube`, `CappedTube` | Round shapes along an axis | [Basic Primitives](../primitives/basic_primitives.md) |
| `Plane`, `Disk` | Flat shapes | [Basic Primitives](../primitives/basic_primitives.md) |
| `Extrusion` | 2D profile swept along a 3D path | [Complex Primitives](../primitives/complex_primitives.md) |
| `SolidLine`, `SolidPolyLine`, `SolidPath` | Tubes between points | [Complex Primitives](../primitives/complex_primitives.md) |
| `Arrow` | 3D arrow | |
| `TextNode` | 3D text | [Text](../primitives/text.md) |
| `Billboard` | Textured quad that always faces the camera | |
| `DynamicMesh` | Geometry from your own vertex arrays, updated at runtime | `custom_mesh1` example |
| `generator` meshes | Around 80 composable shapes, paths and meshes | [Procedural Geometry](../core/procedural_geometry.md) |

## Helpers and lines

| Class | What it is | Guide |
|-------|------------|-------|
| `Axis`, `Grid`, `LineGrid` | Orientation helpers | [Scene Window](../ui/scene_window.md) |
| `Cursor` | 3D cursor with ground projection | [Selection](../advanced/selection.md) |
| `LineTrace`, `TraceNode` | Trail behind a moving point or node | [Spline Animation](../animation/spline_animation.md) |
| `DebugDraw` | Immediate-mode lines and shapes for debugging | `debug_draw1` example |
| `GridLayout` | Arranges nodes in a 3D grid | |

## Materials and textures

| Class | What it is | Guide |
|-------|------------|-------|
| `Material` | Standard lit material: colours and shininess | [Materials and Textures](../core/materials_and_textures.md) |
| `PBRMaterial` | Physically based material: metalness and roughness | |
| `BumpMaterial` | Material with a normal map for surface detail | `bump1` example |
| `ShaderMaterial` | Your own GLSL shaders on a node | [Shader Materials](../core/shader_materials.md) |
| `BlendShaderMaterial` | Shader material with additive or alpha blending | [Shader Materials](../core/shader_materials.md) |
| `GlowMaterial`, `MistMaterial`, `GradientSkyMaterial`, `WaterMaterial` | Ready-made shader materials | [Shader Materials](../core/shader_materials.md) |
| `Texture` | Image texture | [Materials and Textures](../core/materials_and_textures.md) |
| `CheckerboardTexture`, `PerlinNoiseTexture`, `GradientTexture`, `MarbleTexture`, `WoodTexture` | Textures generated on the CPU | `procedural1` example |
| `GPUCheckerboardTexture`, `GPUNoiseTexture`, `GPUGradientTexture` | Textures generated on the GPU | `procedural2` example |
| `Cubemap`, `Skybox` | Six-sided environment images and a sky behind the scene | `skybox1` example |
| `EnvironmentMap` | HDR environment (`.hdr` or procedural sky) for image-based lighting of `PBRMaterial` and as skybox | `ibl1` example |
| `PlanarReflection` | Mirror image of the scene in a plane | [Shader Materials](../core/shader_materials.md#watermaterial) |
| `Palette` | Named colour palettes and colour harmonies | |

## Lighting and shadows

| Class | What it is | Guide |
|-------|------------|-------|
| `LightManager` | Creates and applies lights | [Lighting](../core/lighting.md) |
| `PointLight`, `DirectionalLight`, `SpotLight` | Light types | [Lighting](../core/lighting.md) |
| `ShadowMap` | Depth texture of a shadow-casting light (cascades or cube faces as tiles) | [Shadows](../advanced/shadows.md) |
| `ShadowFilter` | Hard, PCF or PCSS shadow edges (`Light::setShadowFilter()`) | [Shadows](../advanced/shadows.md#soft-shadows) |

## Post-processing effects

Added to the window with `addEffect()`. See the [overview](../effects/index.md).

| Group | Effects |
|-------|---------|
| [Blur, bloom and depth](../effects/blur.md) | `BlurEffect`, `BloomEffect`, `MipBloomEffect`, `DepthOfFieldEffect`, `FogEffect`, `MotionBlurEffect` |
| [Ambient occlusion and reflections](../effects/screen_space.md) | `SsaoEffect`, `ScreenSpaceReflectionEffect` |
| [Colour](../effects/color_effects.md) | `TintEffect`, `ColorGradingEffect`, `PosterizeEffect`, `DitheringEffect`, `HalftoneEffect`, `NightVisionEffect`, `EdgeDetectionEffect`, `FadeEffect` |
| [Film and distortion](../effects/film_effects.md) | `FilmgrainEffect`, `VignetteEffect`, `ChromaticEffect`, `ScanlineEffect`, `GlitchEffect`, `PixelationEffect`, `WaveDistortionEffect`, `SwirlEffect`, `KaleidoscopeEffect`, `FeedbackEffect` |
| [Your own](../effects/custom_effects.md) | Subclass `Effect` |

## Deformers

Added to a `DeformableMeshNode` or `DeformablePrimitive<T>`. See [Deformers](../core/deformers.md).

`TwistDeformer`, `BendDeformer`, `ScaleDeformer`, `WaveDeformer`, `TurbulenceDeformer`, `RandomDeformer`, `FunctionDeformer`, combined in a `DeformerStack` and animated with `DeformerAnimator`.

## Animation and time

| Class | What it is | Guide |
|-------|------------|-------|
| `TimeController` | Global pausable, scalable clock | [Time Control](../core/time_controller.md) |
| `Tween`, `TweenManager`, `Easing` | Animate a value from A to B | [Choosing an Animation Tool](../animation/choosing.md#tweens) |
| `KeyframeAnimation`, `KeyframeInterpolator` | Poses at fixed times | [Keyframe Animation](../animation/keyframe_animation.md) |
| `SplineAnimation`, `Spline` | Movement along a curve | [Spline Animation](../animation/spline_animation.md) |
| `CameraAnimation` | Camera fly-throughs with shake | [Camera Animation](../animation/camera_animation.md) |
| `SceneTimeline`, `SceneTimelinePlayer` | Sequence of scenes | [Choosing an Animation Tool](../animation/choosing.md#scene-timelines) |
| `TimerManager` | One-shot and repeating callbacks | |
| `SpinBehavior`, `PulseBehavior`, `FollowSplineBehavior`, `HoverHighlightBehavior`, `LambdaBehavior` | Reusable behaviours attached to nodes | `behaviors1` example |

## Creative coding and sound

| Class | What it is | Guide |
|-------|------------|-------|
| `FlowField` | Curl-noise vector field | [Flow Fields](../creative/flow_field.md) |
| `ParticleSystem` | CPU-simulated, GPU-drawn particles | `particles1` example |
| `AudioManager` | Music playback and audio analysis | `audio_reactive1` example |
| `MidiController` | MIDI input | [Music-Driven Scenes](../creative/music_scenes.md) |
| `ivfmusic::Automation`, `SongTransport`, `Score`, `LiveControls` | Song-driven parameters | [Music-Driven Scenes](../creative/music_scenes.md) |

## User interface

| Class | What it is | Guide |
|-------|------------|-------|
| `UiWindow` | Base class for your own ImGui panels | [UI Windows](../ui/ui_windows.md), [Custom UI](../ui/custom_ui.md) |
| `ObjectInspector`, `SceneInspector`, `EffectInspector` | Property editors | [Built-in Scene Tools](../ui/scene_ui_integration.md) |
| `FpsWindow`, light windows, `UiInputDialog` | Ready-made panels | [Ready-made Widgets](../ui/widgets.md) |
| `TimeControlPanel` | Play, pause and slow motion | [Time Control Panel](../ui/time_control_panel.md) |
| `UiMainMenu`, `UiMenu`, `UiMenuItem` | The menu bar | [Built-in Scene Tools](../ui/scene_ui_integration.md#the-main-menu) |
| `CameraManipulator`, `FlyCameraManipulator`, `PathCameraManipulator` | Camera control with the mouse or along a path | |
| `FrameWriter` | Saves rendered frames as images | `frame_export1` example |

## Managers

Global singletons, reached with `ClassName::instance()`:

| Manager | Responsible for |
|---------|-----------------|
| `ShaderManager` | Shader programs |
| `LightManager` | Lights and shadows |
| `TextureManager` | Texture binding |
| `FontManager` | Fonts for `TextNode` |
| `TweenManager` | Running tweens |
| `TimeController` | Scene time |
| `TimerManager` | Timers |
