# Physically Based Rendering and Image-Based Lighting

`PBRMaterial` shades surfaces with a physically based model (Cook-Torrance, GGX): a surface is described by its colour, how rough it is and whether it is a metal. Lit by an HDR environment (`EnvironmentMap`), the materials pick up light and reflections from all around them, not only from the scene's lights.

## PBRMaterial

```cpp
#include <ivf/pbr_material.h>

auto gold = ivf::PBRMaterial::create();
gold->setAlbedo({1.0f, 0.78f, 0.34f, 1.0f}); // base colour
gold->setMetallic(1.0f);                       // 0 = dielectric (plastic, stone), 1 = metal
gold->setRoughness(0.3f);                      // 0 = mirror, 1 = completely matte
sphere->setMaterial(gold);
```

| Function | Default | Description |
|----------|---------|-------------|
| `setAlbedo(c)` | white | Base colour (for metals, the colour of the reflection) |
| `setMetallic(m)` | 0 | Metal (1) or dielectric (0) |
| `setRoughness(r)` | 0.5 | Microsurface roughness, clamped to 0.04-1 |
| `setEmissive(c)` | black | Light the surface emits itself |
| `setAO(a)` | 1 | Ambient occlusion: how much ambient light reaches the surface |
| `setAlbedoMap()`, `setNormalMap()`, `setRoughnessMap()`, `setMetallicMap()`, `setAOMap()` | - | Textures for the values above |

The scene's point, directional and spot lights light PBR materials like the stock ones. The `textures4` example shows a grid of spheres from rough to smooth and dielectric to metal.

## Image-based lighting

An environment map is an HDR panorama of the surroundings. `EnvironmentMap` turns it into what the shader needs, once, on the GPU:

- an HDR **cube map** of the environment (also used as the background),
- an **irradiance** map: the diffuse light arriving from every direction,
- a **prefiltered** map: reflections blurred for increasing roughness, one roughness per mip level,
- a **BRDF lookup table** for the split-sum approximation (Karis 2013).

```cpp
#include <ivf/environment_map.h>
#include <ivf/skybox.h>

auto env = ivf::EnvironmentMap::create();
if (!env->load("assets/studio.hdr"))   // equirectangular Radiance .hdr
    return -1;                         // the reason is logged

gold->setEnvironment(env);             // diffuse light and reflections from the environment
add(ivf::Skybox::create(env->cubemap()));
```

Environments are equirectangular (latitude-longitude) images, twice as wide as high. Free CC0 environments are available from [Poly Haven](https://polyhaven.com/hdris) (download the `.hdr` version; `.exr` is not supported). `loadEquirectangular(rgb, width, height)` takes float data from elsewhere.

Without a file, `loadSky()` generates a procedural HDR sky - gradient, ground and a sun:

```cpp
ivf::SkySettings sky;
sky.sunDirection = {0.5f, 0.45f, -0.6f};
sky.sunIntensity = 60.0f;      // the sun is far brighter than the sky
if (!env->loadSky(sky))
    return -1;
```

`EnvironmentMap::skyImage(settings)` returns the generated image, which `ivf::writeHdr()` saves as a `.hdr` file.

| Function | Description |
|----------|-------------|
| `PBRMaterial::setEnvironment(env)` | Light the material with the environment (replaces the plain reflection of `setEnvCubemap()`) |
| `PBRMaterial::setEnvironmentIntensity(i)` | Scale the environment's light (default 1) |
| `Skybox::setLod(l)` | Show a blurred background: mip level `l` of the environment cube map |
| `EnvironmentMap::setCubemapSize(s)` | Face size of the environment cube map for loaded images (default 512) |

Loading needs a current OpenGL context (call it in `onSetup()`). Creating a new environment takes a fraction of a second, so it can be regenerated at runtime, for example when the sun moves.

## HDR output

Environments and bright lights produce values above 1. For the full range, let the window tone map and keep the materials linear:

```cpp
material->setToneMapping(false);          // the material outputs linear HDR values
setToneMapping(ivf::ToneMapping::Aces);   // the window maps HDR to the display...
setGamma(2.2f);                           // ...and encodes the linear values
```

By default a `PBRMaterial` tone maps (Reinhard) and gamma-encodes in its own shader, so it looks right without HDR output. See [HDR and Tone Mapping](../effects/hdr.md).

## Example

`ibl1` shows three rows of spheres (gold, red plastic, white ceramic) with roughness increasing from left to right, lit by a procedural sky or by an `.hdr` file given on the command line (`ibl1 studio.hdr`). Sliders change the environment intensity, the background blur, the exposure and the sun's position.
