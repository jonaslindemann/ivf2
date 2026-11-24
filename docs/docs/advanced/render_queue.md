# Render Queue

ivf2 draws a scene in two steps. First the scene graph is walked once and every drawable node becomes a *draw item* in a `RenderQueue`. Then a `Renderer` draws the items, once per pass: shadow maps, planar reflections, the camera's image and the selection buffer all use the same items. This page explains what happens in a frame, how to influence it, how to extend it and how to see what it costs.

You do not need any of this to build scenes: `GLFWSceneWindow` runs the passes for you, and `scene->draw()` still works.

## A frame in GLFWSceneWindow

```
scene --collect()--> RenderQueue [DrawItem, ...] --sortForView()-->
    shadow pass      (per shadow-casting light, culled with the light's view)
    reflection pass  (per PlanarReflection, culled with the mirrored camera)
    main pass        (culled with the camera)
    selection pass   (only after mouse input or camera changes)
```

Each pass is a `Renderer` that filters, culls and draws the items:

- **Collect.** `Node::collect()` walks the tree. Containers (`CompositeNode`, `InstanceNode`) add no item of their own; they pass their transform, material and textures down to their children (`CollectState`). Every other visible node adds one `DrawItem` with its world matrix, world-space bounding box, the inherited material and textures, and its layer.
- **Sort.** `RenderQueue::sortForView()` orders the items by layer: `Background` (skyboxes), `Opaque` (sorted by shader program and material, then front to back), `Transparent` (back to front) and `Overlay`.
- **Cull.** Items whose bounding box is outside the pass's view volume (`ivf::Frustum`) are skipped. Items without a bounding box are always drawn.
- **Draw.** For each item the renderer sets the model matrix, applies the inherited material (or a neutral default) when the node has none of its own, and calls the node's `doPreDraw()`, `doDrawPass()` and `doPostDraw()`.

## Materials, layers and transparency

- **Inheritance.** A node without a material uses the material of its nearest ancestor that has one; with none at all it gets a neutral light grey material. Textures are inherited the same way.
- **Transparency.** A material with alpha < 1 puts the node in the `Transparent` layer: it is drawn after all opaque objects, back to front, with alpha blending and without depth writes. Blended shader materials (`GlowMaterial`, `MistMaterial`, ...), particles and billboards are transparent too, so they no longer have to be added to the scene last.
- **Explicit layer.** `node->setRenderLayer(ivf::RenderLayer::Overlay)` overrides the layer chosen from the material; `clearRenderLayer()` undoes it.

## Extending

Pick the lowest level that does what you need.

### 1. Geometry nodes

Subclass `MeshNode` and fill the mesh in `doSetup()`, as the built-in primitives do. Shadows, culling, selection, sorting and transparency work without further code. A `MeshNode` adds one item per enabled mesh (`DrawItem::mesh`), so meshes with their own material (`Mesh::setMaterial()`) are sorted, and blended if transparent, on their own. The node's hooks run once per mesh item; if you override `doDraw()`, call `MeshNode::doDraw()`, which draws the item's mesh. Creating the node needs no OpenGL context: the mesh is uploaded when it is first drawn.

### 2. Nodes with their own OpenGL drawing

Subclass `TransformNode` and override `doDraw()`, as `TextNode`, `Skybox` and `ParticleSystem` do. The renderer sets the world matrix and material before calling it. Give the node a bounding box (`setLocalBoundingBox()`) so it can be culled.

When the drawing depends on the pass, or should use the renderer's render context instead of the manager singletons, override `doDrawPass()`. Its default calls `doDraw()`:

```cpp
#include <ivf/draw_context.h>
#include <ivf/render_context.h>

class Halo : public ivf::TransformNode {
protected:
    void doDrawPass(const ivf::DrawContext &dc) override
    {
        if (dc.pass == ivf::RenderPass::Shadow)
            return; // a glow casts no shadow

        auto program = dc.context.shaders()->currentProgram();
        // ... draw with dc.view, dc.projection, dc.item.world
    }
};
```

`DrawContext` holds the `RenderContext` being drawn into, the pass (`Main`, `Shadow`, `Reflection`, `Selection`), the item and the pass's view and projection matrices.

### 3. Deciding what to collect

Override `collect()` to decide per frame whether, or what, a node adds to the queue, for example a level-of-detail switch:

```cpp
void collect(ivf::RenderQueue &queue, const ivf::CollectState &state) override
{
    // state.world is the parent's world matrix; pos() is relative to the parent
    const glm::vec3 worldPos = glm::vec3(state.world * glm::vec4(pos(), 1.0f));
    if (glm::distance(m_eye, worldPos) < m_maxDistance)
        ivf::TransformNode::collect(queue, state);
}
```

## Drawing a queue yourself

Hosts without `GLFWSceneWindow` (see [Render Context](render_context.md)) can run the same passes. A `Renderer` draws into the `RenderContext` it is given; make that context current first, as for any OpenGL call:

```cpp
#include <ivf/renderer.h>

ctx->makeCurrent();

ivf::RenderQueue queue;
scene->collect(queue, ivf::CollectState{});
queue.sortForView(ctx->transforms()->viewPos());

ctx->lights()->renderShadowMaps(*ctx, queue);   // optional

ivf::Renderer renderer(*ctx);                   // RenderPass::Main
renderer.setTransparencyBlending(true);
auto *xfm = ctx->transforms();
renderer.execute(queue, ivf::Frustum(xfm->projectionMatrix() * xfm->viewMatrix()));
```

`scene->draw()` does the same without culling, using the current render context; `scene->draw(*ctx)` names the context explicitly.

## Statistics

Every `Renderer::execute()` records what it did in `renderer.stats()`:

| Field | Meaning |
|-------|---------|
| `items` | Items in the queue |
| `drawn` | Items drawn |
| `culled` | Items outside the view volume |
| `skipped` | Items left out by the pass's filter (a reflection leaves out its mirror) |
| `drawCalls` | OpenGL draw calls (meshes, text, billboards, particles, skybox) |
| `programChanges` | Switches to another shader program |
| `materialApplies` | Materials applied (own, inherited or default) |
| `uniformLookups` | Uniform locations queried from the driver; `Program` caches them, so this is normally 0 after the first frame |

`GLFWSceneWindow` collects them for each frame:

- **Window:** *Windows > Render Statistics* shows a table per pass.
- **Code:** `renderStats()` returns the shadow, reflection, main and selection counts of the last frame; `renderStats().frame()` adds up the passes drawn every frame.
- **Log:** the environment variable `IVF2_RENDER_STATS=n` logs the counts every n frames, also in unattended runs (see [Diagnostics](diagnostics.md)):

```
16:01:15.246 INFO  RenderStats      Frame 30: main 23/23 drawn, 0 culled, 23 draw calls, 2 programs,
                                      23 materials
16:01:15.246 INFO  RenderStats        reflection 21/23 drawn, 1 culled, 21 draw calls, 0 programs,
                                      21 materials
```

For other stretches of code, compare two snapshots of `RenderContext::counters()`, the running totals the statistics are computed from.

High `culled` counts mean culling saves work; `drawCalls` higher than `drawn` comes from transparent meshes (drawn twice, back faces first) and multi-part nodes; many `programChanges` relative to `drawn` point at materials with their own shaders spread over the scene.
