# Shadows

Shadows make it much easier to read where objects are in space: without them, a floating cube and a cube on the floor can look identical. ivf2 uses shadow mapping: the scene is first rendered from the light's point of view to record what the light can see, and every surface the light can't see is darkened.

![Shadows](../images/sc_shadows1.png)

## Turning shadows on

Shadows need two switches: shadow mapping in the `LightManager`, and shadow casting on the light. Only **directional lights** cast shadows.

```cpp
auto lm = ivf::LightManager::instance();
lm->clearLights();
lm->setUseShadows(true);

// The region that shadows are computed for
lm->setAutoCalcBBox(false);
lm->setSceneBoundingBox({-30.0f, -30.0f, -20.0f}, {30.0f, 30.0f, 30.0f});

auto sun = lm->addDirectionalLight();
sun->setDirection({-0.2f, -1.0f, -0.2f});
sun->setDiffuseColor({1.0f, 1.0f, 1.0f});
sun->setAmbientColor({0.3f, 0.3f, 0.3f});
sun->setEnabled(true);
sun->setCastShadows(true);
sun->setShadowMapSize(4096, 4096);
sun->setShadowStrength(0.3f);

lm->apply();
```

Every node in the scene both casts and receives shadows; there is no per-object switch.

## The scene bounding box

A directional light is infinitely far away, so the shadow map has to be fitted to a region of the scene. That region is the scene bounding box. Set it explicitly to enclose everything that should cast or receive shadows:

- **Too small**, and objects outside it get no shadows or have their shadows cut off.
- **Too large**, and the same shadow map is spread over more space, so shadows get blocky.

!!! note
    `setAutoCalcBBox(true)` is meant to fit the box to the scene automatically, but in the current version the computed box is not used. Always set the box with `setSceneBoundingBox()`.

## Quality settings

| Setting | Effect |
|---------|--------|
| `setShadowMapSize(w, h)` | Resolution of the shadow map; default 2048 × 2048. Higher gives sharper shadows and costs memory and time |
| `setShadowStrength(s)` | How dark shadows are, from 0 (none) to 1 (black). Lower values look more natural with ambient light |
| Scene bounding box | A tighter box gives sharper shadows from the same map size |
| Light direction | Light coming from steeply above gives the most stable shadows; very low sun angles stretch them |

For a large scene with a small area of interest, keep the bounding box tight around that area rather than raising the map size.

## Debugging

`LightManager::setDebugShadow(mode)` makes the stock shader show shadow information instead of the lit scene, for the first directional light:

| Mode | Shows |
|------|-------|
| 0 | Normal rendering |
| 1 | Depth stored in the shadow map, as greyscale |
| 2 | Red where a surface is in shadow, green where it is lit |
| 3 | Shadow map coordinates as colour; values outside 0–1 mean the surface is outside the scene bounding box |
| 4 | Surface depth (red) against stored depth (green) |

Common causes of problems:

| Symptom | Likely cause |
|---------|--------------|
| No shadows at all | `setUseShadows(true)` or `setCastShadows(true)` missing, or the light isn't directional |
| Shadows cut off at an edge | The scene bounding box is too small |
| Blocky, jagged shadows | Bounding box too large, or shadow map too small |
| Striped patterns on lit surfaces ("shadow acne") | Light at a grazing angle to the surface; make the light steeper |

## Related

- [Lighting](../core/lighting.md)
- `shadows1` example in [Effects Examples](../examples/effects_examples.md)
