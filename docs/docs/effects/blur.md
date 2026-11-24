# Blur, Bloom and Depth Effects

These effects soften the image or give it depth: plain blur, glow around bright areas, camera-style focus, fog, and motion trails. All of them follow the usual pattern from the [overview](index.md): create, set parameters, `load()`, `addEffect()`, with `setRenderToTexture(true)` on the window.

## BlurEffect

Blurs the whole image evenly. Useful for dreamy backgrounds or as a transition, for example by animating the radius from 0 up and back down.

```cpp
auto blur = ivf::BlurEffect::create();
blur->setBlurRadius(2.0f);
blur->load();
addEffect(blur);
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setBlurRadius(r)` | 0.0 | Blur radius in pixels. Always set a value above 0; cost grows with the square of the radius |

To switch blur off, disable the effect rather than setting the radius to 0.

## BloomEffect

Makes bright areas glow by spreading their light into the surroundings.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setThreshold(t)` | 1.0 | Brightness above which pixels start to glow |
| `setIntensity(i)` | 1.0 | Strength of the glow |

## MipBloomEffect

A wider, softer and cheaper bloom. It builds the glow from progressively blurred copies of the image (the texture's mip chain), so the halo can spread across a large part of the screen. It then tone maps the result: values brighter than white roll off smoothly instead of clipping.

This is the bloom to use for glowing particles, neon and light against a dark background, especially with [`GlowMaterial`](../core/shader_materials.md#glowmaterial) and additive blending.

```cpp
m_bloom = ivf::MipBloomEffect::create();
m_bloom->setThreshold(0.6f);
m_bloom->setIntensity(0.8f);
m_bloom->setSpread(0.75f);
m_bloom->load();
addEffect(m_bloom);   // add last
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setThreshold(t)` | 0.6 | Brightness where the glow starts, with a soft knee |
| `setIntensity(i)` | 0.8 | Strength of the glow |
| `setSpread(s)` | 0.75 | How far the halo reaches; higher uses the larger, blurrier levels more |
| `setToneMapping(on)` | on | The effect's own soft shoulder; turn it off when the window tone maps ([HDR and Tone Mapping](hdr.md)) |
| `setExposure(e)` | 1.0 | Exposure applied before tone mapping |

!!! tip
    Place `MipBloomEffect` last in the chain. Its tone mapping should see the final colours, and effects after it would work on already tone-mapped values.

## Depth of field

`DepthOfFieldEffect` imitates a camera lens: surfaces near the focus distance stay sharp, and everything in front of or behind them blurs with a soft, bokeh-like look. It uses the scene depth buffer, so it knows how far away each pixel is.

```cpp
auto dof = ivf::DepthOfFieldEffect::create();
dof->setFocusDistance(8.0f);
dof->setFocusRange(1.5f);
dof->setBlurRange(6.0f);
dof->load();
addEffect(dof);
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setFocusDistance(d)` | 10.0 | Distance from the camera that is in focus, in world units |
| `setFocusRange(r)` | 1.0 | Depth around the focus distance that stays completely sharp |
| `setBlurRange(r)` | 8.0 | Distance over which blur grows from none to maximum |
| `setMaxBlur(px)` | 12.0 | Largest blur radius in pixels |
| `setRadiusStep(s)` | 0.75 | Sample spacing; larger is faster but coarser |
| `setAutoFocus(on)` | false | Focus on whatever is at the centre of the screen, every frame |
| `setShowBlur(on)` | false | Show the blur amount instead of the image: red in front of the focus, blue behind |

Cost grows quickly with `maxBlur`, so keep it moderate. Use `setShowBlur(true)` while tuning, and add depth of field before any effect that moves pixels (wave, swirl, kaleidoscope).

## Fog

`FogEffect` adds atmospheric fog based on how far away each surface is. With a height falloff the fog gathers near the ground and thins out above, which gives low-lying mist. It still looks right when the camera itself is inside the fog layer.

```cpp
auto fog = ivf::FogEffect::create();
fog->setColor({0.05f, 0.04f, 0.10f});
fog->setDensity(0.08f);
fog->setHeightFalloff(0.6f);
fog->load();
addEffect(fog);
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setColor(rgb)` | (0.5, 0.6, 0.7) | Fog colour; usually match it to the background or sky |
| `setDensity(d)` | 0.05 | How quickly fog thickens with distance |
| `setStart(d)` | 0.0 | Distance where fog begins |
| `setHeightFalloff(f)` | 0.0 | How fast fog thins with height; 0 gives uniform fog |
| `setBaseHeight(y)` | 0.0 | Height where height fog is densest |
| `setMaxOpacity(a)` | 1.0 | Upper limit on fog coverage |
| `setSkyAmount(a)` | 1.0 | How much fog covers the background where nothing was drawn |
| `setGroundBelowHorizon(on)` | false | Treat background below the horizon as distant ground, so it fogs like the floor |
| `setHorizonBlend(b)` | 0.0 | Width of a band above the horizon that blends ground fog into sky fog |

Add fog early in the chain, before bloom and vignette. For a soft, hand-placed mist on individual objects instead of the whole scene, see [`MistMaterial`](../core/shader_materials.md#mistmaterial).

## MotionBlurEffect

Blends each frame with the previous one, leaving smeared trails behind anything that moves.

| Parameter | Description |
|-----------|-------------|
| `setBlendFactor(f)` | How much of the previous frame to keep; higher gives longer trails |

For trails with zoom, rotation and colour decay, use [`FeedbackEffect`](film_effects.md#feedbackeffect) instead.

## Related

- [Post-processing overview](index.md)
- [Effects Examples](../examples/effects_examples.md)
