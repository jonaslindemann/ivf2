# Changelog

All notable changes to ivf2 are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and ivf2 uses
[semantic versioning](https://semver.org/) (see [CONTRIBUTING.md](CONTRIBUTING.md#versioning-and-releases)).
While the version is `2.0.0-alpha.*`, the API may still change between pre-releases;
renamed functions keep a `[[deprecated]]` alias for at least one release.

## [Unreleased]

### Added

- `ivfphysics` (work in progress, see `ROADMAP.md` phase 3.7): rigid body physics for scene nodes,
  a separate library `ivf2::ivfphysics` (CMake option `IVF2_BUILD_PHYSICS`, vcpkg feature
  `physics`, package component `Physics`). ReactPhysics3D is private to it: applications need no
  physics headers. `PhysicsWorld` steps with a fixed time step (default 1/240 s) and interpolates
  the nodes to the frame time, deterministically; pause, single steps, time scale and `reset()`.
  `addBody(node, shape, {mass, friction, bounciness, type, ...})` attaches a static, kinematic
  (follows its node) or dynamic (drives its node) `RigidBody`, with forces, impulses and
  velocities. Shapes: box, sphere, capsule, plane, convex hull (built by ivfphysics, robust for
  coplanar mesh points; large meshes are reduced to their extreme points), triangle mesh, and
  `Shape::fromNode()` from primitives and meshes. Unit tests run without OpenGL.
  Queries and events: `raycast()`/`raycastAll()` (body, point, normal, distance; category mask,
  triggers, skip), contact callbacks (Begin/End, optional Stay, contact points and impact speed)
  and trigger bodies with enter/leave callbacks, delivered after each step so they may add and
  remove bodies; collision categories and masks (`BodyDesc::category`/`collidesWith`,
  `setCollisionFilter()`); step callbacks for forces applied every fixed step; `stats()`.
  Interaction: `DragController` (drag a body with camera rays through a mass-independent damped
  spring) and `throwBody()`. `drawDebug()` draws colliders, contact points, velocities and the drag
  spring through `DebugDraw`. `rigid_body1` drags bodies with the mouse, throws balls (T) and shows
  the debug view (V) and statistics.
- Web builds (work in progress, see `ROADMAP.md` phase 3.3): the CMake preset `wasm` compiles
  the libraries and examples to WebAssembly with Emscripten; programs render with WebGL 2
  (OpenGL ES 3.0) in a full-window canvas. `ivf2_web_executable()` (in `ivf2Helpers.cmake`) builds an application as a web page with
  its data files preloaded. All examples except `multi_window1`, `embedding1`, `audio_reactive1`,
  `music_scene1` and `assimp1` build for the browser, ImGui/ImPlot panels and `ivfvis` included;
  `shader_check` (`tests/web`) compiles every built-in shader with WebGL 2. The canvas follows
  the size of its parent element and uses device pixels on high-DPI screens; `IVF2_*` query
  parameters in the page URL become environment variables (`page.html?IVF2_FIXED_TIMESTEP=0.016`).
  The installed package works in Emscripten projects (`find_package(ivf2)`; `ivf2Emscripten.cmake`).
  Guide page "Running in the Browser".
- The examples run live in the browser: CI builds them (`.github/workflows/web.yml`, for pull
  requests and releases), tests every page in headless Chrome (`tests/web/smoke_test.mjs`) and
  publishes them with the documentation at <https://jonaslindemann.github.io/ivf2/web/>, with a
  start page from `docs/tools/make_gallery.py --web`. Guide pages link to their live examples,
  and every tile of the Example Gallery has a "Run" badge; README section on web builds.
- `RenderContext::isGles()`; `ivf/glsl_es.h`: shaders are translated to GLSL ES 3.00 when
  compiled on OpenGL ES (version line, default precisions; uniform initialisers become values
  set after linking, `Shader::uniformDefaults()`). One shader source serves both.
- `GLFWApplication::frame()`: one frame of every window, used as the browser's main loop.
- `RenderContext::pixelRatio()`: framebuffer pixels per window unit (above 1 on high-DPI displays),
  set by `GLFWWindow` each frame; `OverlayNode::pixelRatio()`.

### Changed

- `rigid_body1` is rewritten on `ivfphysics` (a pile of boxes, spheres, capsules and cylinders;
  drop, pause, step, reset). The vcpkg feature `examples` is replaced by `physics`.
- `OverlayNode` lays out in window units instead of framebuffer pixels (`onLayout()`,
  `viewportWidth()`, `project()`), so overlays keep their size on high-DPI displays (macOS Retina,
  browsers); `ColorBar` and `Axes3D` render their labels at `pixelRatio()` times the resolution.
  `PointCloud` sizes in pixels are window units too. Unchanged on ordinary displays.
- Multisampled offscreen rendering uses a multisampled renderbuffer instead of a multisampled
  texture (same result; available in OpenGL ES 3 too).
- The colour picking buffer (`BufferSelection`) is RGBA8 with a 24-bit depth buffer.
- Render targets, the output step and the post-processing chain no longer leave their textures
  bound to texture units after use.

### Fixed

- Glyph textures and one- or two-channel images used the unsized internal formats `GL_RED`/`GL_RG`,
  which OpenGL ES rejects; they are `GL_R8`/`GL_RG8` now (what desktop drivers chose anyway).
- `RenderContext::wideLinesSupported()` queried `GL_CONTEXT_FLAGS`, which OpenGL ES does not have.
- `generator::SubdivideShape` did not compile with newer Clang (Emscripten).
- Multitexturing in the stock and bump shaders indexed the sampler array with a variable,
  which GLSL 3.30 and GLSL ES do not allow.
- Blur, pixelation and halftone effects used `textureSize()` (an integer vector) as a float
  vector; `default.frag` and `standard_lighting.fs` passed an integer exponent to `pow()`; the
  phong sample shaders in `data/shaders` used interface blocks. All are now valid GLSL ES too.
- Shader compile time: the shadow filter code (`shadow_glsl`) was inlined once per shadow map
  at every call, and sampled the maps with implicit derivatives inside loops. Now only the
  fetch branches on the map (`ivfShadowFetch()`, `textureLod()`) and multitexturing uses
  `textureGrad()`. With ANGLE's Direct3D backend (WebGL on Windows) the stock shader links in
  about 7 s instead of 25 s; desktop drivers and SwiftShader compile faster too. Same images.

## [2.0.0-alpha.3] - 2026-11-26

The rendering pipeline and scientific visualisation release: scenes are drawn through a render
queue (culling, sorting, transparency, automatic instancing) with camera and light uniform blocks,
an HDR pipeline (tone mapping, FXAA, MSAA), image-based lighting, cascaded and soft shadows for
directional, point and spot lights, SSAO and screen-space reflections, glTF 2.0 models with
skinning, morph targets and animations, hot reload of resources and CPU picking. The new
`ivf2::ivfvis` library adds data sets, colormaps, colour bars, point clouds, isosurfaces, slices,
glyphs, streamlines, volume rendering, 3D axes, readers for CSV, NumPy and VTK files, and 2D
plots with ImPlot.

### Changed (may require changes in your code)

- `PBRMaterial`: an albedo map is now multiplied by `setAlbedo()` (it replaced it before; the
  default albedo is white, so materials that set only the map look the same). The shader writes
  alpha 1 unless `setAlphaMode(AlphaMode::Blend)`.
- Shadows: the `IvfLights` block has `ShadowData shadows[4]` and a `shadowIndex` per directional
  and point light instead of `lightSpaceMatrices`; shadow maps are sampled as `shadowMaps[i]`
  on texture units 8-11 (`ivf::ShadowMapUnit`) instead of 1-4. Shaders that sampled
  `shadowMaps[i]` with `lightSpaceMatrices[i]` use `ivf::shadow_glsl` and `ivfShadowFactor()`.
  Only opaque nodes cast shadows (transparent ones did before). A light's `ShadowMap` is created
  by the first shadow pass instead of `setCastShadows(true)`.
- Meshes are uploaded to the GPU when first drawn instead of in `end()`: creating and changing
  geometry (primitives, `MeshNode`, `Mesh::updateVertices()`/`updateNormals()`) needs no OpenGL
  context, so scenes can be built before the window exists. `Mesh::vertexArray()` uploads on
  demand and returns the vertex array of the current context.
- Mesh vertex attributes use the fixed locations bound by `ShaderManager` (`aPos` 0, `aColor` 1,
  `aTex` 2, `aNormal` 3) instead of querying the program that was current at creation.
- Selection (`setSelectionEnabled(true)`) is rendered only after mouse movement, button presses,
  resizing or camera changes instead of every frame, so `onOverNode()` is no longer called every
  frame while the cursor rests on a node. Clicks pick the node under the cursor afresh.
- `GLFWSceneWindow` traverses the scene once per frame; the shadow, reflection, main and selection
  passes draw the same items (`LightManager::renderShadowMaps(const RenderQueue &)`,
  `PlanarReflection::render(const RenderQueue &)`).
- Scenes are drawn through a render queue: `Node::draw()` collects draw items
  (`Node::collect()`, `RenderQueue`) and draws them with a `Renderer`. Nodes keep their
  `doPreDraw()`/`doDraw()`/`doPostDraw()` hooks.
- Opaque objects are drawn sorted by shader program and material, then front to back (fewer
  state changes); `RenderQueue::sortForView()`.
- Nodes without any material (own or inherited) are drawn with a neutral default material instead
  of the material of the previously drawn node. `LineTrace` has its own unlit material.
- Material inheritance follows the scene: a node without a material uses its nearest ancestor's.
  Before, it used whatever material was applied last (often a previous sibling's).
- The built-in shaders (stock, PBR, bump, `GlowMaterial`, `MistMaterial`, `WaterMaterial`,
  `GradientSkyMaterial`) read the camera from the `IvfCamera` uniform block instead of plain
  `view`, `projection` and `viewPos` uniforms. Setting those by name on a built-in program (e.g.
  `program->uniformMatrix4("view", ...)`) no longer has an effect; use `TransformManager`.
  Your own shaders with plain camera uniforms keep working.
- The built-in stock, PBR and bump shaders read the lights from the `IvfLights` uniform block
  instead of plain `pointLights[]`/`dirLights[]`/`spotLights[]` uniforms. Call
  `LightManager::apply()` after changing lights (as before). The PBR shader uses at most 4
  directional lights (was 8). Your own shaders with plain light uniforms keep working.
- `MeshNode` adds one draw item per mesh: meshes of one node are culled, sorted and blended
  separately, and a mesh with its own transparent material is drawn with the transparent
  objects. The node's `doPreDraw()`/`doPostDraw()` run once per mesh, and `MeshNode::doDraw()`
  draws only the mesh being drawn; subclasses overriding `doDraw()` should call
  `MeshNode::doDraw()`.
- `CompositeNode` uses a material set on it (`setMaterial()` on a group used to be ignored
  because groups had `setUseMaterial(false)` by default).

### Added

- `ivfvis`, a new library for scientific visualisation (`ivf2::ivfvis`, `<ivfvis/ivfvis.h>`,
  option `IVF2_BUILD_VIS`): data sets with named scalar and vector arrays (`UniformGrid` with
  trilinear `sample()` and `gradient()`, `PointSet`), `VectorField` (from grids, functions or
  `FlowField`), `Colormap` (viridis, magma, inferno, plasma, cividis, turbo, coolwarm, gray, or
  your own stops; range, log scale, reversal, steps, out-of-range and NaN colours),
  `colorVertices()`/`useVertexColors()` for colouring meshes by values, `ColorBar` and
  nice-number ticks (`niceTicks()`, `logTicks()`, `formatTick()`). New example `vis_colormaps1`.
- `ivfvis::PointCloud`: draws a `PointSet` as point sprites (discs, shaded spheres or squares) in
  one draw call, coloured by a scalar array through a colormap looked up on the GPU, sized in
  world units or pixels and optionally by an array, with transparency and point picking
  (`RayHit::point`). New example `vis_points1`.
- `Node::intersectRay()`: nodes without triangles can provide their own ray test for `pick()`.
- `ivfvis::Isosurface` (marching cubes on a `UniformGrid`, gradient normals, colouring by another
  array, live iso value) and `ivfvis::SlicePlane` (axis-aligned or any plane through the grid,
  unlit colormapped values, contour lines); `extractIsosurface()`, `extractSlice()` and
  `contourLines()` return the geometry as `SurfaceData` without OpenGL. New example `vis_scalar1`.
- `ivfvis::Glyphs` (arrows, cones or lines on a sample grid or given positions, scaled by
  magnitude or constant, coloured through a colormap, picked per glyph) and
  `ivfvis::Streamlines` (RK4 tracing forward, backward or both, as lines or tubes coloured by
  field strength); `traceStreamline()`/`traceStreamlines()` without OpenGL and seed helpers
  (`seedLine`, `seedPlane`, `seedCircle`, `seedRandom`). New example `vis_vectors1`.
- `ivfvis::Volume`: volume rendering of a grid's scalar array by ray marching, with a transfer
  function (colormap colours, opacity curve; `opacityAt()`, `transferTable()`), density, sample
  count and gradient shading; rays stop at opaque geometry. New example `vis_volume1`.
- `ivfvis::Axes3D`: axes around a box with grid lines on the walls facing away from the camera,
  ticks and screen-space labels on one outline edge per axis, and titles; `axisEdge()`. New example
  `vis_axes1`.
- 2D plots with ImPlot: ivfui creates an ImPlot context per window with its ImGui context (CMake
  option `IVF2_WITH_IMPLOT`, vcpkg feature `implot`, `IVF2_HAS_IMPLOT` in `<ivf/config.h>`); ivfvis
  prepares plot data with `sampleLine()` (`LineSamples`) and `histogram()` (`Histogram`). New example
  `vis_dashboard1`.
- ivfvis readers: `readCsv()` (`Table`, `pointsFromTable()`), `readNpy()` (`gridFromNpy()`,
  `pointsFromNpy()`) and `readVtk()` for legacy `.vtk` files (ASCII and binary, versions up to 5.1)
  and VTK XML `.vtu`, `.vtp` and `.vti` files (ASCII, base64 and appended data, zlib); results as
  `VtkData` (grid or points with arrays, triangles, lines) and `createMeshNode()`. XML reading uses
  pugixml and miniz behind the new CMake option `IVF2_WITH_VTK_XML` (vcpkg feature `vtk-xml`).
  New example `vis_readers1`.
- `OverlayNode::project()`, `sceneView()`, `sceneProjection()`, `parentWorld()`: overlays can
  place children at the screen positions of 3D points.
- `Texture3D`: single-channel 3D textures(R8, R16F, R32F) kept on the CPU and uploaded per render
  context when bound.
- `OverlayNode`: children drawn in window pixel coordinates over the scene (HUDs, legends).
- `TextNode::setResolution()` (glyph pixel size, for sharp text at a known size),
  `TextNode::setColor()`/`setUseColor()` (fixed colour independent of lighting).
- `Mesh::colors()`, `Mesh::updateColors()`, `MeshNode::updateColors()` and `Mesh::vertexCount()`:
  change vertex colours after a mesh was built.
- Transparency: materials with alpha < 1 are drawn after all opaque objects, back to front, with
  alpha blending (`setAlpha()` had no visible effect before). Blended materials (`GlowMaterial`,
  `MistMaterial`, ...), particles and billboards are ordered the same way, so blended nodes no
  longer need to be added last. `Node::setRenderLayer()` chooses a layer explicitly
  (`RenderLayer::Background`, `Opaque`, `Transparent`, `Overlay`). New example `transparency1`.
  Transparent meshes are drawn in two passes (back faces, then front faces), so their own
  triangles blend in the right order; `Mesh::clockwise()` tells which winding faces outwards.
- Frustum culling: `GLFWSceneWindow` skips items outside the camera (main and selection passes),
  the light (shadow maps) or the mirrored camera (reflections). `ivf::Frustum`,
  `Renderer::execute(queue, frustum)` and `Renderer::stats()` are public.
- A node can be shown in several windows: each OpenGL context gets its own GPU copy of a mesh
  (`multi_window1` shares a floor between two windows).
- `RenderContext::lifetime()` and `RenderContext::deleteLater()` for objects that hold GL
  resources in several contexts.
- The renderer draws into an explicit render context: `Renderer(RenderContext &, RenderPass)`,
  `LightManager::renderShadowMaps(context, queue)`, `PlanarReflection::render(context, queue)`,
  `Node::draw(context)`, `Mesh::draw(context)`. Nodes can override `Node::doDrawPass(const
  DrawContext &)` to see the pass (`RenderPass::Main`, `Shadow`, `Reflection`, `Selection`),
  the render context and the camera; its default calls `doDraw()`.
- Render statistics: `Renderer::stats()` also counts draw calls, shader program changes and
  material applications; `GLFWSceneWindow::renderStats()` reports them per pass, the
  *Windows > Render Statistics* window shows them, and `IVF2_RENDER_STATS=n` logs them every n
  frames. `RenderContext::counters()` gives the running totals. New user guide page
  *Render Queue*.
- `Program` caches uniform locations: the driver is queried once per name after each `link()`
  (switching to a `ShaderMaterial` or `BumpMaterial` used to query dozens per draw). Render
  statistics count the remaining driver queries (`uniformLookups`, *Lookups* column).
- Camera uniform block `IvfCamera` (`ivf/uniform_blocks.h`): view, projection, view-projection
  and camera position in a std140 uniform buffer shared by all programs of a render context.
  Shaders declare the block (`IVF_CAMERA_BLOCK_GLSL` for C++ strings) and need no per-program
  upload; `Program::link()` binds it, `TransformManager::syncCameraBlock()` writes it when the
  camera changes.
- Ray picking: `ivf::pick(queue, ray)` and `GLFWSceneWindow::pick(x, y)` return the nearest object
  under a ray with the exact surface point, normal and triangle (`RayHit`), tested against the
  meshes' triangles through a bounding volume hierarchy per mesh (`Mesh::bvh()`, `MeshBvh`);
  `pickAll()`, `PickOptions`, `Ray`, `rayFromScreen()`, `cameraRay()`.
  `setSelectionMethod(SelectionMethod::Ray)` drives hover and click events by ray casting
  instead of the selection buffer. New example `picking1`.
- Hot reload: `ResourceWatcher`(`RenderContext::resourceWatcher()`) reloads textures
  (`watchTexture()`, into the same texture), models (`watchModel()`; `GltfModel::reload()` keeps
  the animation and time) and any other files (`watch()` with a reload function) when they are
  saved, after the file has stopped changing. Failed reloads keep the old version. Shader hot
  reload (`ShaderWatcher`) uses it and no longer retries a broken shader every half second.
  `Texture::filename()`. New example `hot_reload1` and guide page *Hot Reload*.
- glTF 2.0 models:`GltfModel::load()` reads `.gltf`/`.glb` files (also without assimp) into
  the scene graph with their node hierarchy, PBR materials (base colour, metallic-roughness,
  normal, occlusion and emissive textures, alpha modes, vertex colours), skinned meshes, morph
  targets and animations (`setAnimation()`, `setTime()`, `setLooping()`, `setMorphWeights()`).
  `ModelLoader::load()` uses it for glTF files. `TransformNode::setRotation()` takes a quaternion.
  `PBRMaterial` gains `setMetallicRoughnessMap()`, `setEmissiveMap()`, `setAlphaMode()` with
  `setAlphaCutoff()`, and `setUseVertexColors()`; `Texture::loadFromMemory()` and `setImage()`.
  New example `gltf1`.
- Screen-space effects:`SsaoEffect` (ambient occlusion from the depth buffer: darker creases,
  corners and contacts) and `ScreenSpaceReflectionEffect` (reflections of the visible scene in
  floors and other surfaces facing one direction, with roughness and Fresnel). Effects also get
  the `projection` and `view` matrices. New examples `ssao1` and `ssr1`, guide page
  *Ambient Occlusion and Reflections*.
- Better shadows: cascaded shadow mapsfor directional lights
  (`DirectionalLight::setShadowCascades()`, `setShadowDistance()`, `setCascadeSplitLambda()`),
  shadows from point lights in all directions (`setCastShadows(true)` on a `PointLight`,
  `setShadowRange()`) and from spot lights (`SpotLight::setShadowRange()`), and soft shadows(`Light::setShadowFilter()` with `ShadowFilter::Hard`,
  `Pcf` or `Pcss`, `setShadowFilterRadius()`, `setShadowSoftness()`), with `setShadowBias()`
  and `setShadowNormalBias()` against acne. PBR materials receive shadows. `Node::setCastShadows()`
  leaves a node out of the shadow maps; `setDebugShadow(5)` shows the cascades. Custom shaders
  can sample shadows with `ivf::shadow_glsl` and `ivfShadowFactor()`. New examples
  `cascaded_shadows1` and `point_shadows1`.
- Image-based lighting: `EnvironmentMap` loadsan equirectangular `.hdr` environment (or
  generates an HDR sky with `loadSky()`) and precomputes irradiance, prefiltered reflections and a
  BRDF table on the GPU; `PBRMaterial::setEnvironment()` lights materials with it (diffuse light
  and reflections that blur with roughness), `Skybox::create(env->cubemap())` shows it and
  `Skybox::setLod()` blurs it. `PBRMaterial::setEnvironmentIntensity()`, `setToneMapping(false)`
  for linear HDR output, `Cubemap::allocate()`, `ivf::writeHdr()`. New example `ibl1` and user
  guide page *PBR and Image-Based Lighting*.
- HDR output: `GLFWSceneWindow::setToneMapping()` (`ivf::ToneMapping`: None, Reinhard, ACES, AgX),
  `setExposure()`, `setGamma()` and `setFxaa()` map the floating-point scene image to the display
  as the last step after all effects; `setMsaaSamples()` sets the multisampling of the offscreen
  image (was fixed at 4). Tone mapping or FXAA turn render to texture on. The defaults leave
  images unchanged. `MipBloomEffect::setToneMapping(false)` leaves tone mapping to the window. New
  example `hdr1` and user guide page *HDR and Tone Mapping*.
- Automatic instancing: copies of a mesh that an `InstanceNode` draws with the same material and
  textures are drawn with one instanced draw call (opaque objects, all passes). Custom shaders
  without the instance attributes get one draw per copy, so images do not change.
  `RenderContext::setAutoInstancing()` and `IVF2_AUTO_INSTANCING=0` turn it off; render
  statistics count `instances`. `Mesh::drawInstances()` and `DrawContext::instances` are public;
  `ivf/instancing.h` (`InstanceAttributes`, `bindInstanceAttributes()`) is shared with
  `InstancedMeshNode`. New example `auto_instancing1`.
- Lights uniform block `IvfLights`: all lights, shadow matrices and counts in one uniform buffer
  shared by all programs, uploaded by `LightManager::apply()` only when a light changed
  (`IVF_LIGHTS_BLOCK_GLSL`, `LightsBlock`). Switching to a lit `ShaderMaterial`, `PBRMaterial` or
  `BumpMaterial` no longer uploads every light to that program.

### Fixed

- API reference: examples grouped by topic, wrong parameter names and duplicate example names in
  the documentation comments fixed, `ivfvis` and `ivfmath` included, styled like the user guide.
- Lit materials with vertex colours (`Material::setUseVertexColor(true)`) used the material's
  diffuse and ambient colour for the light; the vertex colour now takes their place.
- Every `TextNode` rendered its own 128 glyph textures (never freed); glyphs are now created once
  per font and size by the `FontManager` (`FontManager::glyphs()`) and shared.
- A window with selection enableddrew its first frame into the selection buffer
  (`BufferSelection::initialize()` left its framebuffer bound).
- Nodes with a `PBRMaterial`, `BumpMaterial` or `ShaderMaterial`were drawn into shadow maps
  with their own program (the camera's projection) instead of the light's: their shadows were
  wrong or missing. The shadow pass now keeps the depth program (`ShaderManager::lockProgram()`).
- Shadow maps were bound to texture units 1-4, shared with multitexturing and the PBR maps.
- The shadow pass left the cull face mode at `GL_FRONT`.
- `FrameBuffer` leaked its multisampled objectswhen multisampling was turned off.
- Loaded models: a node with several meshes of different materials showed all of them with the
  last mesh's material; further meshes now get their own material (`Mesh::material()`).
- A mesh's own `ShaderMaterial` (`Mesh::setMaterial()`) stayed applied after the mesh was
  drawn.
- `MeshNode::showNormals()` always returned false.
- Shadows of a directional light that is not the first shadow caster: shadow maps and their
  matrices were packed by caster while the shaders read them by light index.
- `Program::uniformLoc()`, `attribId()` and `bindAttribLoc()` passed `std::string_view` data to
  OpenGL without a terminating null; `uniformMatrix4Array()` sets the whole array in one call.
- Bounding boxes of `Box`, `RoundedBox` and `Plane` were half their real size (and those of the
  other generator primitives were approximations): primitives now use the box computed from their
  vertices. Code using scene extents (camera framing, `Extents`) sees the correct size.
- `MeshNode::updateVertices()` updates the bounding box.
- Selection on high-DPI displays (macOS Retina): the selection buffer and its viewport now use window
  coordinates like the mouse position, so hovering and clicking pick the object under the cursor.
- Hovering only reports a node while the cursor is over the window; outside it (or in hidden
  windows) the selection buffer was read at undefined pixels.
- Automatic shadow framing (`LightManager::setAutoCalcBBox(true)`, the default) used an empty,
  infinitely large scene box; it now fits the shadow maps to the scene's bounding boxes, and the
  shadow and main passes use the same light matrix.
- With selection enabled, every frame rendered the shadow maps twice and called `onDraw()` twice;
  the selection pass now draws the frame's collected items only.
- Line widths above 1 (`Mesh::setLineWidth`) now work on Windows and Linux: windows no longer
  request a forward-compatible OpenGL context there (macOS still needs one and keeps
  1-pixel lines). Scenes with wide lines look different from before.

## [2.0.0-alpha.2] - 2026-11-22

### Changed (may require changes in your code)

- C++23 is required (MSVC 2022, GCC 13, Clang 17, Xcode 16 or newer).
- Loaders and shader compilation report failures as `ivf::Result<T>`
  (`std::expected<T, ivf::Error>`): `ShaderManager::compileProgramFromFiles`/`FromStrings`,
  `load*Shader()`, texture, font and model loading. The old functions remain as `[[deprecated]]`
  wrappers. Results are `[[nodiscard]]`.
- GL state lives in a per-context `ivf::RenderContext` (`renderContext()->lights()`, ...).
  `XxxManager::instance()` still works and forwards to the current context.
- glm is the only math library (also in the generator); the bundled glm and dirent are gone.
- Getters are named after the property: `getTexture(i)` -> `texture(i)`, `getAmplitude()` ->
  `amplitude()`, `getProperties()` -> `properties()` and 18 more, with deprecated aliases.
  101 getters are now `const`, `create()` factories are `[[nodiscard]]`, and string parameters are
  `const std::string &`.
- Public headers no longer include assimp, FreeType, miniaudio, RtMidi, stb, nlohmann-json or ImGui
  (ivf core); stb is available through `ivf/image_io.h`.
- Repository layout: library runtime data moved from `bin/` to `data/` (`shaders/`, `fonts/`),
  example assets to `examples/assets/`. Build output goes to `build/<preset>/bin/` and `lib/`, with a
  `<Config>` level only for multi-config generators such as Visual Studio
  (`IVF2_OUTPUT_IN_SOURCE_TREE=ON` puts them in the source tree).

### Added

- Modern CMake build: presets, static targets exported as `ivf2::<name>`, install rules and
  `find_package(ivf2)`, vcpkg auto-detection, precompiled headers, `ivf2_copy_runtime_files()`.
- Optional dependencies `IVF2_WITH_ASSIMP`, `IVF2_WITH_FREETYPE`, `IVF2_WITH_AUDIO`,
  `IVF2_WITH_MIDI` (default ON) with `IVF2_HAS_*` macros in the generated `<ivf/config.h>`.
- CI on Windows, Linux and macOS with warnings as errors, unit tests, a package consumer build,
  a minimal (all options off) build, CodeQL and visual regression tests (`IVF2_VISUAL_TESTS`).
- Diagnostics: OpenGL debug output routed to the Logger, `ivf::glLabel()`, logger sinks, compact
  log format with wrapping.
- Unattended runs: `IVF2_HIDE_WINDOWS`, `IVF2_EXIT_AFTER_FRAMES`, `IVF2_SCREENSHOT`,
  `IVF2_SCREENSHOT_UI`, `IVF2_FIXED_TIMESTEP`, `IVF2_RANDOM_SEED` (`ivf::automaticSeed()`).
- `ivfmusic`: song-driven scenes (`GLFWMusicSceneWindow`, `Automation`, Renoise `.xrns` scores,
  MIDI `LiveControls`, `AutomationRecorder`), blend materials (`GlowMaterial`, `MistMaterial`,
  `GradientSkyMaterial`), `PathCameraManipulator`.
- Effects framework and post-processing effects (bloom, mip bloom, fog, depth of field, ...),
  planar reflections, procedural and GPU-generated textures, multitexturing, bump mapping,
  frame export, particle systems, flow fields, tweens and timers.
- `SECURITY.md`, `CONTRIBUTING.md`, this changelog, issue and pull request templates.

### Fixed

- Crash on exit on Linux: windows and their scenes are now destroyed before `glfwTerminate()`.
- High-DPI displays (macOS Retina): the camera manipulator mixed window and framebuffer sizes,
  so the mouse ray for the 3D cursor was wrong and the projection was recomputed every frame
  while a mouse button was held.
- Bugs found by enabling `/W4` and `-Wall -Wextra`: `Capsule` ignored its constructor
  arguments, keyframe tension was ignored, `GridLayout` setters were stubs,
  `PolyLinePath::setSegments` did nothing, `TransformManager::rotateToVector` used the wrong axis,
  click callbacks in `GLFWSceneWindow` never fired, missing virtual destructors.
- A window whose `onSetup()` fails is closed instead of crashing in `onDraw()`.

### Removed

- Unused files: `test_span.cpp`, `include/ivf/logger.cpp`, the outdated root `shaders/`
  directory, `examples/_example8`, the embedded font `ivf_font.cpp` and the `python/` scripts.

### Security

- Vulnerabilities are reported privately through GitHub (see [SECURITY.md](SECURITY.md)).

## [2.0.0-alpha] - 2025-06-22

First public pre-release of ivf2: scene graph, primitives, materials and lighting, textures,
text rendering, model loading with assimp, keyframe animation, procedural geometry (generator),
ImGui-based user interface and around 20 examples.

[Unreleased]: https://github.com/jonaslindemann/ivf2/compare/v2.0.0-alpha.3...HEAD
[2.0.0-alpha.3]: https://github.com/jonaslindemann/ivf2/compare/v2.0.0-alpha.2...v2.0.0-alpha.3
[2.0.0-alpha.2]: https://github.com/jonaslindemann/ivf2/compare/2.0.0-alpha...v2.0.0-alpha.2
[2.0.0-alpha]: https://github.com/jonaslindemann/ivf2/releases/tag/2.0.0-alpha
