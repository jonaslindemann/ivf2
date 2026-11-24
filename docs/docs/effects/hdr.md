# HDR and Tone Mapping

A light can be brighter than white. With render to texture, `GLFWSceneWindow` draws the scene into a floating-point image (16-bit float per channel), so lit surfaces, specular highlights and additive glows keep values above 1 instead of clipping. Effects work on these values. The very last step, the **output step**, maps them to the display:

```
scene (HDR, RGBA16F, multisampled) -> effects (HDR) -> exposure -> tone mapping -> gamma -> FXAA -> screen
```

## Turning it on

```cpp
#include <ivf/tone_mapping.h>

int onSetup() override
{
    // ... scene with lights brighter than 1, e.g. light->setDiffuseColor({4.0f, 3.0f, 2.0f})

    setToneMapping(ivf::ToneMapping::Aces); // also turns render to texture on
    setExposure(1.0f);
    setFxaa(true);
    return 0;
}
```

A tone mapping other than `None`, or FXAA, switches render to texture on by itself. The defaults (`None`, exposure 1, gamma 1, no FXAA) copy the image unchanged, so existing scenes look the same.

## Settings

| Function | Default | Description |
|----------|---------|-------------|
| `setToneMapping(t)` | `None` | Curve from HDR to the display range, see below |
| `setExposure(e)` | 1.0 | Brightness scale before tone mapping; lower it when the scene looks washed out, raise it when it is dark |
| `setGamma(g)` | 1.0 | Applied after tone mapping. ivf2's materials and lights are authored in display space, so 1 suits most scenes; use 2.2 for shaders that light in linear space |
| `setFxaa(on)` | off | Fast approximate anti-aliasing of the final image: one cheap pass that softens jagged edges |
| `setMsaaSamples(n)` | 4 | Multisampling of the offscreen scene image (0 turns it off; clamped to what the driver supports) |

## Tone mapping operators

| `ivf::ToneMapping` | Look |
|--------------------|------|
| `None` | Values above 1 clip: blown-out, saturated highlights |
| `Reinhard` | `c / (1 + c)`: never clips, but flattens contrast |
| `Aces` | Filmic ACES fit (RRT + ODT, Stephen Hill): contrasty, saturated highlights. A good default |
| `AgX` | AgX (Troy Sobotka): very bright colours desaturate towards white, like overexposed film |

`ivf::toneMappingName()` and `ivf::ToneMappingCount` help to build a selection list in the UI.

## Bloom in HDR

[`MipBloomEffect`](blur.md#mipbloomeffect) has its own soft tone mapping. With the window tone mapping, turn it off so the bloom stays in HDR and the image is mapped only once:

```cpp
auto bloom = ivf::MipBloomEffect::create();
bloom->load();
bloom->setThreshold(1.0f);    // only values brighter than white glow
bloom->setToneMapping(false); // the window maps the result
addEffect(bloom);
setToneMapping(ivf::ToneMapping::Aces);
```

## Anti-aliasing

- **MSAA** (`setMsaaSamples()`) samples every pixel several times while drawing; the multisampled image is resolved before the effects run. Accurate, costs memory and fill rate.
- **FXAA** (`setFxaa()`) works on the final image: it finds high-contrast edges and blurs along them. Cheap and independent of the scene, but softens fine detail and text.

Both can be combined. SMAA is not available yet.

## Example

`hdr1` lights a row of spheres with two lights several times brighter than white and lets you switch the tone mapping, exposure, gamma, light intensity, bloom, FXAA and MSAA at runtime.
